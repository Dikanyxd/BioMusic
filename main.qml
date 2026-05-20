import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: root
    width: 400
    height: 750
    visible: true
    title: "BioMusic Generator"

    property bool isPlaying: false
    property int currentBPM: 0
    property string currentMode: "work"
    property bool heartbeat: false
    property string statusText: "Ожидание подключения..."
    property string errorText: ""

    ListModel {
        id: devicesModel
    }

    color: "#0d0d1a"

    Timer {
        id: beatTimer
        interval: currentBPM > 0 ? Math.round(60000 / currentBPM) : 800
        running: isPlaying && btManager.isConnected && currentBPM > 0
        repeat: true
        onTriggered: {
            heartbeat = true
            beatOffTimer.restart()
        }
    }

    Timer {
        id: beatOffTimer
        interval: 120
        running: false
        repeat: false
        onTriggered: heartbeat = false
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: "#0d0d1a" }
            GradientStop { position: 1.0; color: "#0a0a14" }
        }
    }

    Flickable {
        anchors.fill: parent
        contentHeight: mainLayout.implicitHeight + 40
        clip: true

        ColumnLayout {
            id: mainLayout
            width: parent.width
            anchors.top: parent.top
            anchors.topMargin: 20
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 20
            anchors.rightMargin: 20
            spacing: 16

            Text {
                text: "💓 BioMusic"
                font.pixelSize: 30
                font.bold: true
                color: "#ff4d6d"
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: "Музыка из твоего сердцебиения"
                font.pixelSize: 12
                color: "#666688"
                Layout.alignment: Qt.AlignHCenter
            }

            Rectangle {
                Layout.fillWidth: true
                height: 44
                radius: 22
                color: btManager.isConnected ? "#0a2a1a" : "#1a0a2a"
                border.color: btManager.isConnected ? "#00cc66" : "#9933cc"
                border.width: 1

                RowLayout {
                    anchors.centerIn: parent
                    spacing: 10

                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: btManager.isConnected ? "#00ff88" : "#cc44ff"

                        SequentialAnimation on opacity {
                            running: !btManager.isConnected
                            loops: Animation.Infinite
                            NumberAnimation { to: 0.3; duration: 800 }
                            NumberAnimation { to: 1.0; duration: 800 }
                        }
                    }

                    Text {
                        text: btManager.isConnected
                              ? "🔗 " + btManager.deviceName
                              : "Не подключено"
                        color: btManager.isConnected ? "#00ff88" : "#cc88ff"
                        font.pixelSize: 13
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 36
                radius: 8
                color: "#2a0a0a"
                border.color: "#ff4444"
                border.width: 1
                visible: errorText !== ""

                Text {
                    anchors.centerIn: parent
                    text: "⚠ " + errorText
                    color: "#ff6666"
                    font.pixelSize: 12
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 220

                Rectangle {
                    anchors.centerIn: parent
                    width: 200
                    height: 200
                    radius: 100
                    color: "transparent"
                    border.color: "#ff4d6d"
                    border.width: 1
                    opacity: heartbeat ? 0.5 : 0.15
                    Behavior on opacity { NumberAnimation { duration: 80 } }
                    Behavior on scale { NumberAnimation { duration: 80 } }
                    scale: heartbeat ? 1.05 : 1.0
                }

                Rectangle {
                    anchors.centerIn: parent
                    width: 160
                    height: 160
                    radius: 80
                    color: "transparent"
                    border.color: "#ff4d6d"
                    border.width: 1.5
                    opacity: heartbeat ? 0.7 : 0.25
                    Behavior on opacity { NumberAnimation { duration: 80 } }
                }

                Rectangle {
                    anchors.centerIn: parent
                    width: 120
                    height: 120
                    radius: 60
                    color: heartbeat ? "#cc1133" : "#880022"
                    border.color: "#ff4d6d"
                    border.width: 2
                    Behavior on color { ColorAnimation { duration: 80 } }

                    Text {
                        anchors.centerIn: parent
                        text: currentBPM > 0 ? currentBPM : "--"
                        font.pixelSize: 38
                        font.bold: true
                        color: "white"
                    }
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    text: currentBPM > 0 ? "ударов/мин" : "нет сигнала"
                    color: "#666688"
                    font.pixelSize: 12
                }
            }

            Text {
                text: "Режим"
                color: "#888899"
                font.pixelSize: 12
                Layout.alignment: Qt.AlignHCenter
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Rectangle {
                    Layout.fillWidth: true
                    height: 56
                    radius: 12
                    color: currentMode === "work" ? "#cc1133" : "#141428"
                    border.color: currentMode === "work" ? "#ff4d6d" : "#333355"
                    border.width: 1

                    Behavior on color { ColorAnimation { duration: 150 } }

                    Column {
                        anchors.centerIn: parent
                        spacing: 2
                        Text { text: "💼"; font.pixelSize: 18; anchors.horizontalCenter: parent.horizontalCenter }
                        Text { text: "Работа"; font.pixelSize: 11; color: "white"; anchors.horizontalCenter: parent.horizontalCenter }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            currentMode = "work"
                            btManager.setMusicMode("work")
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 56
                    radius: 12
                    color: currentMode === "road" ? "#cc1133" : "#141428"
                    border.color: currentMode === "road" ? "#ff4d6d" : "#333355"
                    border.width: 1

                    Behavior on color { ColorAnimation { duration: 150 } }

                    Column {
                        anchors.centerIn: parent
                        spacing: 2
                        Text { text: "🚗"; font.pixelSize: 18; anchors.horizontalCenter: parent.horizontalCenter }
                        Text { text: "Дорога"; font.pixelSize: 11; color: "white"; anchors.horizontalCenter: parent.horizontalCenter }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            currentMode = "road"
                            btManager.setMusicMode("road")
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 56
                    radius: 12
                    color: currentMode === "sport" ? "#cc1133" : "#141428"
                    border.color: currentMode === "sport" ? "#ff4d6d" : "#333355"
                    border.width: 1

                    Behavior on color { ColorAnimation { duration: 150 } }

                    Column {
                        anchors.centerIn: parent
                        spacing: 2
                        Text { text: "🏃"; font.pixelSize: 18; anchors.horizontalCenter: parent.horizontalCenter }
                        Text { text: "Трениров."; font.pixelSize: 11; color: "white"; anchors.horizontalCenter: parent.horizontalCenter }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            currentMode = "sport"
                            btManager.setMusicMode("sport")
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 50
                radius: 25
                color: btManager.isConnected ? "#0a2a0a" : "#141428"
                border.color: btManager.isConnected ? "#00cc66" : "#5533cc"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: btManager.isConnected
                          ? "🔌 Отключиться"
                          : "🔍 Найти ESP32"
                    color: btManager.isConnected ? "#00ff88" : "#8866ff"
                    font.pixelSize: 14
                    font.bold: true
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (btManager.isConnected) {
                            btManager.disconnectDevice()
                        } else {
                            devicesModel.clear()
                            errorText = ""
                            statusText = "Поиск устройств..."
                            btManager.startDiscovery()
                        }
                    }
                }
            }

            Text {
                id: searchingText
                Layout.alignment: Qt.AlignHCenter
                text: statusText
                color: "#666688"
                font.pixelSize: 12
                visible: !btManager.isConnected
            }

            Rectangle {
                id: devicesPanel
                Layout.fillWidth: true
                visible: devicesModel.count > 0
                height: visible ? Math.min(devicesModel.count * 64 + 10, 220) : 0
                clip: true
                color: "#141428"
                radius: 12
                border.color: "#333355"
                border.width: 1

                Behavior on height { NumberAnimation { duration: 200 } }

                ListView {
                    id: devicesList
                    anchors.fill: parent
                    anchors.margins: 5
                    clip: true
                    model: devicesModel

                    delegate: Rectangle {
                        width: devicesList.width
                        height: 58
                        radius: 8
                        color: mouseArea.pressed ? "#2a1a4a" : "transparent"

                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 12
                            spacing: 2

                            Text {
                                text: name
                                color: "white"
                                font.pixelSize: 14
                                font.bold: true
                            }

                            Text {
                                text: address
                                color: "#666688"
                                font.pixelSize: 11
                            }
                        }

                        Text {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.rightMargin: 12
                            text: "Подключить →"
                            color: "#8866ff"
                            font.pixelSize: 12
                        }

                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            onClicked: {
                                statusText = "Подключение к " + name + "..."
                                btManager.connectToDevice(address)
                            }
                        }
                    }
                }
            }

            Item { Layout.preferredHeight: 20 }
        }
    }

    Connections {
        target: btManager

        function onBpmChanged() {
            currentBPM = btManager.currentBPM
            isPlaying = currentBPM > 0
        }

        function onDeviceDiscovered(name, address) {
            devicesModel.append({ "name": name, "address": address })
            statusText = "Найдено устройств: " + devicesModel.count
        }

        function onDiscoveryFinished() {
            if (devicesModel.count === 0) {
                statusText = "Устройства не найдены. Проверь Bluetooth и права доступа."
            } else {
                statusText = "Выберите устройство из списка"
            }
        }

        function onConnectedChanged() {
            if (btManager.isConnected) {
                statusText = "Подключено к " + btManager.deviceName
                devicesModel.clear()
            } else {
                isPlaying = false
                currentBPM = 0
                statusText = "Отключено"
            }
        }

        function onErrorOccurred(message) {
            errorText = message
            errorHideTimer.restart()
        }
    }

    Timer {
        id: errorHideTimer
        interval: 5000
        running: false
        repeat: false
        onTriggered: errorText = ""
    }

    Connections {
        target: synthesizer

        function onNotePlaying(pitch) {
            console.log("Playing MIDI pitch:", pitch)
        }
    }
}