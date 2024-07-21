#include <EEPROM.h>
#include "DS18B20.h"
#include "log.h"
#include "lcd.h"
#include "Kotel.h"
#include "menu.h"

class __Kotel Kotel;

uint32_t EEPROM_read4(int16_t addr) {
  union {
    uint32_t dword;
    uint8_t byte[4];
  } res;
  res.byte[0] = (uint8_t)EEPROM.read(addr++);
  res.byte[1] = (uint8_t)EEPROM.read(addr++);
  res.byte[2] = (uint8_t)EEPROM.read(addr++);
  res.byte[3] = (uint8_t)EEPROM.read(addr);
  return res.dword;
}

bool EEPROM_update4(int16_t addr, uint32_t newValue) {
  union {
    uint32_t dword;
    uint8_t byte[4];
  } res;
  res.dword = EEPROM_read4(addr);
  if (res.dword == newValue) return false;
  
  res.dword = newValue;
  EEPROM.update(addr++, res.byte[0]);
  EEPROM.update(addr++, res.byte[1]);
  EEPROM.update(addr++, res.byte[2]);
  EEPROM.update(addr, res.byte[3]);
  return true;
}

__Kotel::__Kotel()
{
  Temp = 0;

  deltaTemp = (int8_t)EEPROM.read(0);
  rabotaTemp = (int8_t)EEPROM.read(1);
  tlenieTemp = (int8_t)EEPROM.read(2);
  tlenieTimerHour = (int8_t)EEPROM.read(3);
  tlenieTimerMinute = (int8_t)EEPROM.read(4);

  uint32_t _rabotaTimerLast = 0;
  uint32_t _tlenieTimerLast = 0;
  //if (EEPROM.read(5) == 3) {  // eeprom ver
    _tlenieTimerLast = EEPROM_read4(8);
    timeoutTempFix = EEPROM_read4(12);
    timeoutTempFail = EEPROM_read4(16);
    intervalProduvka = EEPROM_read4(20);
    timeoutRastopka = EEPROM_read4(24);
    rabotaTimerHour = (int8_t)EEPROM.read(28);
    rabotaTimerMinute = (int8_t)EEPROM.read(29);
    _rabotaTimerLast = EEPROM_read4(30);
    rabotaTimerLast = new DateTime(_rabotaTimerLast);
    tlenieTimerLast = new DateTime(_tlenieTimerLast);

  //} else {
  //  rabotaTimerLast = new DateTime(_rabotaTimerLast);
  //  tlenieTimerLast = new DateTime(_tlenieTimerLast);
  //
  //  timeoutTempFix = MINUTE_TO_MS(20);     // Не вышли на заданный режим: пытаемся исправить сбросом золы
  //  timeoutTempFail = MINUTE_TO_MS(40);    // Не вышли на заданный режим: ничего не помогло - нет топлива
  //  timeoutRastopka = MINUTE_TO_MS(60);    // Не вышли на заданный режим при растопке
  //  intervalProduvka = MINUTE_TO_MS(15);   // Период продувки котла
  //
  //  EEPROM_update4( 8, _tlenieTimerLast);
  //  EEPROM_update4(12, timeoutTempFix);
  //  EEPROM_update4(16, timeoutTempFail);
  //  EEPROM_update4(20, intervalProduvka);
  //  EEPROM_update4(24, timeoutRastopka);
  //  EEPROM.update(28, rabotaTimerHour);
  //  EEPROM.update(29, rabotaTimerMinute);
  //  EEPROM_update4(30, _rabotaTimerLast);
  //
  //  EEPROM.update(5, 3);
  //}

  rabotaMode = EEPROM.read(7);
  if (rabotaMode != mode_Avaria && rabotaMode != mode_OstanovkaKotla) {
    rabotaMode = mode_RastopkaKotla;
  }

  lastProduvkaOn = 0;
  statusDoProduvki = "";

  lastTimeOn = 0;
  curTimeOn = 0;

  lastTimeOff = 0;
  curTimeOff = 0;

  releStatus = 0;

  zolaSbrosStatus = 0;
  lastZolaSbros = 0;
}

