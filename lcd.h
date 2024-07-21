#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif
#include <RTClib.h>

// внешние переменные и функции
extern DateTime now;

class __LCD {
public:
  String lastStatus;
  bool blink;

  __LCD();

  void Setup();

  void Run();

  void printTitle(String str);
  void println(String str);

  void printStatus(String status);
  void printTime();
  void printRabotaTemp();
  void printTemp();
  void printRabotaMode();
};

extern __LCD LCD;