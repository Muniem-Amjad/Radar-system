/*
  Arduino Radar System — Wokwi Ready
  Buzzer uses tone() for proper audio output in Wokwi
  Components: HC-SR04 | SG90 Servo | SSD1306 OLED | Buzzer
  Libraries:
    - Adafruit SSD1306
    - Adafruit GFX Library
    - Servo (built-in)
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

// ── OLED Config ───────────────────────────────────────────────
#define SCREEN_W   128
#define SCREEN_H    64
#define OLED_ADDR  0x3C
Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);

// ── Pin Definitions ───────────────────────────────────────────
#define TRIG_PIN    9
#define ECHO_PIN   10
#define SERVO_PIN   6
#define BUZZER_PIN  5

// ── Radar Config ──────────────────────────────────────────────
#define MAX_DIST      150
#define SWEEP_DELAY     8

// Proximity zones
#define ZONE_CRITICAL  30
#define ZONE_CLOSE     70
#define ZONE_MID      120

// Tone frequencies
#define FREQ_CRITICAL  1200   // high pitch
#define FREQ_CLOSE      800   // medium pitch
#define FREQ_NEAR       500   // low pitch

// Radar arc origin
#define CX  64
#define CY  63
#define R   58

Servo radarServo;

int  currentAngle = 0;
int  sweepDir     = 1;
int  lastAngle    = 0;
long lastDist     = 0;

// Non-blocking tone timing
unsigned long toneTimer    = 0;
bool          tonePlaying  = false;

// ── Helpers ───────────────────────────────────────────────────

long measureCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long dur = pulseIn(ECHO_PIN, HIGH, 30000);
  if (dur == 0) return MAX_DIST + 1;
  return dur / 58;
}

void getZoneParams(long dist, int &freq, int &interval) {
  if (dist < ZONE_CRITICAL) {
    freq = FREQ_CRITICAL;
    interval = 150;
  } else if (dist < ZONE_CLOSE) {
    freq = FREQ_CLOSE;
    interval = 400;
  } else if (dist < ZONE_MID) {
    freq = FREQ_NEAR;
    interval = 800;
  } else {
    freq = 0;
    interval = 0;
  }
}

void updateBuzzer(long dist) {
  int freq, interval;
  getZoneParams(dist, freq, interval);

  if (interval == 0) {
    noTone(BUZZER_PIN);
    tonePlaying = false;
    return;
  }

  unsigned long now = millis();

  if (!tonePlaying && (now - toneTimer >= (unsigned long)interval)) {
    tone(BUZZER_PIN, freq);
    tonePlaying = true;
    toneTimer = now;
  } else if (tonePlaying && (now - toneTimer >= 80)) {
    noTone(BUZZER_PIN);
    tonePlaying = false;
    toneTimer = now;
  }
}

void polarToXY(int angleDeg, float frac, int &x, int &y) {
  float rad = radians(180 - angleDeg);
  x = CX + (int)(frac * R * cos(rad));
  y = CY - (int)(frac * R * sin(rad));
}

void drawRadarFrame() {
  display.clearDisplay();
  for (int i = 1; i <= 3; i++) {
    int r = (R * i) / 3;
    for (int a = 0; a <= 180; a += 2) {
      float rad = radians(180 - a);
      int x = CX + (int)(r * cos(rad));
      int y = CY - (int)(r * sin(rad));
      display.drawPixel(x, y, SSD1306_WHITE);
    }
  }
  display.drawFastHLine(CX - R - 2, CY, (R + 2) * 2 + 1, SSD1306_WHITE);
  display.drawPixel(CX, CY, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(CX + R/3 + 2, CY - 8);
  display.print("50");
  display.setCursor(CX + (2*R)/3 + 2, CY - 8);
  display.print("100");
}

void drawSweepLine(int angleDeg) {
  int x, y;
  polarToXY(angleDeg, 1.0, x, y);
  display.drawLine(CX, CY, x, y, SSD1306_WHITE);
}

void drawBlip(int angleDeg, long distCm) {
  if (distCm > MAX_DIST) return;
  float frac = constrain((float)distCm / MAX_DIST, 0.05, 1.0);
  int x, y;
  polarToXY(angleDeg, frac, x, y);
  display.fillCircle(x, y, 2, SSD1306_WHITE);
}

void drawZoneLabel(long distCm) {
  if (distCm >= ZONE_MID) return;
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(72, 56);
  if      (distCm < ZONE_CRITICAL) display.print("!!!CRIT");
  else if (distCm < ZONE_CLOSE)    display.print("!!CLOSE");
  else                             display.print("! NEAR ");
}

// ── Setup ──────────────────────────────────────────────────────

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  radarServo.attach(SERVO_PIN);
  radarServo.write(0);
  delay(300);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("SSD1306 not found!"));
    while (true);
  }

  // 3 rising beeps at startup to confirm buzzer works
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(14, 20);
  display.print(F("RADAR INITIALISING"));
  display.setCursor(22, 36);
  display.print(F("Buzzer test..."));
  display.display();

  tone(BUZZER_PIN, 500);  delay(150); noTone(BUZZER_PIN); delay(100);
  tone(BUZZER_PIN, 800);  delay(150); noTone(BUZZER_PIN); delay(100);
  tone(BUZZER_PIN, 1200); delay(150); noTone(BUZZER_PIN); delay(400);
}

// ── Main Loop ──────────────────────────────────────────────────

void loop() {
  radarServo.write(currentAngle);
  delay(SWEEP_DELAY);

  long dist = measureCm();

  updateBuzzer(dist);

  drawRadarFrame();
  drawSweepLine(currentAngle);

  if (lastDist <= MAX_DIST) drawBlip(lastAngle, lastDist);

  if (dist <= MAX_DIST) {
    drawBlip(currentAngle, dist);
    lastAngle = currentAngle;
    lastDist  = dist;
  }

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 56);
  display.print(currentAngle);
  display.print((char)247);
  display.print(" ");
  if (dist <= MAX_DIST) {
    display.print(dist);
    display.print("cm");
  } else {
    display.print("--  ");
  }

  drawZoneLabel(dist);
  display.display();

  Serial.print(currentAngle);
  Serial.print(",");
  Serial.print(dist);
  Serial.print(",zone:");
  if      (dist < ZONE_CRITICAL) Serial.println("CRITICAL");
  else if (dist < ZONE_CLOSE)    Serial.println("CLOSE");
  else if (dist < ZONE_MID)      Serial.println("NEAR");
  else                           Serial.println("CLEAR");

  currentAngle += sweepDir;
  if (currentAngle >= 180) sweepDir = -1;
  if (currentAngle <= 0)   sweepDir =  1;
}