bool __Kotel::CheckMode() {
  DateTime rabotaTimerToday(now.year(), now.month(), now.day(), rabotaTimerHour, rabotaTimerMinute, 0);
  DateTime tlenieTimerToday(now.year(), now.month(), now.day(), tlenieTimerHour, tlenieTimerMinute, 0);

  if (rabotaTimerToday < tlenieTimerToday) {
    if (tlenieTimerToday != tlenieTimerLast && tlenieTimerToday <= now) {
      setTlenieTimerLast(tlenieTimerToday);
      return setRabotaMode(mode_Tlenie);
    }
    if (rabotaTimerLast != rabotaTimerLast && rabotaTimerToday <= now) {
      setRabotaTimerLast(rabotaTimerToday);
      return setRabotaMode(mode_PoTemperature);
    }
  }
  else if (rabotaTimerToday > tlenieTimerToday) {
    if (rabotaTimerLast != rabotaTimerLast && rabotaTimerToday <= now) {
      setRabotaTimerLast(rabotaTimerToday);
      return setRabotaMode(mode_PoTemperature);
    }
    if (tlenieTimerToday != tlenieTimerLast && tlenieTimerToday <= now) {
      setTlenieTimerLast(tlenieTimerToday);
      return setRabotaMode(mode_Tlenie);
    }
  }
}

enum rabotaModes __Kotel::getRabotaMode() {
  return rabotaMode;
}

int8_t __Kotel::getTemp(bool ReadTemp) {
  if (ReadTemp) {
    Temp = DS18B20.ReadInt();
  }
  return Temp;
}

int8_t __Kotel::getDeltaTemp() {
  return deltaTemp;
}

uint8_t __Kotel::set_int8(int8_t* pValue, int8_t newValue, int8_t minValue, int8_t maxValue, int16_t eeprom_addr) {
  if (newValue < minValue) return 0;
  if (newValue > maxValue) return 0;
  if (*pValue != newValue) {
    *pValue = newValue;
    EEPROM.update(eeprom_addr, newValue);

    #ifdef USE_SERIAL_LOG
      LOG._println(log_eeprom_set, sizeof(log_eeprom_set));
    #endif
    return 2;
  }

  return 1;
}

uint8_t __Kotel::set_double_int8(int8_t* pValue1, int8_t* pValue2, int8_t newValue1, int8_t newValue2, int8_t minValue1, int8_t minValue2, int8_t maxValue1, int8_t maxValue2, int16_t eeprom_addr1, int16_t eeprom_addr2) {
  if (newValue1 < minValue1 || newValue1 > maxValue1) return 0;
  if (newValue2 < minValue2 || newValue2 > maxValue2) return 0;

  if (*pValue1 != newValue1 || *pValue2 != newValue2) {
    *pValue1 = newValue1;
    *pValue2 = newValue2;
    EEPROM.update(eeprom_addr1, newValue1);
    EEPROM.update(eeprom_addr2, newValue2);

    #ifdef USE_SERIAL_LOG
      LOG._println(log_eeprom_set, sizeof(log_eeprom_set));
    #endif
    return 2;
  }

  return 1;
}

uint8_t __Kotel::set_uint32(uint32_t* pValue, uint32_t newValue, uint32_t minValue, uint32_t maxValue, int16_t eeprom_addr) {
  if (newValue < minValue) return 0;
  if (newValue > maxValue) return 0;
  if (*pValue != newValue) {
    *pValue = newValue;
    EEPROM_update4(eeprom_addr, newValue);

    #ifdef USE_SERIAL_LOG
      LOG._println(log_eeprom_set, sizeof(log_eeprom_set));
    #endif
    return 2;
  }

  return 1;
}

