# Arduino Radar System

A real-time radar simulation built with Arduino UNO, ultrasonic sensor, servo motor,
OLED display, and proximity buzzer. Fully simulated on Wokwi.

---


## Components

| Component          | Model             | Purpose                        |
|--------------------|-------------------|--------------------------------|
| Microcontroller    | Arduino UNO       | Main controller                |
| Ultrasonic Sensor  | HC-SR04           | Measures object distance       |
| Servo Motor        | SG90              | Sweeps sensor 0 to 180 degrees |
| OLED Display       | SSD1306 128x64    | Shows radar screen             |
| Buzzer             | Passive Buzzer    | Proximity audio alert          |

---

## Circuit Connections

### HC-SR04 Ultrasonic Sensor

| Sensor Pin | Arduino Pin |
|------------|-------------|
| VCC        | 5V          |
| GND        | GND         |
| TRIG       | Pin 9       |
| ECHO       | Pin 10      |

### SG90 Servo Motor

| Servo Pin    | Arduino Pin |
|--------------|-------------|
| V+ (Red)     | 5V          |
| GND (Brown)  | GND         |
| PWM (Orange) | Pin 6       |

### SSD1306 OLED Display (I2C)

| OLED Pin | Arduino Pin |
|----------|-------------|
| VCC      | 3.3V        |
| GND      | GND         |
| SDA      | A4          |
| SCL      | A5          |

### Passive Buzzer

| Buzzer Pin    | Arduino Pin |
|---------------|-------------|
| Positive (+)  | Pin 5       |
| Negative (-)  | GND         |

---

## Proximity Detection Zones

| Zone     | Distance     | Beep Speed      | OLED Label |
|----------|--------------|-----------------|------------|
| CRITICAL | 0 to 29 cm   | Very rapid 80ms | !!! CRIT   |
| CLOSE    | 30 to 69 cm  | Fast 280ms      | !! CLOSE   |
| NEAR     | 70 to 119 cm | Slow 580ms      | ! NEAR     |
| CLEAR    | 120 cm+      | Silent          | none       |

Objects beyond 150 cm are ignored completely with no blip and no beep.

---

## Project Structure

```
arduino-radar/
│
├── radar.ino          # Main Arduino sketch
├── diagram.json       # Wokwi simulation layout
└── README.md          # Project documentation
```

---

## Libraries Required

Install these from the Arduino Library Manager or the Wokwi Library panel:

- Adafruit SSD1306 v2.5.7 — https://github.com/adafruit/Adafruit_SSD1306
- Adafruit GFX Library v1.11.5 — https://github.com/adafruit/Adafruit-GFX-Library
- Servo — built-in with Arduino IDE
- Wire — built-in with Arduino IDE

---

## How to Run on Wokwi Simulator

1. Go to https://wokwi.com and create a new Arduino UNO project
2. Replace diagram.json with the one from this repository
3. Replace sketch.ino with radar.ino from this repository
4. Add libraries: Adafruit SSD1306 and Adafruit GFX Library
5. Click Play
6. Click the HC-SR04 sensor and drag the distance slider to test zones
7. Watch the radar sweep on the OLED and listen for buzzer beeps

---

## How to Run on Real Hardware

1. Wire all components as per the circuit tables above
2. Open radar.ino in Arduino IDE
3. Install required libraries via Tools > Manage Libraries
4. Select Board: Arduino UNO and the correct COM port
5. Click Upload
6. Point the sensor at objects and observe the OLED radar display

---

## Customization

Detection zones can be adjusted by editing these defines in radar.ino:

```cpp
#define ZONE_CRITICAL  30    // cm — critical range threshold
#define ZONE_CLOSE     70    // cm — close range threshold
#define ZONE_MID      120    // cm — near range threshold
#define MAX_DIST      150    // cm — objects beyond this are ignored
```

Buzzer frequencies can be adjusted here:

```cpp
#define FREQ_CRITICAL  1200  // Hz — high pitch
#define FREQ_CLOSE      800  // Hz — medium pitch
#define FREQ_NEAR       500  // Hz — low pitch
```

Sweep speed can be adjusted here:

```cpp
#define SWEEP_DELAY  8  // ms per degree — lower value means faster sweep
```

---

## How It Works

The servo motor rotates continuously between 0 and 180 degrees. At each degree
position the HC-SR04 ultrasonic sensor fires a sound pulse. The pulse bounces
off any object in front of it and returns to the sensor. The Arduino measures
how long the pulse took to return and converts that time into a distance in
centimetres. That distance is then mapped to a position on the OLED radar arc
as a blip dot. The closer the object, the faster the buzzer beeps.

---

## Serial Monitor Output

The system outputs data over Serial at 9600 baud in the following format:

```
angle,distance,zone
45,60,zone:CLOSE
46,60,zone:CLOSE
47,28,zone:CRITICAL
```

---

## Built With

- Arduino UNO — https://www.arduino.cc
- Wokwi Simulator — https://wokwi.com
- Adafruit SSD1306 Library — https://github.com/adafruit/Adafruit_SSD1306
- Adafruit GFX Library — https://github.com/adafruit/Adafruit-GFX-Library

---

## License

This project is open source and available under the MIT License.

---

## Author

Developed using Arduino and the Wokwi online simulator by Muniem Amjad