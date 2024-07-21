#include <LCD_1602_RUS.h>
#include "lcd.h"
#include "log.h"
#include "Kotel.h"
#include "DS18B20.h"
#include "menu.h"

__LCD LCD;

LCD_1602_RUS lcd(0x27, 16, 2);

__LCD::__LCD() {
  blink = true;
}

void __LCD::Setup() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void __LCD::Run() {
  if ((now.second() & 1) == 0) {
    blink = true;
  } else {
    blink = false;
  }
}

void __LCD::println(String str) {
  static String str0 = "";
  if (str0 == str) return;
  lcd.setCursor(0, 1);
  lcd.print(str);
  while (lcd.getCursorCol()<16)
    lcd.print(" ");
}

void __LCD::printStatus(String status) {
  if (lastStatus == status)
    return;
  lastStatus = status;

  #ifdef USE_SERIAL_LOG
    LOG.println2("Статус: ", status);
  #endif

  if (MENU.isActive()) {
    return;
  }

  println(status);
}

void __LCD::printTitle(String str) {
  lcd.setCursor(6, 0);
  lcd.print(str);
  while (lcd.getCursorCol()<16)
    lcd.print(" ");
}

void __LCD::printTemp() {
  static char buf0[7] = {0,0,0,0,0,0,0};

  int Temp = Kotel.getTemp();
  int rabotaTemp = (Kotel.getRabotaMode() == mode_Tlenie) ? Kotel.getTlenieTemp() : Kotel.getRabotaTemp();

  char ch, buf[6], ch2, ch3;
  if (Temp > 0) ch=' '; else if (Temp < 0) ch='-'; else ch=' ';
//if (Temp == rabotaTemp) ch2=' '; else if (Temp > rabotaTemp) ch2=char(127); else ch2=char(126); // стрелки влево-вправо
  if (Temp == rabotaTemp) ch2='='; else if (Temp > rabotaTemp) ch2='>'; else ch2='<';
  if (Kotel.getZolaSbrosStatus()) ch3=char(219); else if (Kotel.getReleStatus()) ch3='*'; else ch3=' ';
  sprintf(buf, "%c%c%2i%c%c", ch3, ch, abs(Temp)%100, char(223), ch2);

  if (Temp == WRONG_TEMPERATURE) {
    buf[2]='-';
    buf[3]='-';
  }

  for (int i=0; i<sizeof(buf); i++) {
    if (buf[i] != buf0[i]) {
      buf0[i] = buf[i];
      lcd.setCursor(i, 0);
      lcd.print(buf[i]);
    }
  }
}

void __LCD::printRabotaTemp() {
  static int8_t rabotaTemp0 = 0;
  //static int8_t isTlenie0 = -1;

  if (MENU.isActive()) {
    rabotaTemp0 = 0;
    return;
  }

  int rabotaMode = Kotel.getRabotaMode();
  int8_t isTlenie = (rabotaMode == mode_Tlenie) ? 1 : 0;
  int rabotaTemp = (isTlenie) ? Kotel.getTlenieTemp() : Kotel.getRabotaTemp();

  if (rabotaTemp0 != rabotaTemp) {
    rabotaTemp0 = rabotaTemp;

    lcd.setCursor(6, 0);
    lcd.print(rabotaTemp);
    lcd.print(char(223));
  }

  /*if (isTlenie0 != isTlenie) {
    isTlenie0 = isTlenie;
    lcd.setCursor(9, 0);
    if (!isTlenie) {
      lcd.print("o");
    } else {
      lcd.print("}");
    }
  }*/
}

void __LCD::printTime() {
  static char buf0[7] = {0,0,0,0,0,0,0};

  if (MENU.isActive()) {
    memset(buf0,0,7);
    return;
  }

  int h=now.hour(), m=now.minute();

  char ch, buf[7];
  if (blink) ch=':'; else ch=' ';
  sprintf(buf, "%02i%c%02i", h, ch, m);
  for (int i=0; i<sizeof(buf); i++) {
    if (buf[i] != buf0[i]) {
      buf0[i] = buf[i];
      lcd.setCursor(11+i, 0);
      lcd.print(buf[i]);
    }
  }
}

void __LCD::printRabotaMode() {
  char* str = Kotel.getRabotaModeName();
  printStatus(str);
}
