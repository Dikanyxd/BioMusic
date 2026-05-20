# 🎵 BioMusic Generator  
### Биомузыкальный синтезатор на основе сердечного ритма

![Platform](https://img.shields.io/badge/platform-ESP32%20%7C%20Android-0A84FF?style=for-the-badge)
![Bluetooth](https://img.shields.io/badge/Bluetooth-Classic-34C759?style=for-the-badge)
![Qt](https://img.shields.io/badge/Qt-6.x-41CD52?style=for-the-badge)
![License](https://img.shields.io/badge/license-MIT-FFD60A?style=for-the-badge)

---

## 💡 О проекте

**BioMusic Generator** — это интерактивная система, которая превращает сердцебиение человека в музыку в реальном времени.

Датчик **MAX30102** считывает фотоплетизмограмму (PPG) с пальца, микроконтроллер **ESP32** вычисляет частоту сердечных сокращений (BPM), а затем генерирует MIDI-ноты, передаваемые по Bluetooth на Android-приложение для синтеза звука.

> *«Ваше сердце — дирижёр, а музыка отражает ваше состояние»*

---

# ✨ Возможности

| Возможность | Описание |
|---|---|
| 💓 **Измерение пульса в реальном времени** | Определение BPM с помощью MAX30102 |
| 🎹 **Алгоритмическая генерация музыки** | Генерация MIDI-нот на основе сердечного ритма |
| 📱 **Bluetooth Classic** | Передача MIDI-команд на Android без заметной задержки |
| 🎚️ **Адаптивная динамика** | Громкость и интенсивность музыки зависят от пульса |
| 🎨 **Визуальная синхронизация** | Интерфейс пульсирует в такт сердцебиению |
| ⚡ **Низкая задержка** | Работа в реальном времени на ESP32 |

---

# 🎼 Музыкальные режимы

| Режим | Диапазон MIDI | Темп | Характер |
|---|---|---|---|
| 💼 **Work** | C4–C5 (60–72) | Медленный | Спокойная фоновая музыка |
| 🚗 **Road** | C3–C4 (48–60) | Средний | Ритмичный и предсказуемый бас |
| 🏃 **Sport** | C5–C6 (72–84) | Быстрый | Энергичные арпеджио |

---

# 🛠️ Технологический стек

## Аппаратная часть

| Компонент | Назначение |
|---|---|
| ESP32 (NodeMCU-32S) | Основной микроконтроллер |
| MAX30102 | Оптический датчик пульса |
| I2C (GPIO21 / GPIO22) | Связь ESP32 с датчиком |
| Bluetooth Classic | Передача MIDI-сообщений |

---

## Программная часть

| Компонент | Технологии |
|---|---|
| **ESP32 Firmware** | C++, Arduino Framework, BluetoothSerial |
| **Обработка пульса** | MAX30105, heartRate, скользящее среднее |
| **Музыкальная логика** | MIDI, генерация паттернов, гаммы |
| **Android-приложение** | Qt 6, QML, QtBluetooth, QtMultimedia |

---

# 📊 Архитектура системы

```text
Палец
   ↓
MAX30102
   ↓
ESP32
   ↓
Анализ BPM + генерация MIDI
   ↓
Bluetooth Classic
   ↓
Qt Android App
   ↓
Синтез звука
   ↓
Динамики телефона
```

---

# 🔌 Схема подключения

| MAX30102 | ESP32 |
|---|---|
| VIN | 3V3 |
| GND | GND |
| SDA | GPIO21 |
| SCL | GPIO22 |

---

# 🚀 Быстрый старт

## 1️⃣ Установка библиотек Arduino

Установите библиотеки через **Arduino IDE → Library Manager**:

```text
MAX30105 by SparkFun
```

> Библиотека `BluetoothSerial` входит в пакет ESP32 Arduino Core.

---

## 2️⃣ Прошивка ESP32

Настройки Arduino IDE:

| Параметр | Значение |
|---|---|
| Board | NodeMCU-32S |
| Upload Speed | 115200 |

После настройки нажмите **Upload**.

---

## 3️⃣ Сборка Android-приложения

```bash
git clone https://github.com/Dikanyxd/BioMusic.git

cd BioMusic

qmake
make
```

После сборки установите APK на Android-устройство.

---

# 📂 Структура проекта

```text
BioMusic/
├── esp32_firmware/
│   └── biomusic.ino          # Прошивка ESP32
│
├── android/
│   ├── AndroidManifest.xml   # Android permissions
│   ├── build.gradle
│   └── ...
│
├── bluetoothmanager.cpp      # Bluetooth менеджер
├── bluetoothmanager.h
│
├── midisynthesizer.cpp       # MIDI-синтезатор
├── midisynthesizer.h
│
├── main.cpp                  # Точка входа Qt
├── main.qml                  # Интерфейс QML
│
├── newbio.pro                # Qt project
├── qml.qrc
├── qtquickcontrols2.conf
│
└── README.md
```

---

# 🎓 Для чего подходит проект

| Область | Описание |
|---|---|
| 🎓 Обучение | Изучение ESP32, I2C, Bluetooth и Qt |
| 🎵 Biofeedback Art | Превращение биосигналов в музыку |
| 🧠 DSP / Signal Processing | Работа с PPG и BPM |
| 📱 Embedded + Mobile | Связь микроконтроллера и Android |

---

# 📌 Возможные улучшения

- 🎹 Поддержка настоящего MIDI over BLE
- 🌈 Визуализация пульсовой волны
- 🤖 Генерация музыки через AI/ML
- ☁️ Сохранение BPM-данных в облако
- 🎧 Поддержка внешних MIDI-синтезаторов

---

# 🤝 Лицензия

Проект распространяется под лицензией **MIT License**.  
Вы можете свободно использовать, изменять и распространять проект.

---

# 📬 Контакты

- Telegram: `@Dikanyy`
- Telegram: `@evgenpelmen8`

---


---

> *BioMusic Generator — когда технологии начинают чувствовать ритм жизни.*
