#include "midisynthesizer.h"

#include <cmath>
#include <cstring>

#include <QDebug>
#include <QMutexLocker>

#include <QMediaDevices>
#include <QAudioDevice>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double MidiSynthesizer::pitchToFrequency(int pitch)
{
    return 440.0 * pow(2.0, (pitch - 69) / 12.0);
}

double WaveGenerator::adsrEnvelope(
    double t,
    double totalDur,
    double attack,
    double decay,
    double sustain,
    double release)
{
    if (t < attack)
    {
        return t / attack;
    }

    if (t < attack + decay)
    {
        double progress =
            (t - attack) / decay;

        return 1.0 -
               progress * (1.0 - sustain);
    }

    if (t > totalDur - release)
    {
        double progress =
            (t - (totalDur - release))
            / release;

        return sustain * (1.0 - progress);
    }

    return sustain;
}

QVector<float> WaveGenerator::generateNote(
    int pitch,
    int durationMs,
    int sampleRate,
    double startPhase,
    double *endPhase)
{
    double frequency =
        MidiSynthesizer::pitchToFrequency(pitch);

    int totalSamples =
        durationMs * sampleRate / 1000;

    QVector<float> samples(totalSamples);

    double totalDurSec =
        durationMs / 1000.0;

    double attack  = 0.012;
    double decay   = 0.055;
    double sustain = 0.46;
    double release = 0.095;

    double phase = startPhase;

    double phaseIncrement =
        2.0 * M_PI * frequency / sampleRate;

    for (int i = 0; i < totalSamples; ++i)
    {
        double t =
            (double)i / sampleRate;

        double env =
            adsrEnvelope(
                t,
                totalDurSec,
                attack,
                decay,
                sustain,
                release
                );

        double s1 = sin(phase);
        double s2 = 0.18 * sin(phase * 2.0);
        double s3 = 0.08 * sin(phase * 3.0);

        double sample =
            (s1 + s2 + s3);

        sample *= env;

        sample *= 0.45;

        samples[i] =
            static_cast<float>(sample);

        phase += phaseIncrement;

        if (phase >= 2.0 * M_PI)
        {
            phase -= 2.0 * M_PI;
        }
    }
    int fadeSamples =
        sampleRate * 10 / 1000;

    fadeSamples =
        qMin(fadeSamples, totalSamples);

    for (int i = totalSamples - fadeSamples;
         i < totalSamples;
         ++i)
    {
        float ratio =
            float(i - (totalSamples - fadeSamples))
            / fadeSamples;

        samples[i] *= (1.0f - ratio);
    }

    if (endPhase)
    {
        *endPhase = phase;
    }

    return samples;
}

MidiSynthesizer::MidiSynthesizer(QObject *parent)
    : QObject(parent),
    m_audioSink(nullptr),
    m_audioDevice(nullptr),
    m_sampleRate(44100),
    m_initialized(false),
    m_useInt16(false),
    m_phase(0.0),
    m_notePlaying(false)
{
    qDebug() << "MidiSynthesizer created";
}

MidiSynthesizer::~MidiSynthesizer()
{
    if (m_audioSink)
    {
        m_audioSink->stop();
    }
}

bool MidiSynthesizer::ensureAudioReady()
{
    if (m_initialized)
    {
        return true;
    }

    QAudioDevice outputDevice =
        QMediaDevices::defaultAudioOutput();

    if (outputDevice.isNull())
    {
        qWarning()
        << "No audio device";

        return false;
    }

    QAudioFormat format;

    format.setSampleRate(m_sampleRate);

    format.setChannelCount(1);

    format.setSampleFormat(
        QAudioFormat::Int16
        );

    m_useInt16 = true;

    if (!outputDevice.isFormatSupported(format))
    {
        format =
            outputDevice.preferredFormat();
    }

    m_audioSink =
        new QAudioSink(
            outputDevice,
            format,
            this
            );

    m_audioSink->setBufferSize(65536);

    m_audioSink->setVolume(0.8);

    m_audioDevice =
        m_audioSink->start();

    if (!m_audioDevice)
    {
        qWarning()
        << "Audio start failed";

        return false;
    }

    m_initialized = true;

    qDebug()
        << "Audio initialized";

    return true;
}

float MidiSynthesizer::softClip(float x)
{
    return tanh(x * 1.2f);
}

void MidiSynthesizer::playNote(
    int pitch,
    int velocity)
{
    QMutexLocker locker(&m_audioMutex);

    if (m_notePlaying)
    {
        return;
    }

    if (!ensureAudioReady())
    {
        return;
    }

    m_notePlaying = true;

    emit notePlaying(pitch);

    int durationMs =
        qMax(
            250,
            700 - (pitch - 60) * 4
            );

    double endPhase = 0.0;

    QVector<float> samples =
        WaveGenerator::generateNote(
            pitch,
            durationMs,
            m_sampleRate,
            m_phase,
            &endPhase
            );

    m_phase = endPhase;

    float velocityScale =
        0.2f +
        (velocity / 127.0f) * 0.5f;

    for (float &s : samples)
    {
        s *= velocityScale;

        s = softClip(s);
    }

    playSamples(samples);

    QTimer::singleShot(
        durationMs,
        this,
        [this]()
        {
            m_notePlaying = false;
        });
}

void MidiSynthesizer::playSamples(
    const QVector<float> &samples)
{
    if (!m_audioDevice)
    {
        return;
    }

    QByteArray pcm;

    pcm.resize(
        samples.size()
        * sizeof(qint16)
        );

    qint16 *out =
        reinterpret_cast<qint16*>(
            pcm.data()
            );

    for (int i = 0;
         i < samples.size();
         ++i)
    {
        float s =
            qBound(
                -1.0f,
                samples[i],
                1.0f
                );

        out[i] =
            static_cast<qint16>(
                s * 32767.0f
                );
    }

    m_audioDevice->write(
        pcm.constData(),
        pcm.size()
        );
}