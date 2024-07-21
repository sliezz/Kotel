#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif
#include <RTClib.h>
#include "Kotel.h"

// внешние переменные и функции
extern DateTime now;
extern RTC_DS1307 rtc;
extern char KeyPressed;
extern __Kotel Kotel;

class __MENU {
  bool bIsActive;
  int8_t stage;
  int8_t option;
  int8_t part;
  int data[6];
  int min[6];
  int max[6];

  char buf[32];

  void printOption();
  void Open();
  void Close();

  bool edit_Number02(char *suffix);
  bool edit_Number03(char *suffix);
  bool edit_Mode();

  void load_DateTime();
  bool save_DateTime();
  bool edit_DateTime();

  void load_DeltaTemp();
  bool save_DeltaTemp();
  bool edit_DeltaTemp();

  void load_Rabota();
  bool save_Rabota();
  bool edit_Rabota();

  void load_Tlenie();
  bool save_Tlenie();
  bool edit_Tlenie();

  void load_intervalProduvka();
  bool save_intervalProduvka();
  bool edit_intervalProduvka();

  void load_timeoutRastopka();
  bool save_timeoutRastopka();
  bool edit_timeoutRastopka();

  void load_timeoutTempFix();
  bool save_timeoutTempFix();
  bool edit_timeoutTempFix();

  void load_timeoutTempFail();
  bool save_timeoutTempFail();
  bool edit_timeoutTempFail();

  bool SelectOption();
  bool EditOption();

public:
  __MENU();
  bool isActive();
  bool Run();
};

extern __MENU MENU;