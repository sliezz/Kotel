// Управление котлом версия 1.0 от 20.07.2024
// Первая переработанная версия со сбросом золы, сигналом остановки, меню настроек и русскими буквами

#include <Wire.h>
#include <RTClib.h>
#include <EEPROM.h>
#include "log.h"
#include "lcd.h"
#include "Kotel.h"
#include "menu.h"
#include "DS18B20.h"

// ГЛОБАЛЬНЫЕ НАСТРОЙКИ
const int8_t criticalTemp = 88;          // Температура аварии
const uint8_t pinZolaSbros = 9;           // Реле сброса золы
const uint8_t pinSignalOstanovKotla = 11; // Сигнал остановки котла
const uint8_t pinRele = 13;               // Реле вентилятора
const uint8_t pinTempSensor = 10;         // Датчик температуры
const uint8_t pinKeyL = 2;                // Пин клавиатуры
const uint8_t pinKeyD = 3;                // Пин клавиатуры
const uint8_t pinKeyU = 4;                // Пин клавиатуры
const uint8_t pinKeyO = 5;                // Пин клавиатуры
const uint8_t pinKeyR = 6;                // Пин клавиатуры

RTC_DS1307 rtc;  //Создаем объект rtc - ЧАСЫ
DateTime now;

// ОБРАБОТКА КЛАВИАТУРЫ
char KeyPressed = 'N';  // Текущая нажатая кнопка клавиатуры

bool stateKeyL = false;
bool stateKeyD = false;
bool stateKeyU = false;
bool stateKeyO = false;
bool stateKeyR = false;

int countKeyL = 0;
int countKeyD = 0;
int countKeyU = 0;
int countKeyO = 0;
int countKeyR = 0;
int longCountKey = 5;
int overCountKey = 50;

bool isTimeEllapsed(unsigned long lastTime, unsigned long testTime) {
  unsigned long curTime = millis();
  unsigned long diff;
  if (lastTime <= curTime)
    diff = curTime - lastTime;
  else
    diff = curTime + ((unsigned long)0xFFFFFFFF - lastTime) + 1;
  if (diff >= testTime)
    return true;
  return false;
}

void keySetup() {
  pinMode(pinKeyL, INPUT);
  pinMode(pinKeyD, INPUT);
  pinMode(pinKeyU, INPUT);
  pinMode(pinKeyO, INPUT);
  pinMode(pinKeyR, INPUT);

  // Для включения всех подтягивающих резисторов!!!
  digitalWrite(pinKeyL, HIGH);
  digitalWrite(pinKeyD, HIGH);
  digitalWrite(pinKeyU, HIGH);
  digitalWrite(pinKeyO, HIGH);
  digitalWrite(pinKeyR, HIGH);
}

void setup() {
#ifdef USE_SERIAL_LOG
  LOG.setup();
  LOG._println(log_init_begin, sizeof(log_init_begin));
#endif

  rtc.begin();
  DS18B20.Setup();
  LCD.Setup();
  keySetup();

  pinMode(pinZolaSbros, OUTPUT);
  digitalWrite(pinZolaSbros, LOW);

  pinMode(pinSignalOstanovKotla, OUTPUT);
  digitalWrite(pinSignalOstanovKotla, LOW);

  pinMode(pinRele, OUTPUT);
  digitalWrite(pinRele, LOW);

#ifdef USE_SERIAL_LOG
  now = rtc.now();
  LOG._println2(log_rtc, sizeof(log_rtc), now);

  LOG._println(log_const_dump, sizeof(log_const_dump));
  LOG._println2(log_criticalTemp, sizeof(log_criticalTemp), criticalTemp);
/*
  LOG._println2(log_pinTempSensor, sizeof(log_pinTempSensor), pinTempSensor);
  LOG._println2(log_pinRele, sizeof(log_pinRele), pinRele);
  LOG._println2(log_pinZolaSbros, sizeof(log_pinZolaSbros), pinZolaSbros);
  LOG._println2(log_pinSignalOstanovKotla, sizeof(log_pinSignalOstanovKotla), pinSignalOstanovKotla);
  LOG._println2(log_pinKeyL, sizeof(log_pinKeyL), pinKeyL);
  LOG._println2(log_pinKeyD, sizeof(log_pinKeyD), pinKeyD);
  LOG._println2(log_pinKeyU, sizeof(log_pinKeyU), pinKeyU);
  LOG._println2(log_pinKeyO, sizeof(log_pinKeyO), pinKeyO);
*/
  LOG._println(log_eeprom_dump, sizeof(log_eeprom_dump));
  LOG._println2(log_DeltaTemp, sizeof(log_DeltaTemp), Kotel.getDeltaTemp());

  LOG._println2(log_RabotaTemp, sizeof(log_RabotaTemp), Kotel.getRabotaTemp());
  LOG._println2(log_RabotaTimer, sizeof(log_RabotaTimer), Kotel.getRabotaTimer());
  LOG._println2(log_RabotaTimerLast, sizeof(log_RabotaTimerLast), Kotel.getRabotaTimerLast());

  LOG._println2(log_TlenieTemp, sizeof(log_TlenieTemp), Kotel.getTlenieTemp());
  LOG._println2(log_TlenieTimer, sizeof(log_TlenieTimer), Kotel.getTlenieTimer());
  LOG._println2(log_TlenieTimerLast, sizeof(log_TlenieTimerLast), Kotel.getTlenieTimerLast());

  LOG._println2(log_timeoutTempFix, sizeof(log_timeoutTempFix), Kotel.getTimeoutTempFixMinutes());
  LOG._println2(log_timeoutTempFail, sizeof(log_timeoutTempFail), Kotel.getTimeoutTempFailMinutes());
  LOG._println2(log_timeoutRastopka, sizeof(log_timeoutRastopka), Kotel.getTimeoutRastopkaMinutes());
  LOG._println2(log_intervalProduvka, sizeof(log_intervalProduvka), Kotel.getIntervalProduvkaMinutes());

  LOG._println(log_init_end, sizeof(log_init_end));
#endif

  LCD.printRabotaMode();
}

