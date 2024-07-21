#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif
#include <RTClib.h>

#define USE_SERIAL_LOG
//#define USE_KEYS_LOG

#ifdef USE_SERIAL_LOG

const char PROGMEM log_title[] = "Kotel 1.0";
const char PROGMEM log_init_begin[] = "";
const char PROGMEM log_init_end[]   = "Инициализация завершена";
const char PROGMEM log_rtc[] = "RTC: ";
const char PROGMEM log_const_dump[] = "Параметры:";
const char PROGMEM log_criticalTemp[]  = "* Критическая t°: ";
const char PROGMEM log_pinTempSensor[]  = "* pin датчика t°: ";
const char PROGMEM log_pinRele[]  = "* pin реле вентилятора: ";
const char PROGMEM log_pinZolaSbros[]  = "* pin сброса золы: ";
const char PROGMEM log_pinSignalOstanovKotla[]  = "* pin сигнала остановки котла: ";
const char PROGMEM log_pinKeyL[]  = "* pin кнопки L (R*): ";
const char PROGMEM log_pinKeyD[]  = "* pin кнопки D: ";
const char PROGMEM log_pinKeyU[]  = "* pin кнопки U: ";
const char PROGMEM log_pinKeyO[]  = "* pin кнопки O (S*): ";
const char PROGMEM log_eeprom_dump[]  = "Загрузка настроек:";
const char PROGMEM log_eeprom_set[]  = "Сохранение настроек:";
const char PROGMEM log_DeltaTemp[]  = "* погрешность t°: ";
const char PROGMEM log_RabotaTemp[]  = "* День t°: ";
const char PROGMEM log_RabotaTimer[]  = "* День таймер: ";
const char PROGMEM log_RabotaTimerLast[]  = "* День поледний запуск: ";
const char PROGMEM log_TlenieTemp[]  = "* Ночь t°: ";
const char PROGMEM log_TlenieTimer[]  = "* Ночь таймер: ";
const char PROGMEM log_TlenieTimerLast[]  = "* Ночь поледний запуск: ";
const char PROGMEM log_timeoutTempFix[]  = "* Таймаут набора t° до сброса золы, мин: ";
const char PROGMEM log_timeoutTempFail[]  = "* Таймаут набора t° до останова, мин: ";
const char PROGMEM log_timeoutRastopka[]  = "* Таймаут набора t° при растопке, мин: ";
const char PROGMEM log_intervalProduvka[]  = "* Интервал запуска продувки, мин: ";
const char PROGMEM log_produvka_run[]  = "Запуск продувки котла";
const char PROGMEM log_sbros_zoly[]  = "Сброс золы";
const char PROGMEM log_produvka_run_30[]  = "Продувка котла пока t°>30°";
const char PROGMEM log_rastopka_done[]  = "Достигли рабочей t°";
const char PROGMEM log_avaria_run[]  = "Переход в аварию из-за критической t° (потери сигнала от датчика)!";
const char PROGMEM log_avaria_done[]  = "Выход из аварии в останов котла при нормализации t°";
const char PROGMEM log_temp_now[]  = "t° = ";
const char PROGMEM log_menuOpened[]  = "Открыто меню";
const char PROGMEM log_menuClosed[]  = "Закрыто меню";
const char PROGMEM log_DS18B20_not_found[]  = "Датчик t° не найден!";
const char PROGMEM log_DS18B20_now_found[]  = "Датчик t° снова доступен!";
const char PROGMEM log_DS18B20_too_many[]  = "Найдено более одного датчик t°!";
const char PROGMEM log_DS18B20_found[]  = "Найден датчик t°, id ";
const char PROGMEM log_DS18B20_not_supported[]  = "Датчик t° не поддерживается!";
const char PROGMEM log_DS18B20_config[]  = "Переводим датчик t° в 9-битный режим";

#ifdef USE_KEYS_LOG
const char PROGMEM log_keyPressed[]  = "Нажата кнопка ";
#endif
#endif

class __LOG {
public:
  void setup();

  void nl();

  void print(String str);
  void print(long num);
  //void print(char ch);
  void print(DateTime dt);

  void println(String str);
  void println2(String str1, String str2);
  void println2(String str1, long str2);
  //void println2(String str1, char str2);
  void println2(String str1, DateTime str2);
  void println3(String str1, String str2, String str3);
  void println3(String str1, long str2, String str3);
  void println4(String str1, String str2, String str3, String str4);
  void println5(String str1, String str2, String str3, String str4, String str5);

  void _print(const char* str, int str_size);
  void _println(const char* str, int str_size);
  void _println2(const char* str1, int str1_size, String str2);
  void _println2(const char* str1, int str1_size, long str2);
  //void _println2(const char* str1, int str1_size, char str2);
  void _println2(const char* str1, int str1_size, DateTime str2);
  void _println3(const char* str1, int str1_size, String str2, String str3);
  void _println3(const char* str1, int str1_size, long str2, String str3);
  void _println4(const char* str1, int str1_size, String str2, String str3, String str4);
  void _println5(const char* str1, int str1_size, String str2, String str3, String str4, String str5);
};

extern __LOG LOG;