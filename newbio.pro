TARGET = newbio
CONFIG += skip_target_version_ext

QT += quick bluetooth multimedia quickcontrols2

CONFIG += c++17

SOURCES += \
    main.cpp \
    bluetoothmanager.cpp \
    midisynthesizer.cpp

HEADERS += \
    bluetoothmanager.h \
    midisynthesizer.h

RESOURCES += qml.qrc

android {
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
    ANDROID_MIN_SDK_VERSION     = 28
    ANDROID_TARGET_SDK_VERSION  = 33
    ANDROID_COMPILE_SDK_VERSION = 33
    ANDROID_PLATFORM            = android-33
    ANDROID_PACKAGE_NAME        = com.biomusic.generator
}

DEFINES += QT_DEPRECATED_WARNINGS

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/res/values/libs.xml \
    android/res/xml/qtprovider_paths.xml