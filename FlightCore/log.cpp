#include "WInterrupts.h"
#include "log.h"

File logFile;
const int logBufferSize = 100;
char* logBuffer = (char*) malloc(sizeof(char) * logBufferSize);
unsigned long lastLog = 0;
unsigned long logSliceSize;

bool logCanLog = false;
bool logDoLog = true;

bool logger::init(int sliceMS) {
  SD.begin(LOG_FILE_CS);

  return logger::openNewLogfile(sliceMS);
}

bool logger::openNewLogfile(int sliceMS) {
  // open metadata file
  logFile = SD.open(META_LOG_FILE);
  
  unsigned char metaBufferI = 0;
  int modifier = 0;
  char metaBuffer[10];
  if (logFile) {
    while (logFile.available()) {
      metaBuffer[metaBufferI] = logFile.read();
      metaBufferI++;
      if (metaBufferI == 10) { // too big: ignore the rest
        metaBufferI--;
        break;
      }
    }
    
    if (metaBufferI != 0) {
      metaBuffer[metaBufferI] = 0; // set last char as null byte
      modifier = atoi(metaBuffer) + 1;
    }

  }
  logFile.close();
    
  SD.remove(META_LOG_FILE); // clear file
  logFile = SD.open(META_LOG_FILE, FILE_WRITE);
  logFile.println(modifier); // write new modifier to file
  logFile.close();
  
  logger::setLogSliceSize(sliceMS);
  delay(500);

  char* filenameBuffer = (char*) calloc(sizeof(char), strlen(LOG_FILE_BASE) + 10); // +10 gives wiggle room to play around with name
  strcat(filenameBuffer, LOG_FILE_BASE);
  if (modifier != 0) {
    snprintf(metaBuffer, 10, "%d", modifier); // convert int to char*, reusing metaBuffer
    strcat(filenameBuffer, metaBuffer);
  }
  strcat(filenameBuffer, ".txt");
  
  SD.remove(filenameBuffer); // clean slate
  logFile = SD.open(filenameBuffer, FILE_WRITE);
  
  free(filenameBuffer);
  if (!logFile) return false;
  
  logCanLog = true; // only now is logging viable
  return true;
}

void logger::log(logger::IDENTIFIERS type, float data, unsigned long now) {
  if (!logCanLog || !logDoLog) return; // if either option set: don't log

  if (now - lastLog > logSliceSize) { // only log time if different
    lastLog = now;
    logFile.print('\n');
    logFile.print(now); // print time
    logFile.print(':'); // indicate time
  }
  
  logFile.print((char) type);
  logFile.print(data);
  logFile.print(',');
  logFile.flush();
}

bool logger::isLogging() {
  return logCanLog && logDoLog;
}

bool logger::canLog() {
  return logCanLog;
}

void logger::setLoggingState(bool doLog) {
  if (doLog == logDoLog) return; // no change
  logDoLog = doLog;
  if (logDoLog) { // start logging
    logger:openNewLogfile(logSliceSize); // start with the same stats
  }
  else { // stop logging
    logFile.close();
  }
}

void logger::setLogSliceSize(int sliceMS) {
  logSliceSize = sliceMS;
}
