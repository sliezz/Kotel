#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

// внешние переменные и функции
extern const uint8_t pinTempSensor;

#define WRONG_TEMPERATURE (-128)

class __DS18B20 {
  bool found;
  uint8_t count;
  byte addr[8];
public:
  __DS18B20();
  bool Setup();
  int8_t ReadInt();
};

extern __DS18B20 DS18B20;