int checkKey(const int pin, int* pCount, bool* pState, const char shortPress, const char longPress) {
  if (digitalRead(pin) == LOW) {
    *pState = true;
    if (*pCount < overCountKey) (*pCount)++;
  } else {
    *pState = false;
    if (*pCount > overCountKey) {
      // залипло
    } else if (*pCount > longCountKey) {
      *pCount = 0;
      KeyPressed = longPress;
      #ifdef USE_SERIAL_LOG
        #ifdef USE_KEYS_LOG
          char buf[2]={KeyPressed,0};
          String str=buf;
          LOG._println2(log_keyPressed, sizeof(log_keyPressed), str);
        #endif
      #endif
      return 1;
    } else if (*pCount > 0) {
      *pCount = 0;
      KeyPressed = shortPress;
      #ifdef USE_SERIAL_LOG
        #ifdef USE_KEYS_LOG
          char buf[2]={KeyPressed,0};
          String str=buf;
          LOG._println2(log_keyPressed, sizeof(log_keyPressed), str);
        #endif
      #endif
      return 1;
    }
  }
  return 0;
}

bool getKey() {
  int resKeyL = checkKey(pinKeyL, &countKeyL, &stateKeyL, 'L', 'X');
  int resKeyD = checkKey(pinKeyD, &countKeyD, &stateKeyD, 'D', 'Y');
  int resKeyU = checkKey(pinKeyU, &countKeyU, &stateKeyU, 'U', 'Z');
  int resKeyO = checkKey(pinKeyO, &countKeyO, &stateKeyO, 'O', 'S');
  int resKeyR = checkKey(pinKeyR, &countKeyR, &stateKeyR, 'R', 'W');

  // в одно время может быть нажата только одна кнопка
  if ((resKeyL + resKeyD + resKeyU + resKeyO + resKeyR) != 1) {
    KeyPressed = 'N';
    return false;
  }
  return true;
}

void loop() {
  now = rtc.now();
  static int8_t TempLast = WRONG_TEMPERATURE;
  static unsigned long whenTempLast = 0;
  int8_t Temp = Kotel.getTemp(true);

  #ifdef USE_SERIAL_LOG
    if (TempLast==WRONG_TEMPERATURE || abs(TempLast-Temp)>=3 || (abs(TempLast-Temp)>=2 && isTimeEllapsed(whenTempLast, 10000))) {
      TempLast = Temp;
      whenTempLast = millis();
      LOG._println3(log_temp_now, sizeof(log_temp_now), Temp, "°");
    }
  #endif

  // Превышение критической температуры?
  if (Temp >= criticalTemp || Temp == WRONG_TEMPERATURE) {
    Kotel.ReleOff();
    if (Kotel.setRabotaMode(mode_Avaria)) {
      #ifdef USE_SERIAL_LOG
        LOG._println(log_avaria_run, sizeof(log_avaria_run));
      #endif
    }
  }

  getKey();

  LCD.Run();

  MENU.Run();

  LCD.printTemp();
  LCD.printTime();
  LCD.printRabotaTemp();

  Kotel.Run();
}
