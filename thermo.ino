#include <max6675.h>

int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
//int vccPin = 3;
//int gndPin = 2;

int relayPin = 3;

float targetTemp = 48.0f;
bool heating;

void setup() {
  Serial.begin(9600);
  pinMode(relayPin, OUTPUT);
  heaterOff();
  Serial.println("Thermocouple feedback test");
  // wait for MAX chip to stabilize
  delay(500);
  digitalWrite(relayPin, HIGH);
}

#define STAGE_PREWARM 0
#define STAGE_WAIT_FOR_PEAK 1
#define STAGE_MAINTAIN_TEMP 2

int stage = STAGE_MAINTAIN_TEMP;
//int stage = STAGE_PREWARM;
int skip = 0;
float peak = 0;
int postPeakCount = 0;


void loop() {
  unsigned long tick = 0;
  float temp = thermocouple.readCelsius();
  float remaining = targetTemp - temp;
  Serial.print("temp:");
  Serial.print(temp);
  Serial.print(", remaining:");
  Serial.print(remaining);
  Serial.print(", stage:");
  Serial.print(stage);
  Serial.print(", heating:");
  Serial.println(heating);
  if (skip > 0) {
    skip--;
  }
  else {
    int oldStage = stage;
    switch (stage) {
      case STAGE_PREWARM:
        prewarm(remaining);
        break;
      case STAGE_WAIT_FOR_PEAK:
        waitForPeak(temp);
        break;
      case STAGE_MAINTAIN_TEMP:
        maintainTemp(temp, remaining);
        break;
    }
    if (stage != oldStage) {
      Serial.print("New stage:");
      Serial.println(stage);
    }
  }
  delay(5000);
  tick++;
}

void prewarm(int remaining) {
  if (remaining > 22) {
    heaterOn();
  }
  else {
    heaterOff();
    stage = STAGE_WAIT_FOR_PEAK;
  }
}

void waitForPeak(float temp) {
  heaterOff();
  if (temp > peak) {
    postPeakCount = 0;
    peak = temp;
  }
  else if (temp < peak) {
    postPeakCount++;
  }
  if (postPeakCount > 10) {
    peak = 0;
    postPeakCount = 0;
    stage = STAGE_MAINTAIN_TEMP;
  }
}

void maintainTemp(float temp, float remaining) {
  if (remaining < 1.25f) {
    heaterOff();
    skip = 10;
  }
  else {
    if (heating) {
      heaterOff();
      skip = 13;
    }
    else {
      heaterOn();
      skip = 3;
    }
  }
}

void heaterOn() {
  if (heating)
    return;
  heating = true;
  digitalWrite(relayPin, LOW);
  Serial.println("Turning ON heater");
}

void heaterOff() {
  if (!heating)
    return;
  heating = false;
  digitalWrite(relayPin, HIGH);
  Serial.println("Turning OFF heater");
}

