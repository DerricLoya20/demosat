#include "shell.h"

File shellFile;

shell::STATE shellState = shell::M_IDLE;
shell::COMMANDS shellCommand = shell::COMMANDS::NOP;
unsigned char shellCommandState = 0;
char shellStream = 0;
unsigned long shellTimer;

char shellToken[SHELL_MAX_TOKEN_LEN+1]; // +1 for null byte
int shellTokenI = 0;

void shell::init() {
  Serial1.begin(SHELL_TRANSMISSION_BAUD);
}

void shell::tick() {
  if (shellCommand != shell::COMMANDS::NOP) {
    if (shell::run()) return; // returning true will block
  }

  if (Serial1.available()) {
    unsigned char read = Serial1.read();
    
    switch (shellState) {
      case shell::M_IDLE:
        if (read & 0x20) { // command
          shell::interpretCommand((shell::COMMANDS) read);
        }
        else { // requesting variable
          shell::sendVariable(read);
        }
        break;
      case shell::M_TOKEN:
        if (shellTokenI == SHELL_MAX_TOKEN_LEN) {
          shellState = shell::M_UNTOKEN; // out of space for token
          shellTokenI++;
          break;
        }
        shellToken[shellTokenI] = read;
        shellTokenI++;
        // nobreak
      case shell::M_UNTOKEN:
        if (read == ' ' || read == '\n') {
          shellToken[shellTokenI-1] = 0; // fill last char with null byte
          shellState = shell::M_IDLE;
        }
        break;
      case shell::M_STREAM:
        shellStream = read;
        if (read == SHELL_STREAM_ENDER) { // end of stream
          shellState = shell::M_IDLE;
          break;
        }
        break;
    }
  }
}

void shell::sendVariable(unsigned char val) {
  float finalVal = 0;
  switch (val) {
    case shell::MPU_X_ACC:
      finalVal = sensor::mpu::getAx(true);
      break;
    case shell::MPU_Y_ACC:
      finalVal = sensor::mpu::getAy(true);
      break;
    case shell::MPU_Z_ACC:
      finalVal = sensor::mpu::getAz(true);
      break;
    case shell::BMP_ALTITUDE:
      finalVal = sensor::bmp::getAltitude();
      break;
    case shell::AHT_HUMIDITY:
      finalVal = sensor::aht::getHumidity();
      break;
    case shell::ANA_EXT_TEMP:
      finalVal = sensor::analog::getExternalTemp();
      break;
    case shell::ANA_INT_TEMP:
      finalVal = sensor::analog::getInternalTemp();
      break;
    case shell::ANA_VOLTAGE:
      finalVal = sensor::analog::getBattVoltage();
      break;
    case shell::ANA_DETECTIONS:
      finalVal = sensor::analog2::getRayDetections();
      break;
    case shell::META_SD_FUNCTIONAL:
      finalVal = logger::canLog() ? 1 : 0;
      break;
    case shell::META_SD_LOGGING:
      finalVal = logger::isLogging() ? 1 : 0;
      break;
  }
  
  // send as individual bytes
  for (int i = 0; i < 4; i++) {
    Serial1.write(reinterpret_cast<unsigned char*>(&finalVal)[i]);
  }
}

void shell::interpretCommand(shell::COMMANDS val) {
  shellTokenI = 0; // reset token
  shellCommandState = 0; // reset state
  
  switch (val) {
    case shell::FS_LS:
      shellCommand = val; // doesn't accept user input: run immediately
      break;
    case shell::MODIFY_LOGGING: // accept a stream
      shellCommand = val;
      shellState = shell::M_STREAM;
      break;

    // accept user input
    case shell::FS_WRITE:
    case shell::FS_CAT:
    case shell::FS_RM:
      shellCommand = val;
      shellState = shell::M_TOKEN;
      break;
    default:
      shellCommand = shell::NOP;
  }
}

bool shell::run() {
  if (shellState != shell::M_IDLE && shellState != shell::M_STREAM) return false; // wait until in idle state to run command

  switch (shellCommand) {
    case shell::FS_LS:
      shell::fs_ls();
      break;
    case shell::FS_CAT:
      shell::fs_cat();
      break;
    case shell::FS_WRITE: // overwrite file (remove, then write)
      shell::fs_write();
      break;
    case shell::FS_RM:
      shell::fs_rm();
      break;
    case shell::MODIFY_LOGGING:
      shell::modifyLogState();
      break;
    default:
      shellCommand = shell::NOP; // do nothing
  }
  shellCommandState++; // autoinc
  return shellCommand != shell::NOP && shellState == shell::M_IDLE;
}