bool __Kotel::setDeltaTemp(int8_t newDeltaTemp) {
  uint8_t res = set_int8(&deltaTemp, newDeltaTemp, 0, 15, 0);
  if (res==0) {
    return false;
  }
  if (res==2) {
    #ifdef USE_SERIAL_LOG
      LOG._println2(log_DeltaTemp, sizeof(log_DeltaTemp), newDeltaTemp);
    #endif
  }
  return true;
}

int8_t __Kotel::getRabotaTemp() {
  return rabotaTemp;
}

bool __Kotel::setRabotaTemp(int8_t newRabotaTemp) {
  uint8_t res = set_int8(&rabotaTemp, newRabotaTemp, 25, 85, 1);
  if (res==0) {
    return false;
  }
  if (res==2) {
    #ifdef USE_SERIAL_LOG
      LOG._println2(log_RabotaTemp, sizeof(log_RabotaTemp), newRabotaTemp);
    #endif
  }
  return true;
}

bool __Kotel::getRabotaTimerOn() {
  if (rabotaTimerHour<0) return false;
  return true;
}

int8_t __Kotel::getRabotaTimerHour() {
  if (rabotaTimerHour<0) return 0;
  return rabotaTimerHour;
}

int8_t __Kotel::getRabotaTimerMinute() {
  if (rabotaTimerHour<0) return 0;
  return rabotaTimerMinute;
}

String __Kotel::getRabotaTimer() {
  if (rabotaTimerHour<0) return "--:--";
  String res;
  if (rabotaTimerHour<10) res = "0";
  res += rabotaTimerHour;
  res += ":";
  if (rabotaTimerMinute<10) res += "0";
  res += rabotaTimerMinute;
  return res;
}

bool __Kotel::setRabotaTimerOff() {
  uint8_t res = set_double_int8(&rabotaTimerHour, &rabotaTimerMinute, -1, 0, -1, 0, -1, 0, 28, 29);
  if (res==0) {
    return false;
  }
  if (res==2) {
    #ifdef USE_SERIAL_LOG
      LOG._println2(log_RabotaTimer, sizeof(log_RabotaTimer), getRabotaTimer());
    #endif
  }
  return true;
}

bool __Kotel::setRabotaTimer(int8_t newHour, int8_t newMinute) {
  uint8_t res = set_double_int8(&rabotaTimerHour, &rabotaTimerMinute, newHour, newMinute, 0, 0, 23, 59, 28, 29);
  if (res==0) {
    return false;
  }
  if (res==2) {
    #ifdef USE_SERIAL_LOG
      LOG._println2(log_RabotaTimer, sizeof(log_RabotaTimer), getRabotaTimer());
    #endif
  }
  return true;
}

DateTime __Kotel::getRabotaTimerLast() {
  return rabotaTimerLast;
}

bool __Kotel::setRabotaTimerLast(DateTime newRabotaTimer) {
  uint32_t _rabotaTimerLast = rabotaTimerLast.unixtime();
  uint32_t _newRabotaTimer = newRabotaTimer.unixtime();
  uint8_t res = set_uint32(&_rabotaTimerLast, _newRabotaTimer, _newRabotaTimer, _newRabotaTimer, 30);
  if (res==0) {
    return false;
  }
  if (res==2) {
    rabotaTimerLast=newRabotaTimer;
    #ifdef USE_SERIAL_LOG
      LOG._println2(log_RabotaTimerLast, sizeof(log_RabotaTimerLast), Kotel.getRabotaTimerLast());
    #endif
  }
  return true;
}


int8_t __Kotel::getTlenieTemp() {
  return tlenieTemp;
}

