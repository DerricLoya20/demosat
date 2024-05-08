#include "sensor.h"
#include "log.h"
#include "shell.h"

#define DEBUG false

void setup() {
  Serial.begin(115200);
  shell::init();
  if (DEBUG) while (!Serial) {}
  logger::init(); // log errors handled in log.cpp now 

  sensor::init();
  sensor::aht::setReadTimeout(2000);
}

void loop() {
  shell::tick();
  sensor::tick(millis());
  
  if (sensor::mpu::isNewData()) {
    int16_t x,y,z;
    sensor::mpu::fillAcc(&x,&y,&z);
    
    logger::log(logger::MPU_X_ACC, (float) x, millis());
    logger::log(logger::MPU_Y_ACC, (float) y, millis());
    logger::log(logger::MPU_Z_ACC, (float) z, millis());
  }

  if (sensor::bmp::isNewData()) {
    float altitude = sensor::bmp::getAltitude();

    logger::log(logger::BMP_ALTITUDE, altitude, millis());
  }

  if (sensor::aht::isNewData()) {
    float humidity = sensor::aht::getHumidity();

    logger::log(logger::AHT_HUMIDITY, humidity, millis());
  }

  if (sensor::analog::isNewData()) {
    float internal = sensor::analog::getInternalTemp();\
    float external = sensor::analog::getExternalTemp();
    float voltage = sensor::analog::getBattVoltage();

    logger::log(logger::ANA_INT_TEMP, internal, millis());
    logger::log(logger::ANA_EXT_TEMP, external, millis());
    logger::log(logger::ANA_VOLTAGE, voltage, millis());
  
    if (sensor::analog2::isNewData()) {
      int detections = sensor::analog2::getRayDetections();
      logger::log(logger::ANA_DETECTIONS, (float) detections, millis());
    }
  }
}
