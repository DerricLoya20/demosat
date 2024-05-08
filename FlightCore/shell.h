#ifndef SHELL_H
#define SHELL_H

#include <SPI.h>
#include <SD.h>

#include "log.h"
#include "sensor.h"

#define SHELL_TRANSMISSION_BAUD 9600
#define SHELL_MAX_TOKEN_LEN 20
#define SHELL_STREAM_ENDER 0 // space for testing; use null byte in production

namespace shell {
  enum STATE {
    M_IDLE,
    M_TOKEN,
    M_UNTOKEN, // out of space for tokens
    M_STREAM
  };

  enum COMMANDS { // all lowercase to get 0b011BBBBB
    NOP = 0,

    FS_LS = 'l',
    FS_CAT = 'c',
    FS_RM = 'r',
    FS_WRITE = 'w',

    MODIFY_LOGGING = 'm'
  };

  enum LOG_COMM {
    LOG_START = 's',
    LOG_END = 'e',
    LOG_RESTART = 'r',

    LOG_PRECISION_0 = '0', // only group like logs when on the same tick
    LOG_PRECISION_1 = '1', // group like logs within 10ms
    LOG_PRECISION_2 = '2', // group like logs within 40ms
    LOG_PRECISION_3 = '3'  // group like logs within 100ms
  };

  enum IDENTIFIERS { // notice all uppercase
    MPU_X_ACC = 'X',
    MPU_Y_ACC = 'Y',
    MPU_Z_ACC = 'Z',
    
    BMP_ALTITUDE = 'A',

    AHT_HUMIDITY = 'H',

    ANA_EXT_TEMP = 'E',
    ANA_INT_TEMP = 'I',
    ANA_VOLTAGE = 'V',
    ANA_DETECTIONS = 'D',

    META_SD_FUNCTIONAL = 'S',
    META_SD_LOGGING = 'T',
  };

  void init();
  void tick();
  void sendVariable(unsigned char value);
  void interpretCommand(shell::COMMANDS value);

  bool run();
  void fs_ls();
  void fs_cat();
  void fs_write();
  void fs_rm();
  void modifyLogState();

  void printDirectory(File &dir, int numTabs);
  
}

#endif