bool __Kotel::setTlenieTemp(int8_t newTlenieTemp) {
  if (newTlenieTemp < 25) return false;
  if (newTlenieTemp > 85) return false;

  if (tlenieTemp != newTlenieTemp) {
    tlenieTemp = newTlenieTemp;
    EEPROM.update(2, newTlenieTemp);

    #ifdef USE_SERIAL_LOG
      LOG._println(log_eeprom_set, sizeof(log_eeprom_set));
      LOG._println2(log_TlenieTemp, sizeof(log_TlenieTemp), newTlenieTemp);
    #endif
  }

  return true;
}

bool __Kotel::getTlenieTimerOn() {
  if (tlenieTimerHour<0) return false;
  return true;
}

int8_t __Kotel::getTlenieTimerHour() {
  if (tlenieTimerHour<0) return 0;
  return tlenieTimerHour;
}

int8_t __Kotel::getTlenieTimerMinute() {
  if (tlenieTimerHour<0) return 0;
  return tlenieTimerMinute;
}

String __Kotel::getTlenieTimer() {
  if (tlenieTimerHour<0) return "--:--";
  String res;
  if (tlenieTimerHour<10) res = "0";
  res += tlenieTimerHour;
  res += ":";
  if (tlenieTimerMinute<10) res += "0";
  res += tlenieTimerMinute;
  return res;
}

bool __Kotel::setTlenieTimerOff() {
  uint8_t res = set_double_int8(&tlenieTimerHour, &tlenieTimerMinute, -1, 0, -1, 0, -1, 0, 3, 4);
  if (res==0) {
    return false;
  }
  if (res==2) {
    #ifdef USE_SERIAL_LOG
      LOG._println2(log_TlenieTimer, sizeof(log_TlenieTimer), getRabotaTimer());
    #endif
  }
  return true;
}

bool __Kotel::setTlenieTimer(int8_t newHour, int8_t newMinute) {
  uint8_t res = set_double_int8(&tlenieTimerHour, &tlenieTimerMinute, newHour, newMinute, 0, 0, 23, 59, 3, 4);
  if (res==0) {
    return false;
  }
  if (res==2) {
    #ifdef USE_SERIAL_LOG
      LOG._println2(log_TlenieTimer, sizeof(log_TlenieTimer), getRabotaTimer());
    #endif
  }
  return true;
}

DateTime __Kotel::getTlenieTimerLast() {
  return tlenieTimerLast;
}

bool __Kotel::setTlenieTimerLast(DateTime newTlenieTimerLast) {
  uint32_t _tlenieTimerLast = tlenieTimerLast.unixtime();
  uint32_t _newTlenieTimerLast = newTlenieTimerLast.unixtime();
  uint8_t res = set_uint32(&_tlenieTimerLast, _newTlenieTimerLast, _newTlenieTimerLast, _newTlenieTimerLast, 8);
  if (res==0) {
    return false;
  }
  if (res==2) {
    tlenieTimerLast=newTlenieTimerLast;
    #ifdef USE_SERIAL_LOG
      LOG._println2(log_TlenieTimerLast, sizeof(log_TlenieTimerLast), Kotel.getTlenieTimerLast());
    #endif
  }
  return true;
}

uint16_t __Kotel::getTimeoutTempFixMinutes() {
  return MS_TO_MINUTE(timeoutTempFix);
}

uint16_t __Kotel::getTimeoutTempFailMinutes() {
  return MS_TO_MINUTE(timeoutTempFail);
}

uint16_t __Kotel::getTimeoutRastopkaMinutes() {
  return MS_TO_MINUTE(timeoutRastopka);
}

uint16_t __Kotel::getIntervalProduvkaMinutes() {
  return MS_TO_MINUTE(intervalProduvka);
}

bool __Kotel::setTimeoutTempFixMinutes(uint16_t newTimeoutTempFix) {
  if (newTimeoutTempFix < 10) return false;
  uint32_t _newTimeoutTempFix = MINUTE_TO_MS(newTimeoutTempFix);

  uint8_t res = set_uint32(&timeoutTempFix, _newTimeoutTempFix, _newTimeoutTempFix, timeoutTempFail, 12);
  if (res==0) {
    return false;
  }
  if (res==2) {
    #ifdef USE_SERIAL_LOG
      LOG._println2(log_timeoutTempFix, sizeof(log_timeoutTempFix), newTimeoutTempFix);
    #endif
  }
  return true;
}

