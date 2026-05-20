#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>

#include "bluetoothmanager.h"
#include "midisynthesizer.h"

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QGuiApplication app(argc, argv);

    app.setOrganizationName("BioMusic");
    app.setApplicationName("BioMusic Generator");
    app.setApplicationVersion("1.0");

    QQmlApplicationEngine engine;

    BluetoothManager btManager;
    MidiSynthesizer synthesizer;

    QObject::connect(&btManager, &BluetoothManager::noteReceived,
                     &synthesizer, &MidiSynthesizer::playNote);

    engine.rootContext()->setContextProperty("btManager", &btManager);
    engine.rootContext()->setContextProperty("synthesizer", &synthesizer);

    const QUrl url(QStringLiteral("qrc:/main.qml"));

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                qCritical() << "Failed to load QML!";
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection
        );

    engine.load(url);

    return app.exec();
}