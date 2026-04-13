#include "Adafruit_VL53L0X.h"
#include <Wire.h>

// address we will assign if dual sensor is present
#define LOX1_ADDRESS 0x30
#define LOX2_ADDRESS 0x31

// board-safe defaults 

#if defined(ESP32)
  #define SDA_PIN 21
  #define SCL_PIN 22
  #define SHT_LOX1 4
  #define SHT_LOX2 14
#else
  #error "Unsupported MCU for this sketch. Define SDA_PIN, SCL_PIN, SHT_LOX1, SHT_LOX2."
#endif

// objects for the vl53l0x
Adafruit_VL53L0X lox1 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox2 = Adafruit_VL53L0X();
bool sensorsReady = false;

/*
    Reset all sensors by setting all of their XSHUT pins low for delay(10), then set all XSHUT high to bring out of reset
    Keep sensor #1 awake by keeping XSHUT pin high
    Put all other sensors into shutdown by pulling XSHUT pins low
    Initialize sensor #1 with lox.begin(new_i2c_address) Pick any number but 0x29 and it must be under 0x7F. Going with 0x30 to 0x3F is probably OK.
    Keep sensor #1 awake, and now bring sensor #2 out of reset by setting its XSHUT pin high.
    Initialize sensor #2 with lox.begin(new_i2c_address) Pick any number but 0x29 and whatever you set the first sensor to
 */

void setID() {
  // Keep both sensors in reset so they never collide on default address 0x29.
  digitalWrite(SHT_LOX1, LOW);
  digitalWrite(SHT_LOX2, LOW);
  delay(30);

  // Bring up LOX1 alone, then move it away from default address.
  digitalWrite(SHT_LOX1, HIGH);
  delay(30);
  if (!lox1.begin(LOX1_ADDRESS)) {
    Serial.println(F("Failed to boot first VL53L0X (check LOX1 XSHUT/wiring)"));
    sensorsReady = false;
    return;
  }

  // Bring up LOX2 after LOX1 already has a unique address.
  digitalWrite(SHT_LOX2, HIGH);
  delay(30);
  if (!lox2.begin(LOX2_ADDRESS)) {
    Serial.println(F("Failed to boot second VL53L0X (check LOX2 XSHUT/wiring)"));
    sensorsReady = false;
    return;
  }

  lox1.startRangeContinuous(); // puts in continuous mode (reads constantly)
  lox2.startRangeContinuous();
  sensorsReady = true;
  Serial.println(F("Both VL53L0X sensors initialized."));
}

void read_dual_sensors() {
  if (!sensorsReady) {
    return;
  }
  
  if (lox1.isRangeComplete()) {
    int distance1 = lox1.readRange();
    if (distance1 <= 0 || distance1 >= 8190) {
      Serial.println("Distance 1: invalid");
    } else {
      Serial.print("Distance 1: ");
      Serial.println(distance1);
    }
  }

  if (lox2.isRangeComplete()) {
    int distance2 = lox2.readRange();
    if (distance2 <= 0 || distance2 >= 8190) {
      Serial.println("Distance 2: invalid");
    } else {
      Serial.print("Distance 2: ");
      Serial.println(distance2);
    }
  }
  
}
void initSensors() {

  delay(200);
  Serial.println();
  Serial.println(F("Booting dual VL53L0X setup..."));
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);

  // wait until serial port opens for native USB devices
  //while (! Serial) { delay(1); }

  pinMode(SHT_LOX1, OUTPUT);
  pinMode(SHT_LOX2, OUTPUT);

  Serial.println(F("Shutdown pins inited..."));

  digitalWrite(SHT_LOX1, LOW);
  digitalWrite(SHT_LOX2, LOW);

  Serial.println(F("Both in reset mode...(pins are low)"));
  
  
  Serial.println(F("Starting..."));
  setID();
  if (!sensorsReady) {
    Serial.println(F("Sensor init failed. Retrying every 1 second..."));
  }
 
}

void loopSensors() {
  if (!sensorsReady) {
    setID();
    delay(1000);
    return;
  }
  read_dual_sensors();
  delay(5); // 5 ms delay between reads 
}