bool __Kotel::setTimeoutTempFailMinutes(uint16_t newTimeoutTempFail) {
  if (newTimeoutTempFail < 15 || newTimeoutTempFail > 120) return false;
  uint32_t _newTimeoutTempFail = MINUTE_TO_MS(newTimeoutTempFail);
  uint8_t res = set_uint32(&timeoutTempFail, _newTimeoutTempFail, timeoutTempFix, _newTimeoutTempFail, 16);
  if (res==0) {
    return false;
  }
  if (res==2) {
    #ifdef USE_SERIAL_LOG
      LOG._println2(log_timeoutTempFail, sizeof(log_timeoutTempFail), newTimeoutTempFail);
    #endif
  }
  return true;
}

bool __Kotel::setTimeoutRastopkaMinutes(uint16_t newTimeoutRastopka) {
  if (newTimeoutRastopka < 10 || newTimeoutRastopka > 180) return false;
  uint32_t _newTimeoutRastopka = MINUTE_TO_MS(newTimeoutRastopka);
  uint8_t res = set_uint32(&timeoutRastopka, _newTimeoutRastopka, _newTimeoutRastopka, _newTimeoutRastopka, 24);
  if (res==0) {
    return false;
  }
  if (res==2) {
    #ifdef USE_SERIAL_LOG
      LOG._println2(log_timeoutRastopka, sizeof(log_timeoutRastopka), newTimeoutRastopka);
    #endif
  }
  return true;
}

bool __Kotel::setIntervalProduvkaMinutes(uint16_t newIntervalProduvka) {
  if (newIntervalProduvka < 10 || newIntervalProduvka > 90) return false;
  uint32_t _newIntervalProduvka = MINUTE_TO_MS(newIntervalProduvka);

  uint8_t res = set_uint32(&intervalProduvka, _newIntervalProduvka, _newIntervalProduvka, _newIntervalProduvka, 20);
  if (res==0) {
    return false;
  }
  if (res==2) {
    #ifdef USE_SERIAL_LOG
      LOG._println2(log_intervalProduvka, sizeof(log_intervalProduvka), newIntervalProduvka);
    #endif
  }
  return true;
}

char* __Kotel::getRabotaModeName(enum rabotaModes _rabotaMode) {
  if (_rabotaMode==-1) _rabotaMode=rabotaMode;
  switch(_rabotaMode)
  {
    case mode_Avaria:
      return "! А В А Р И Я !";
    case mode_OtkrytTopku:
      return "Открыть топку";
    case mode_OstanovkaKotla:
      return "Остановка котла";
    case mode_RastopkaKotla:
      return "Растопка котла";
    case mode_PoTemperature:
      return "Работа (ДЕНЬ)";
    case mode_Tlenie:
      return "Работа (НОЧЬ)";
  }
  return "???";
}

bool __Kotel::setRabotaMode(enum rabotaModes newRabotaMode) {
  if (rabotaMode == newRabotaMode)
    return false;

  if (newRabotaMode == mode_OstanovkaKotla) {
    digitalWrite(pinSignalOstanovKotla, HIGH);
  } else {
    digitalWrite(pinSignalOstanovKotla, LOW);
  }
  rabotaModeLast = rabotaMode;
  rabotaMode = newRabotaMode;
  EEPROM.update(7, rabotaMode);

  LCD.printRabotaMode();

  return true;
}

void __Kotel::ReleOff() {
  if (releStatus == 0)
    return;

  releStatus = 0;
  digitalWrite(13, LOW);

  // Запоминаем когда выключили вентилятор
  lastTimeOff = millis();
}

bool __Kotel::getReleStatus() {
  if (releStatus==0)
    return false;
  return true;
}

