#include "sensor.h"

MPU6050 sensor_mpu;          // 0x68
Adafruit_BMP085 sensor_bmp;  // 0x77
AHT20 sensor_aht;            // 0x38

unsigned long mpuTimer,bmpTimer,ahtTimer,analogTimer,analog2Timer;
uint16_t mpuTimeout,bmpTimeout,ahtTimeout,analogTimeout,analog2Timeout;

float analogIntM,analogExtM;
float analogIntB,analogExtB;

void sensor::init() {
  Wire.begin();

  #if I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
    Fastwire::setup(400, true);
  #endif

  mpuTimer = 0;
  bmpTimer = 0;
  ahtTimer = 0;
  analogTimer = 0;
  // analog2Timer = 0;

  mpuTimeout = 500;
  bmpTimeout = 500;
  ahtTimeout = 500;
  analogTimeout = 500;
  // analog2Timeout = 0;

  sensor_mpu.initialize();
  sensor_bmp.begin();
  sensor_aht.begin();
  sensor::analog::init();

  analogReadResolution(ANALOG_READ_RESOLUTION);
  pinMode(ANALOG_INTERNAL_READ, INPUT);
  pinMode(ANALOG_EXTERNAL_READ, INPUT);
  pinMode(ANALOG_BATT_READ, INPUT);
  pinMode(ANALOG_GEIGAR_READ, INPUT_PULLDOWN);

  attachInterrupt(digitalPinToInterrupt(ANALOG_GEIGAR_READ), sensor::analog2::_update, FALLING);
}

void sensor::tick(unsigned long time) {
  if (time >= mpuTimer) {
    mpuTimer = time + mpuTimeout;
    sensor::mpu::_update();
  }
  if (time >= bmpTimer) {
    bmpTimer = time + bmpTimeout;
    sensor::bmp::_update();
  }
  if (time >= ahtTimer) {
    ahtTimer = time + ahtTimeout;
    sensor::aht::_update();
  }
  if (time >= analogTimer) {
    analogTimer = time + analogTimeout;
    sensor::analog::_update();
  }
  
  // if (time >= analog2Timer) {
  //   analogTimer = time + analogTimeout; // special: designed to run EVERY tick
  //   sensor::analog2::_update();
  // }
}

bool isNewMPUData = false;
int16_t mpuAx, mpuAy, mpuAz, mpuGx, mpuGy, mpuGz = 0;
float mpuTemp = 0;

bool sensor::mpu::isNewData() {
  if (isNewMPUData) {
    isNewMPUData = false;
    return true;
  }
  return false;
}

void sensor::mpu::_update() {
  sensor_mpu.getMotion6(&mpuAx, &mpuAy, &mpuAz, &mpuGx, &mpuGy, &mpuGz);
  mpuTemp = sensor_mpu.getTemperature() / 340.0 + 36.53;
  isNewMPUData = true;
}

void sensor::mpu::fillAcc(int16_t *ax, int16_t *ay, int16_t *az) {
 *ax = mpuAx;
 *ay = mpuAy;
 *az = mpuAz;
}

void sensor::mpu::fillGyro(int16_t *gx, int16_t *gy, int16_t *gz) {
  *gx = mpuGx;
  *gy = mpuGy;
  *gz = mpuGz;
}

int16_t sensor::mpu::getAx(bool forceNew) {
  return forceNew ? sensor_mpu.getAccelerationX() : mpuAx;
}

int16_t sensor::mpu::getAy(bool forceNew) {
  return forceNew ? sensor_mpu.getAccelerationY() : mpuAy;
}

int16_t sensor::mpu::getAz(bool forceNew) {
  return forceNew ? sensor_mpu.getAccelerationZ() : mpuAz;
}

int16_t sensor::mpu::getGx() {
  return mpuGx;
}

int16_t sensor::mpu::getGy() {
  return mpuGy;
}

int16_t sensor::mpu::getGz() {
  return mpuGz;
}


float sensor::mpu::getTemperature() {
  return mpuTemp;
}


bool isNewBMPData = false;
int32_t bmpPressure = 0;
float bmpAltitude = 0;
float bmpTemp = 0;

bool sensor::bmp::isNewData() {
  if (isNewBMPData) {
    isNewBMPData = false;
    return true;
  }
  return false;
}

void sensor::bmp::_update() {
  bmpTemp = sensor_bmp.readTemperature();
  bmpPressure = sensor_bmp.readPressure();
  bmpAltitude = sensor_bmp.readAltitude();
  isNewBMPData = true;
}
    
int32_t sensor::bmp::getPressure() {
  return bmpPressure;
}

float sensor::bmp::getAltitude() {
  return bmpAltitude;
}

float sensor::bmp::getTemperature() {
  return bmpTemp;
}


