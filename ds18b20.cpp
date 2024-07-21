#include <OneWire.h>
#include "DS18B20.h"
#include "log.h"

__DS18B20 DS18B20;

OneWire ds(pinTempSensor);  

__DS18B20::__DS18B20() {
  found=false;
}

bool __DS18B20::Setup() {
  byte data[9];

  count=0;
  ds.reset_search();
  while (ds.search(data)) {
    if (OneWire::crc8(data,7) == data[7]) {
      count++;
    }
  }
  if (count == 0) {
    #ifdef USE_SERIAL_LOG
      LOG._println(log_DS18B20_not_found, sizeof(log_DS18B20_not_found));
    #endif
    return false;
  }
  else if (count == 1) {
    found = true;
  }
  else {
    #ifdef USE_SERIAL_LOG
      LOG._println(log_DS18B20_too_many, sizeof(log_DS18B20_too_many));
    #endif
    return false;
  }

  char buf[30]={0};
  for (int i=0; i<8; i++) {
    sprintf(buf+2*i, "%02x", data[i]);
  }
  String str=buf;
  #ifdef USE_SERIAL_LOG
    LOG._println2(log_DS18B20_found, sizeof(log_DS18B20_found), str);
  #endif

  if (data[0] != 0x28) {
    #ifdef USE_SERIAL_LOG
      LOG._println(log_DS18B20_not_supported, sizeof(log_DS18B20_not_supported));
    #endif
    return false;
  }

  memcpy(addr, data, 8);

  ds.reset();
  ds.select(addr);
  ds.write(0xBE);
  for (int i=0; i<9; i++) {
    data[i] = ds.read();
  }

  for (int try_no=0; try_no<5; try_no++) {
    if (data[8] == OneWire::crc8(data,8)) {
      if ((data[4] & 0x60) != 0) {
        ds.reset();
        ds.select(addr);
        ds.write(0x4E);
        ds.write(data[2]);
        ds.write(data[3]);
        ds.write(data[4] & ~0x60);
        #ifdef USE_SERIAL_LOG
          LOG._println(log_DS18B20_config, sizeof(log_DS18B20_config));
        #endif
      }
      break;
    }
  }
  return true;
}


int8_t __DS18B20::ReadInt() {
  union {
    byte bin[9];
    struct {
      int16_t temp16;
      byte alertHigh;
      byte alertLow;
      byte config;
      byte reserved1;
      byte reserved2;
      byte reserved3;
      byte crc;
    } ds;
  } packet;

  static bool foundNow = found;

  if (!found) {
    Setup();
    foundNow = found;
  }
  if (!found) {
    return WRONG_TEMPERATURE;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1);
  delay(94);  // 93.75ms в режиме 9-битной точности DS18B20
  ds.reset();
  ds.select(addr);
  ds.write(0xBE);

  memset(packet.bin, 0, 9);
  for (int i=0; i<9; i++) {
    packet.bin[i] = ds.read();
  }
  if (packet.ds.crc == OneWire::crc8(packet.bin,8)) {
    if (!foundNow) {
      foundNow = true;
      #ifdef USE_SERIAL_LOG
        LOG._println(log_DS18B20_now_found, sizeof(log_DS18B20_now_found));
      #endif
    }
    return (int8_t)(packet.ds.temp16>>4);
  }
  if (foundNow) {
    foundNow=false;
    #ifdef USE_SERIAL_LOG
      LOG._println(log_DS18B20_not_found, sizeof(log_DS18B20_not_found));
    #endif
  }

  return WRONG_TEMPERATURE;
}
