/*
  4DOF servo robot arm: base yaw, shoulder, elbow, wrist pitch
  Servos: Miuzei 9g metal gear 180 deg (MG90S-type)

  Serial commands (115200 baud, newline terminated):
    P x y z phi      Move tool tip to x,y,z [mm], tool pitch phi [deg] (0 = horizontal, -90 = pointing down)
    A s0 s1 s2 s3    Set raw servo angles [deg] (for calibration)
    H                Home position
    ?                Print current servo angles

  Coordinate frame:
    Origin = base rotation axis at base plate level
    x = forward, y = left, z = up
    Joint angles: shoulder from horizontal, elbow relative to upper arm,
                  wrist relative to forearm (positive = up)
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

const int SDA_PIN = 23;
const int SCL_PIN = 22;

const int US_MIN = 500;    // pulse at 0 deg
const int US_MAX = 2500;   // pulse at 180 deg (reduce if servo buzzes at end)
const uint8_t NUM_SERVOS = 4;

Adafruit_PWMServoDriver pwm(0x40, Wire);

void setAngle(uint8_t ch, float deg) {
  deg = constrain(deg, 0, 180);
  uint16_t us = US_MIN + deg / 180.0 * (US_MAX - US_MIN);
  pwm.writeMicroseconds(ch, us);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(SDA_PIN, SCL_PIN);

  // Check the PCA9685 answers on I2C
  Wire.beginTransmission(0x40);
  if (Wire.endTransmission() != 0) {
    Serial.println("PCA9685 not found at 0x40 - check wiring");
    while (true) delay(1000);
  }

  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(50);   // 50 Hz servo frame
  delay(10);

  for (uint8_t ch = 0; ch < NUM_SERVOS; ch++) {
    setAngle(ch, 90);
    delay(300);         // stagger startup current
  }
  Serial.println("Ready. Send: <channel> <angle>, e.g. 0 45");
}

void loop() {
  if (Serial.available()) {
    int ch = Serial.parseInt();
    float deg = Serial.parseFloat();
    while (Serial.available()) Serial.read();   // flush line ending

    if (ch < 0 || ch >= NUM_SERVOS) {
      Serial.println("Channel out of range");
      return;
    }
    setAngle(ch, deg);
    Serial.print("Ch "); Serial.print(ch);
    Serial.print(" -> "); Serial.println(deg);
  }
}