bool isNewAHTData = false;
float ahtHumidity = 0;
float ahtTemp = 0;

bool sensor::aht::isNewData() {
  if (isNewAHTData) {
    isNewAHTData = false;
    return true;
  }
  return false;
}

void sensor::aht::_update() {
  if (!sensor_aht.available()) return;
  ahtHumidity = sensor_aht.getHumidity();
  ahtTemp = sensor_aht.getTemperature();
  isNewAHTData = true;
}

float sensor::aht::getHumidity() {
  return ahtHumidity;
}

float sensor::aht::getTemperature() {
  return ahtTemp;
}

// setup stuff
void sensor::mpu::setReadTimeout(uint16_t timeout) {
  mpuTimeout = timeout;
}

void sensor::bmp::setReadTimeout(uint16_t timeout) {
  bmpTimeout = timeout;
}

void sensor::aht::setReadTimeout(uint16_t timeout) {
  ahtTimeout = timeout;
}

void sensor::analog::setReadTimeout(uint16_t timeout) {
  analogTimeout = timeout;
}

void sensor::analog2::setReadTimeout(uint16_t timeout) {
  analog2Timeout = timeout;
}


bool isNewAnalogData = false;
uint16_t analogInternalTempV;
uint16_t analogExternalTempV;
float analogBattVoltage;
bool sensor::analog::isNewData() {
  if (isNewAnalogData) {
    isNewAnalogData = false;
    return true;
  }
  return false;
}

void sensor::analog::_update() {
  analogInternalTempV = analogRead(ANALOG_INTERNAL_READ);
  analogExternalTempV = analogRead(ANALOG_EXTERNAL_READ);
  analogBattVoltage = (3.3 * analogRead(ANALOG_BATT_READ) / pow(2,ANALOG_READ_RESOLUTION)) * ANALOG_VOLTAGE_UNDIV;
  isNewAnalogData = true;
}

float sensor::analog::getInternalTemp() {
  return analogInternalTempV * analogIntM + analogIntB;
}

uint16_t sensor::analog::getInternalVoltage() {
  return analogInternalTempV;
}

float sensor::analog::getExternalTemp() {
  return analogExternalTempV * analogExtM + analogExtB;
}

uint16_t sensor::analog::getExternalVoltage() {
  return analogExternalTempV;
}

float sensor::analog::getBattVoltage() {
  return analogBattVoltage;
}

void sensor::analog::init() {
  sensor::analog::calibrateInternal(0,-50, 0.7,20);
  sensor::analog::calibrateExternal(0,-50, 0.7,20);
}

void sensor::analog::calibrateInternal(float v1, float temp1, float v2, float temp2) {
  float dt = temp2 - temp1;
  float dv = (v2 - v1) * (pow(2,ANALOG_READ_RESOLUTION) / 3.3); // convert from voltage to ADC output

  analogIntM = dt/dv;
  analogIntB = -v1 * analogIntM + temp1;
}

void sensor::analog::calibrateExternal(float v1, float temp1, float v2, float temp2) {
  float dt = temp2 - temp1;
  float dv = (v2 - v1) * (pow(2,ANALOG_READ_RESOLUTION) / 3.3); // convert from voltage to ADC output
  
  analogExtM = dt/dv;
  analogExtB = -v1 * analogExtM + temp1;
}

int analogDetections = 0;
bool isNewAnalog2Data = false;

// unsigned char analog2SubDetections = 0;
// bool analog2SubDetectionsCooldown = false;

bool sensor::analog2::isNewData() {
  if (isNewAnalog2Data) {
    isNewAnalog2Data = false;
    return true;
  }
  return false;
}

void sensor::analog2::_update() {
  analogDetections++; // rely an interrupt to not bounce
  isNewAnalog2Data = true;
  // bool pin = digitalRead(ANALOG_GEIGAR_READ);
  // if (pin) {
  //   Serial.println("detect!");
  //   if (analog2SubDetectionsCooldown) analog2SubDetections = DEBOUNCE_CYCLES; // reset count
  //   else { // ramping up
  //     analog2SubDetections++;

  //     if (analog2SubDetections == DEBOUNCE_CYCLES) {
  //       analog2SubDetectionsCooldown = true; // reverse counting direction
        
  //       // detection!
  //       analogDetections++;
  //       isNewAnalog2Data = true;
  //     }
  //   }
  // }
  // else {
  //   if (analog2SubDetectionsCooldown) { // good cooldown!
  //     analog2SubDetections--;
  //     if (analog2SubDetections == 0) {
  //       analog2SubDetectionsCooldown = false; // now able to count new detection
  //     }
  //   }
  //   else analog2SubDetections = 0; // reset ramping
  // }
}

int sensor::analog2::getRayDetections() {
  return analogDetections;
}

