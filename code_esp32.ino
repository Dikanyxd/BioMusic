#include <Wire.h>
#include "BluetoothSerial.h"
#include "MAX30105.h"
#include "heartRate.h"

BluetoothSerial SerialBT;
MAX30105 sensor;

#define RATE_SIZE         8      // Сколько ударов усредняем
#define MIN_BPM           40
#define MAX_BPM           180
#define IR_THRESHOLD      50000L 
#define BPM_INTERVAL      1000   // Отправка BPM/мс
#define NOTE_DENSITY      85     // Вероятность ноты в % (0-100)

//номера нот
#define SCALE_SIZE 8
const int SCALE_WORK[SCALE_SIZE]  = {60,62,64,65,67,69,71,72}; 
const int SCALE_ROAD[SCALE_SIZE]  = {48,50,52,53,55,57,59,60}; 
const int SCALE_SPORT[SCALE_SIZE] = {72,74,76,77,79,81,83,84}; 

enum Mode { MODE_WORK, MODE_ROAD, MODE_SPORT };
Mode currentMode = MODE_WORK;

float         bpmFiltered    = 75.0f;
float         beatAvg        = 75.0f;
bool          fingerPresent  = false;
int           lastNoteIndex  = 0;
unsigned long lastBeatTime   = 0;
unsigned long lastBpmSent    = 0;
unsigned long lastNoteTime   = 0;
String        cmdBuffer      = "";

// Для скользящего среднего
byte          rates[RATE_SIZE] = {0};
int           rateSpot       = 0;

void setup() {
  Serial.begin(115200);

  // Bluetooth 
  SerialBT.begin("BioMusic_ESP32");
  Serial.println("BT ready: BioMusic_ESP32");


  Wire.begin(21, 22);
  if (!sensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found! Check: VIN->3V3, SDA->21, SCL->22");
    pinMode(2, OUTPUT);
    while (1) {
      digitalWrite(2, HIGH); delay(300);
      digitalWrite(2, LOW);  delay(300);
    }
  }
  Serial.println("MAX30102 OK");

  // Настройка датчика
  sensor.setup();
  sensor.setPulseAmplitudeRed(0x1F);
  sensor.setPulseAmplitudeIR(0x1F);

  randomSeed(analogRead(34));

  Serial.println("Ready. Put finger on sensor.");
}

void loop() {
  handleCommands();    
  handleHeartRate();   
  sendBpmToPhone();     
  generateMusic();      
}

void handleHeartRate() {
  long irValue = sensor.getIR();


  bool fingerNow = (irValue > IR_THRESHOLD);

  if (fingerNow != fingerPresent) {
    fingerPresent = fingerNow;

    if (fingerPresent) {
      Serial.println("Finger detected");

      // Полный сброс фильтра при новом контакте
      for (int i = 0; i < RATE_SIZE; i++) rates[i] = 0;
      rateSpot = 0;
      beatAvg = 0;
      bpmFiltered = 0.0f;
      lastBeatTime = 0;
      lastNoteTime = 0;

    } else {
      Serial.println("Finger removed");

      // Сбрасываем всё, если пальца нету на датчике
      for (int i = 0; i < RATE_SIZE; i++) rates[i] = 0;
      rateSpot = 0;
      beatAvg = 0;
      bpmFiltered = 0.0f;
      lastBeatTime = 0;
      lastNoteTime = 0;
    }
}

  if (!fingerPresent) return;

  if (checkForBeat(irValue)) {
    unsigned long now   = millis();
    unsigned long delta = now - lastBeatTime;

    // Защита: дельта 250мс..2000мс = 30..240 BPM
    if (delta > 250 && delta < 2000 && lastBeatTime != 0) {
      float bpm = 60000.0f / delta; // мс → BPM

      if (bpm >= MIN_BPM && bpm <= MAX_BPM) {
        rates[rateSpot] = (byte)bpm;
        rateSpot = (rateSpot + 1) % RATE_SIZE;

        // Скользящее среднее по 8 ударам
        long sum = 0;
        for (int i = 0; i < RATE_SIZE; i++) sum += rates[i];
        beatAvg = sum / (float)RATE_SIZE;

        // EMA-сглаживание 
        bpmFiltered = 0.15f * beatAvg + 0.85f * bpmFiltered;
      }
    }
    lastBeatTime = now;
  }
}

