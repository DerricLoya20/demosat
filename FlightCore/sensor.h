#ifndef SENSOR_H
#define SENSOR_H

// communication includes
#include <Wire.h>
#include "I2Cdev.h"
#include "math.h"

// various sensor libraries
#include "MPU6050.h"
#include <Adafruit_BMP085.h>
#include <AHT20.h>

#define ANALOG_EXTERNAL_READ 1
#define ANALOG_INTERNAL_READ 2
#define ANALOG_BATT_READ 3
#define ANALOG_GEIGAR_READ 0

// #define ANALOG_VOLTAGE_DIV_A 4600.0
// #define ANALOG_VOLTAGE_DIV_B 10000.0

// #define ANALOG_VOLTAGE_UNDIV (ANALOG_VOLTAGE_DIV_A + ANALOG_VOLTAGE_DIV_B) / ANALOG_VOLTAGE_DIV_A
#define ANALOG_VOLTAGE_UNDIV 4.552122693931985
#define ANALOG_READ_RESOLUTION 12
#define DEBOUNCE_CYCLES 100

namespace sensor {
  void init();
  void tick(unsigned long time);
  
  namespace mpu {
    bool isNewData();
    void _update();

    void fillAcc(int16_t *ax, int16_t *ay, int16_t *az);
    void fillGyro(int16_t *gx, int16_t *gy, int16_t *gz);

    int16_t getAx(bool forceNew = false);
    int16_t getAy(bool forceNew = false);
    int16_t getAz(bool forceNew = false);
    int16_t getGx();
    int16_t getGy();
    int16_t getGz();

    float getTemperature();

    void setReadTimeout(uint16_t timeout);
  }

  namespace bmp {
    bool isNewData();
    void _update();
    
    int32_t getPressure();
    float getAltitude();
    float getTemperature();

    void setReadTimeout(uint16_t timeout);
  }

  namespace aht {
    bool isNewData();
    void _update();

    float getHumidity();
    float getTemperature();

    void setReadTimeout(uint16_t timeout);
  }

  namespace analog {
    bool isNewData();
    void _update();

    float getInternalTemp();
    float getExternalTemp();
    float getBattVoltage();

    uint16_t getInternalVoltage();
    uint16_t getExternalVoltage();

    void init();
    void calibrateInternal(float v1, float temp1, float v2, float temp2);
    void calibrateExternal(float v1, float temp1, float v2, float temp2);

    void setReadTimeout(uint16_t timeout);
  }

  namespace analog2 {
    bool isNewData();
    void _update();

    int getRayDetections();

    void setReadTimeout(uint16_t timeout);
  }
}


#endif