void __Kotel::Avaria()
{
  if (Temp < criticalTemp && Temp > 0) {
    setRabotaMode(mode_OstanovkaKotla);

    #ifdef USE_SERIAL_LOG
      LOG._println(log_avaria_done, sizeof(log_avaria_done));
    #endif
  }
}

void __Kotel::ReleOn() {
  if (releStatus == 1)
    return;

  releStatus = 1;
  digitalWrite(13, HIGH);

  // Запоминаем когда включили вентилятор
  lastTimeOn = millis();
}

void __Kotel::ProduvkaOn() {
  if (lastProduvkaOn == 0) {
    lastProduvkaOn = millis();
    statusDoProduvki = LCD.lastStatus;
    LCD.printStatus("Продувка 5 секунд");
    ReleOn();
  }
}

bool __Kotel::Produvka() {
  if (lastProduvkaOn == 0) {
    return false;
  }
  if (isTimeEllapsed(lastProduvkaOn, 5000)) {
    ReleOff();
    LCD.printStatus(statusDoProduvki);
    lastProduvkaOn = 0;
    statusDoProduvki = "";
    return false;
  }

  return true;
}

bool __Kotel::OtkrytTopku() {
  static int stage = 0;
  static unsigned long dw;

  if (stage == 0) {
    ReleOn();
    if (LCD.blink)
      LCD.println("ПАУЗА!");
    else
      LCD.println("Продув топки");

    dw = millis();
    stage = 1;
  }

  if (stage==1 && isTimeEllapsed(dw,3000)) {
    stage=2;
  }

  if (stage == 2) {
    ReleOff();

    if (LCD.blink)
      LCD.println("ПАУЗА!");
    else
      LCD.println("Откройте топку");
    if (KeyPressed != 'N') {
      stage = 0;

      setRabotaMode(rabotaModeLast);
      LCD.println(LCD.lastStatus);
      return false;
    }
  }
  return true;
}

void __Kotel::OstanovkaKotla() {
  if (releStatus == 0) {
    if (Temp > 30) {
      if (isTimeEllapsed(lastTimeOff, (intervalProduvka * 2))) {
        #ifdef USE_SERIAL_LOG
          LOG._println(log_produvka_run_30, sizeof(log_produvka_run_30));
        #endif
        ProduvkaOn();
        return;
      }
    }
    else {
    if (LCD.blink)
      LCD.println("ПАУЗА!");
    else
      LCD.println("Котел остановлен");
    }
    if (KeyPressed != 'N') {
      setRabotaMode(mode_RastopkaKotla);
    }
  } else {
    ReleOff();
  }
}

void __Kotel::ZolaSbrosReset() {
  if (lastZolaSbros != 0) {
    lastZolaSbros = 0;
  }
}

void __Kotel::ZolaSbrosOn() {
  if (lastZolaSbros == 0) {
    lastZolaSbros = millis();
    zolaSbrosStatus = 1;
    digitalWrite(pinZolaSbros, HIGH);
    #ifdef USE_SERIAL_LOG
      LOG._println(log_sbros_zoly, sizeof(log_sbros_zoly));
    #endif
  }
}

bool __Kotel::ZolaSbros() {
  if (lastZolaSbros != 0) {
    if (zolaSbrosStatus == 1) {
      if (isTimeEllapsed(lastZolaSbros, 1000)) {
        zolaSbrosStatus = 0;
        digitalWrite(pinZolaSbros, LOW);
        #ifdef USE_SERIAL_LOG
          LOG._println2(log_sbros_zoly, sizeof(log_sbros_zoly), 0);
        #endif
      }
    }
  }
  return false;
}

bool __Kotel::getZolaSbrosStatus() {
  if (!lastZolaSbros) return false;
  if (!zolaSbrosStatus) return false;
  return true;
}

