#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 
#include <RTClib.h>

#define MINUTE_TO_MS(_MINUTE_) (_MINUTE_ * 60000ul)
#define MS_TO_MINUTE(_MS_) (_MS_ / 60000ul)
#define MS_TO_SECOND(_MS_) (_MS_ / 1000ul)

// внешние переменные и функции
extern String lastStatus;
extern char KeyPressed;
void printStatus(String status);
void printRabotaMode();
bool isTimeEllapsed(unsigned long lastTime, unsigned long testTime);
bool getKey();

enum rabotaModes {
    mode_Avaria=0,
    mode_OtkrytTopku=1,
    mode_OstanovkaKotla=2,
    mode_RastopkaKotla=3,
    mode_PoTemperature=4,
    mode_Tlenie=5
};

extern const int8_t criticalTemp;
extern const uint8_t pinZolaSbros;
extern const uint8_t pinSignalOstanovKotla;
extern const uint8_t pinRele;

class __Kotel {
private:
  int8_t Temp;
  int8_t TempLast;
  int8_t deltaTemp;

  uint32_t timeoutTempFix;
  uint32_t timeoutTempFail;
  uint32_t timeoutRastopka;
  uint32_t intervalProduvka;

  int8_t rabotaTemp;
  int8_t rabotaTimerHour;
  int8_t rabotaTimerMinute;
  DateTime rabotaTimerLast;

  int8_t tlenieTemp;
  int8_t tlenieTimerHour;
  int8_t tlenieTimerMinute;
  DateTime tlenieTimerLast;

  rabotaModes rabotaMode;
  rabotaModes rabotaModeLast;

  unsigned long lastProduvkaOn; // Время, когда началась продувка
  String statusDoProduvki;      // Статус до начала продувки (чтобы восстановить после)

  unsigned long lastTimeOn;     // Время, когда произошло включение вентилятора
  unsigned long curTimeOn;

  unsigned long lastTimeOff;    // Время, когда произошло выключение вентилятора
  unsigned long curTimeOff;

  int releStatus;               // включен ли вентилятор

  int zolaSbrosStatus;          // активен ли сброс золы
  unsigned long lastZolaSbros;

  uint8_t set_int8(int8_t* pValue, int8_t newValue, int8_t minValue, int8_t maxValue, int16_t eeprom_addr);
  uint8_t set_double_int8(int8_t* pValue1, int8_t* pValue2, int8_t newValue1, int8_t newValue2, int8_t minValue1, int8_t minValue2, int8_t maxValue1, int8_t maxValue2, int16_t eeprom_addr1, int16_t eeprom_addr2);
  uint8_t set_uint32(uint32_t* pValue, uint32_t newValue, uint32_t minValue, uint32_t maxValue, int16_t eeprom_addr);

public:
  int8_t getTemp(bool ReadTemp=false);

  int8_t getDeltaTemp();
  bool setDeltaTemp(int8_t newDeltaTemp);

  int8_t getRabotaTemp();
  bool setRabotaTemp(int8_t newRabotaTemp);
   bool getRabotaTimerOn();
  int8_t getRabotaTimerHour();
  int8_t getRabotaTimerMinute();
  String getRabotaTimer();
  bool setRabotaTimerOff();
  bool setRabotaTimer(int8_t newHour, int8_t newMinute);
  DateTime getRabotaTimerLast();
  bool setRabotaTimerLast(DateTime newRabotaTimer);

  int8_t getTlenieTemp();
  bool setTlenieTemp(int8_t newTlenieTemp);
  bool getTlenieTimerOn();
  int8_t getTlenieTimerHour();
  int8_t getTlenieTimerMinute();
  String getTlenieTimer();
  bool setTlenieTimerOff();
  bool setTlenieTimer(int8_t newHour, int8_t newMinute);
  DateTime getTlenieTimerLast();
  bool setTlenieTimerLast(DateTime dt);

  char* getRabotaModeName(enum rabotaModes _rabotaMode=-1);
  enum rabotaModes getRabotaMode();
  bool setRabotaMode(enum rabotaModes newRabotaMode);

  uint16_t getTimeoutTempFixMinutes();
  uint16_t getTimeoutTempFailMinutes();
  uint16_t getIntervalProduvkaMinutes();
  uint16_t getTimeoutRastopkaMinutes();

  bool setTimeoutTempFixMinutes(uint16_t newTimeoutTempFix);
  bool setTimeoutTempFailMinutes(uint16_t newTimeoutTempFail);
  bool setIntervalProduvkaMinutes(uint16_t newIntervalProduvka);
  bool setTimeoutRastopkaMinutes(uint16_t newTimeoutRastopka);

public:
  __Kotel();

  bool CheckMode();

  void ReleOn();
  void ReleOff();
  bool getReleStatus();

  void ProduvkaOn();
  bool Produvka();

  void ZolaSbrosReset();
  void ZolaSbrosOn();
  bool ZolaSbros();
  bool getZolaSbrosStatus();

  void Avaria();
  bool OtkrytTopku();
  void OstanovkaKotla();
  void PoTemperature();
  void Tlenie();
  void RastopkaKotla();

  void Run();
};

extern class __Kotel Kotel;