#include "midisynthesizer.h"
#include <cmath>
#include <QDebug>
#include <QMediaDevices>
#include <QAudioDevice>

double MidiSynthesizer::pitchToFrequency(int pitch)
{
    return 440.0 * pow(2.0, (pitch - 69) / 12.0);
}

double WaveGenerator::adsrEnvelope(double t, double totalDur,
                                   double attack, double decay,
                                   double sustain, double release)
{
    if (t < attack)
        return t / attack;
    else if (t < attack + decay) {
        double progress = (t - attack) / decay;
        return 1.0 - progress * (1.0 - sustain);
    }
    else if (t > totalDur - release) {
        double progress = (t - (totalDur - release)) / release;
        return sustain * (1.0 - progress);
    }
    else
        return sustain;
}

QVector<float> WaveGenerator::generateNote(int pitch, int durationMs, int sampleRate)
{
    double frequency = MidiSynthesizer::pitchToFrequency(pitch);
    int totalSamples = durationMs * sampleRate / 1000;
    QVector<float> samples(totalSamples);
    double totalDurSec = durationMs / 1000.0;

    double attack  = 0.015;
    double decay   = 0.08;
    double sustain = 0.65;
    double release = 0.06;

    for (int i = 0; i < totalSamples; ++i) {
        double t = (double)i / sampleRate;
        double env = adsrEnvelope(t, totalDurSec, attack, decay, sustain, release);

        double value = sin(2.0 * M_PI * frequency * t);
        value += 0.30 * sin(2.0 * M_PI * 2.0 * frequency * t);
        value += 0.12 * sin(2.0 * M_PI * 3.0 * frequency * t);
        value += 0.06 * sin(2.0 * M_PI * 4.0 * frequency * t);
        value /= 1.5;

        samples[i] = static_cast<float>(env * value);
    }

    return samples;
}

MidiSynthesizer::MidiSynthesizer(QObject *parent)
    : QObject(parent),
    m_audioSink(nullptr),
    m_audioDevice(nullptr),
    m_sampleRate(44100),
    m_initialized(false),
    m_useInt16(false)
{
    qDebug() << "MidiSynthesizer created";
}

MidiSynthesizer::~MidiSynthesizer()
{
    if (m_audioSink) {
        m_audioSink->stop();
    }
}

bool MidiSynthesizer::ensureAudioReady()
{
    if (m_initialized) return true;

    qDebug() << "Initializing audio subsystem...";

    QAudioDevice outputDevice = QMediaDevices::defaultAudioOutput();
    if (outputDevice.isNull()) {
        qWarning() << "No audio output device available!";
        return false;
    }

    QAudioFormat format;
    format.setSampleRate(m_sampleRate);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Float);

    if (!outputDevice.isFormatSupported(format)) {
        qWarning() << "Float audio format not supported, falling back to Int16";
        format.setSampleFormat(QAudioFormat::Int16);
        m_useInt16 = true;

        if (!outputDevice.isFormatSupported(format)) {
            qWarning() << "Int16 also not supported! Trying preferred format.";
            format = outputDevice.preferredFormat();
            m_useInt16 = (format.sampleFormat() != QAudioFormat::Float);
        }
    }

    qDebug() << "Audio format: rate=" << format.sampleRate()
             << "ch=" << format.channelCount()
             << "fmt=" << format.sampleFormat()
             << "(useInt16=" << m_useInt16 << ")";

    m_audioSink = new QAudioSink(outputDevice, format, this);
    m_audioSink->setVolume(1.0);
    m_audioDevice = m_audioSink->start();

    if (!m_audioDevice) {
        qWarning() << "Failed to start audio sink (m_audioDevice is null)";
        return false;
    }

    if (m_audioSink->error() != QAudio::NoError) {
        qWarning() << "Audio sink error after start:" << m_audioSink->error();
        return false;
    }

    m_initialized = true;
    qDebug() << "Audio initialized successfully (push mode)";
    return true;
}

void MidiSynthesizer::playNote(int pitch, int velocity)
{
    if (!ensureAudioReady()) {
        qWarning() << "Cannot play note: audio not ready";
        return;
    }

    emit notePlaying(pitch);

    int durationMs = qMax(200, 600 - (pitch - 60) * 5);
    QVector<float> samples = WaveGenerator::generateNote(pitch, durationMs, m_sampleRate);

    float velocityScale = 0.4f + (velocity / 127.0f) * 0.6f;
    for (float &s : samples) {
        s *= velocityScale;
    }

    playSamples(samples);
}

void MidiSynthesizer::playSamples(const QVector<float> &samples)
{
    if (!m_audioDevice) return;

    if (m_useInt16) {
        QVector<qint16> converted(samples.size());
        for (int i = 0; i < samples.size(); ++i) {
            float clamped = qBound(-1.0f, samples[i], 1.0f);
            converted[i] = static_cast<qint16>(clamped * 32767.0f);
        }
        m_audioDevice->write(
            reinterpret_cast<const char*>(converted.constData()),
            static_cast<qint64>(converted.size()) * sizeof(qint16));
    } else {
        m_audioDevice->write(
            reinterpret_cast<const char*>(samples.constData()),
            static_cast<qint64>(samples.size()) * sizeof(float));
    }
}
