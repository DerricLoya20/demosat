#ifndef LOG_H
#define LOG_H

#include <cstdio>
#include <SPI.h>
#include <SD.h>

#define META_LOG_FILE "_m"
#define LOG_FILE_BASE "log"
#define LOG_FILE_CS 3

namespace logger {
  enum IDENTIFIERS {
    MPU_X_ACC = 'X',
    MPU_Y_ACC = 'Y',
    MPU_Z_ACC = 'Z',
    
    BMP_ALTITUDE = 'B',

    AHT_HUMIDITY = 'H',

    ANA_EXT_TEMP = 'E',
    ANA_INT_TEMP = 'I',
    ANA_VOLTAGE = 'V',
    ANA_DETECTIONS = 'D'
  };

  bool init(int sliceSizeMS=100);
  bool openNewLogfile(int sliceSizeMS=100);
  void log(IDENTIFIERS type, float data, unsigned long now);
  bool canLog();
  bool isLogging();
  
  void setLoggingState(bool doLog);
  void setLogSliceSize(int sliceMS);
};

#endif