inline void shell::fs_ls() {
  switch (shellCommandState) {
    case 0:
      shellFile = SD.open("/"); // opoen root
      break;
    case 1:
      shell::printDirectory(shellFile, 0);
      Serial1.write((char) 0); // file names are strings, so null byte enough to indicate end of files
      break;
    case 2:
      shellFile.close();
      // nobreak
    default:
      shellCommand = shell::NOP; // done
  }
}

inline void shell::fs_cat() {
  switch (shellCommandState) {
    case 0: // token held, load file
      if (!SD.exists(shellToken)) {
        Serial1.write('0'); // bad file name
        shellCommand = shell::NOP;
        break;
      }
      Serial1.write('G'); // good file

      shellFile = SD.open(shellToken); // open with only read permissions
      break;
    case 1:
      if (!shellFile) {
        shellFile.close(); // failed to open (maybe needs closing, maybe not?)
        shellCommand = shell::NOP; // done
        break;
      }
      // successfully opened: now read
      while (shellFile.available()) { Serial1.write(shellFile.read()); }
      for (int i = 0; i < 5; i++) { Serial1.write((char) 0); } // 5 consecutive 0 bytes indicate end of file
      break;
    case 2:
      shellFile.close();
      // nobreak
    default:
      shellCommand = shell::NOP; // done
  }
}

inline void shell::fs_write() {
  switch (shellCommandState) {
    case 0: // token held, remove file
      if (!SD.exists(shellToken)) {
        Serial1.write('0'); // bad file name
        shellCommand = shell::NOP;
        break;
      }
      SD.remove(shellToken);
      shellTimer = millis() + 100;
      break;
    case 1: // wait a bit, for removal to take effect
      if (millis() < shellTimer) --shellCommandState; // conteract autoinc
      else shellState = shell::M_STREAM; // stream in data
      break;
    case 2:
      shellFile = SD.open(shellToken, FILE_WRITE);
      ++shellCommandState;
      // nobreak;
    case 3:
      if (shellState != shell::M_STREAM) break; // no longer streaming
      --shellCommandState; // combat autoinc

      if (shellStream != 0) {
        shellFile.print(shellStream);
        shellStream = 0; // wait for next data
      }
      break;
    case 4:
      shellStream = 0; // reset for later
      shellFile.flush();
      shellFile.close();
      Serial1.write('G'); // successfully added new file
      // nobreak;
    default:
      shellCommand = shell::NOP; // done
  }
}

inline void shell::fs_rm() {
  if (!SD.exists(shellToken)) {
    Serial1.write('0'); // bad file name
    shellCommand = shell::NOP;
    return;
  }
  SD.remove(shellToken);
  Serial1.write('G'); // successfully removed

  shellCommand = shell::NOP; // done
}

inline void shell::modifyLogState() {
  switch (shellCommandState) {
    case 0:
      if (shellStream == 0) --shellCommandState;
      else shellState = shell::M_IDLE; // stop streaming in once a single character is captured
      break;
    case 1:
      switch (shellStream) {
        case shell::LOG_START:
          logger::setLoggingState(true);
          break;
        case shell::LOG_END:
          logger::setLoggingState(false);
          break;
        case shell::LOG_RESTART: // turn off, then on again
          logger::setLoggingState(false);
          logger::setLoggingState(true);
          break;
        
        case shell::LOG_PRECISION_0:
          logger::setLogSliceSize(0);
          break;
        case shell::LOG_PRECISION_1:
          logger::setLogSliceSize(10);
          break;
        case shell::LOG_PRECISION_2:
          logger::setLogSliceSize(40);
          break;
        case shell::LOG_PRECISION_3:
          logger::setLogSliceSize(100);
          break;

        default: // do nothing
          break;
      }
      shellStream = 0; // reset for later
      // nobreak;
    default:
      shellCommand = shell::NOP; // done
  }
}

void shell::printDirectory(File &dir, int numTabs) {
  while (true) {
    File entry =  dir.openNextFile();
    if (!entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial1.print(' ');
    }
    Serial1.print(entry.name());
    if (entry.isDirectory()) {
      Serial1.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial1.print(" ");
      Serial1.println(entry.size(), DEC);
    }
    entry.close();
  }
}