void sendBpmToPhone() {
  unsigned long now = millis();
  if (now - lastBpmSent < BPM_INTERVAL) return;
  lastBpmSent = now;

  int bpmToSend = 0;
  if (fingerPresent && bpmFiltered >= MIN_BPM && bpmFiltered <= MAX_BPM) {
    bpmToSend = (int)bpmFiltered;
  }

  String msg = "BPM:" + String(bpmToSend);
  SerialBT.println(msg);  
  Serial.println(msg);
}

void generateMusic() {
  if (!fingerPresent || !SerialBT.hasClient()) return;

  unsigned long interval = calcInterval();
  unsigned long now = millis();
  if (now - lastNoteTime < interval) return;
  lastNoteTime = now;

  if (random(100) >= NOTE_DENSITY) return;

  int noteIdx = pickNoteIndex();
  lastNoteIndex = noteIdx;

  const int* scale = getScale();
  int note     = constrain(scale[noteIdx], 0, 127);
  int velocity = calcVelocity();

  String msg = "NOTE:" + String(note) + "," + String(velocity);
  SerialBT.println(msg);
  Serial.println(">> " + msg);
}

unsigned long calcInterval() {
  float t = (bpmFiltered - MIN_BPM) / (float)(MAX_BPM - MIN_BPM);
  t = constrain(t, 0.0f, 1.0f);
  int base = (int)(700.0f - t * 450.0f); 

  float mult;
  switch (currentMode) {
    case MODE_WORK:  mult = 1.3f; break;
    case MODE_ROAD:  mult = 1.0f; break;
    case MODE_SPORT: mult = 0.6f; break;
    default:         mult = 1.0f;
  }
  return (unsigned long)max(100, (int)(base * mult));
}

int pickNoteIndex() {
  int offset = random(-2, 3);
  int idx    = constrain(lastNoteIndex + offset, 0, SCALE_SIZE - 1);

  if (random(10) < 2) idx = random(0, SCALE_SIZE);

  return idx;
}

int calcVelocity() {
  float t = (bpmFiltered - MIN_BPM) / (float)(MAX_BPM - MIN_BPM);
  t = constrain(t, 0.0f, 1.0f);
  int vel = (int)(55.0f + t * 65.0f); // 55..120
  vel += random(-5, 6);                // ±5 для живости
  return constrain(vel, 1, 127);
}

const int* getScale() {
  switch (currentMode) {
    case MODE_ROAD:  return SCALE_ROAD;
    case MODE_SPORT: return SCALE_SPORT;
    default:         return SCALE_WORK;
  }
}

void handleCommands() {
  while (SerialBT.available()) {
    char c = SerialBT.read();
    if (c == '\n' || c == '\r') {
      if (cmdBuffer.length() > 0) {
        processCommand(cmdBuffer);
        cmdBuffer = "";
      }
    } else {
      if (cmdBuffer.length() < 64) cmdBuffer += c;
    }
  }
}

void processCommand(const String& cmd) {
  String s = cmd;
  s.trim();

  Serial.println("<< CMD: " + s);

  if (s == "MODE_WORK") {
    currentMode   = MODE_WORK;
    lastNoteIndex = 4; // Середина гаммы
    SerialBT.println("OK:MODE_WORK");

  } else if (s == "MODE_ROAD") {
    currentMode   = MODE_ROAD;
    lastNoteIndex = 0;
    SerialBT.println("OK:MODE_ROAD");

  } else if (s == "MODE_SPORT") {
    currentMode   = MODE_SPORT;
    lastNoteIndex = SCALE_SIZE - 1; 
    SerialBT.println("OK:MODE_SPORT");

  } else if (s == "STATUS") {
  
    String modeStr = (currentMode == MODE_WORK)  ? "WORK"  :
                     (currentMode == MODE_ROAD)  ? "ROAD"  : "SPORT";
    String resp = "STATUS:mode=" + modeStr
                + ",bpm=" + String((int)bpmFiltered)
                + ",finger=" + String(fingerPresent ? 1 : 0);
    SerialBT.println(resp);
    Serial.println(resp);

  } else if (s == "PING") {
    SerialBT.println("PONG");

  } else {
    SerialBT.println("ERR:unknown:" + s);
  }
}
