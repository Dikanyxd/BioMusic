#ifndef MIDISYNTHESIZER_H
#define MIDISYNTHESIZER_H

#include <QObject>
#include <QVector>
#include <QAudioSink>
#include <QAudioFormat>
#include <QIODevice>

class WaveGenerator {
public:
    static QVector<float> generateNote(int pitch, int durationMs, int sampleRate = 44100);

private:
    static double adsrEnvelope(double t, double totalDur,
                               double attack, double decay,
                               double sustain, double release);
};

class MidiSynthesizer : public QObject
{
    Q_OBJECT

public:
    explicit MidiSynthesizer(QObject *parent = nullptr);
    ~MidiSynthesizer();

    Q_INVOKABLE void playNote(int pitch, int velocity);

    static double pitchToFrequency(int pitch);

signals:
    void notePlaying(int pitch);

private:
    bool ensureAudioReady();
    void playSamples(const QVector<float> &samples);

    QAudioSink *m_audioSink;
    QIODevice  *m_audioDevice;
    int  m_sampleRate;
    bool m_initialized;
    bool m_useInt16;
};

#endif