void __Kotel::RastopkaKotla() {
  if (Temp >= rabotaTemp) {
    #ifdef USE_SERIAL_LOG
      LOG._println(log_rastopka_done, sizeof(log_rastopka_done));
    #endif
    setRabotaMode(mode_PoTemperature);
  }
  else {
    if (releStatus == 1) {
      if (isTimeEllapsed(lastTimeOn, timeoutTempFix)) {
        ZolaSbrosOn();
      }
      if (isTimeEllapsed(lastTimeOn, timeoutRastopka)) {
        ReleOff();
        setRabotaMode(mode_OstanovkaKotla);
      }
    } else {
      ReleOn();
      ZolaSbrosReset();
    }
  }
}

void __Kotel::PoTemperature() {
  if (Temp >= rabotaTemp) {
    ZolaSbrosReset();
    if (releStatus == 0) {
      if (isTimeEllapsed(lastTimeOff, intervalProduvka)) {
        #ifdef USE_SERIAL_LOG
          LOG._println(log_produvka_run, sizeof(log_produvka_run));
        #endif
        ProduvkaOn();
        return;
      }
    } else {
      ReleOff();
    }
  }
  else if (Temp < rabotaTemp-deltaTemp) {
    if (releStatus == 1) {
      if (isTimeEllapsed(lastTimeOn, timeoutTempFix)) {
        ZolaSbrosOn();
      }
      if (isTimeEllapsed(lastTimeOn, timeoutTempFail)) {
        ReleOff();
        setRabotaMode(mode_OstanovkaKotla);
      }
    } else {
      ReleOn();
      ZolaSbrosReset();
      LCD.printStatus("Раздув котла");
    }
  }

  if (releStatus == 0) {
    LCD.printRabotaMode();
  }
}

void __Kotel::Tlenie() {
  if (Temp >= tlenieTemp) {
    ZolaSbrosReset();
    if (releStatus == 0) {
      if (isTimeEllapsed(lastTimeOff, intervalProduvka)) {
        #ifdef USE_SERIAL_LOG
          LOG._println(log_produvka_run, sizeof(log_produvka_run));
        #endif
        ProduvkaOn();
        return;
      }
    } else {
      ReleOff();
    }
  }

  if (Temp < (tlenieTemp - deltaTemp)) {
    if (releStatus == 1) {
      if (isTimeEllapsed(lastTimeOn, timeoutTempFix)) {
        ZolaSbrosOn();
      }
      if (isTimeEllapsed(lastTimeOn, timeoutTempFail)) {
        ReleOff();
        setRabotaMode(mode_OstanovkaKotla);
      }
    } else {
      ReleOn();
      ZolaSbrosReset();
      LCD.printStatus("Раздув котла");
    }
  }

  if (releStatus == 0) {
    LCD.printRabotaMode();
  }
}

void __Kotel::Run() {
  if (ZolaSbros())
    return;     // другие режимы работы не обрабатываем

  if (Produvka())
    return;     // другие режимы работы не обрабатываем

  if (rabotaMode == mode_OtkrytTopku) {
    if (OtkrytTopku())
      return;   // временно выключаем автоматику
  }

  if (rabotaMode == mode_Avaria) {
    Avaria();
  }

  if (rabotaMode == mode_RastopkaKotla) {
    RastopkaKotla();
  }

  if (rabotaMode == mode_Tlenie || rabotaMode == mode_PoTemperature) {
    CheckMode();
  }

  if (rabotaMode == mode_Tlenie) {
    Tlenie();
  }

  if (rabotaMode == mode_PoTemperature) {
    PoTemperature();
  }

  if (rabotaMode == mode_OstanovkaKotla) {
    OstanovkaKotla();
  }

  if (KeyPressed == 'X' && !MENU.isActive()) {
    setRabotaMode(mode_OtkrytTopku);
  }

  if (KeyPressed == 'W' && !MENU.isActive()) {
    ZolaSbrosReset();
    ZolaSbrosOn();
  }
}
