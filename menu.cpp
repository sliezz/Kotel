#include "menu.h"
#include "lcd.h"
#include "log.h"

__MENU MENU;

enum menuStages {
  stage_SelectOption=0,
  stage_EditOption,
  stage_Restart
};

enum menuOptions {
  menu_DateTime=0,
  menu_DeltaTemp,
  menu_Rabota,
  menu_Tlenie,
  menu_intervalProduvka,
  menu_timeoutRastopka,
  menu_timeoutTempFix,
  menu_timeoutTempFail,
  menuOptionsCount
};

__MENU::__MENU() {
  bIsActive = false;
  stage = stage_SelectOption;
  option = menu_DateTime;
  part = 0;
  memset(buf, 0, sizeof(buf));
}

bool __MENU::isActive() {
  return bIsActive;
}

void __MENU::printOption() {
  char* str = "";
  switch (option) {
    case menu_DateTime: str = "Время"; break;
    case menu_DeltaTemp: str = "Допуск t°"; break;
    case menu_Rabota: str = "День"; break;
    case menu_Tlenie: str = "Ночь"; break;
    case menu_intervalProduvka: str = "период продувки"; break;
    case menu_timeoutRastopka: str = "timeout растопки"; break;
    case menu_timeoutTempFix: str = "timeout сбр.золы"; break;
    case menu_timeoutTempFail: str = "timeout стоп"; break;
  }
  if (stage == stage_SelectOption) {
    LCD.printTitle(" НАСТРОЙКИ");
    LCD.println(str);
  }
  if (stage == stage_EditOption) {
    LCD.printTitle(str);
  }
}

void __MENU::Open() {
  bIsActive = true;

  #ifdef USE_SERIAL_LOG
    LOG._println(log_menuOpened, sizeof(log_menuOpened));
  #endif

  stage = stage_SelectOption;
  option = 0;
  printOption();
}

void __MENU::Close() {
  bIsActive = false;

  #ifdef USE_SERIAL_LOG
    LOG._println(log_menuClosed, sizeof(log_menuClosed));
  #endif

  LCD.printTitle("");
  LCD.println(LCD.lastStatus);
}

bool __MENU::SelectOption() {
  if (KeyPressed == 'S' || KeyPressed == 'L') {
    Close();
    return false;
  }

  if (KeyPressed == 'U') {
    if (option==0) option=menuOptionsCount;
    option--;
    printOption();
  }
  else if (KeyPressed == 'D') {
    option++;
    if (option==menuOptionsCount) option=0;
    printOption();
  }
  else if (KeyPressed == 'O') {
    stage = stage_EditOption;
    part = 0;
    printOption();

    #ifdef USE_SERIAL_LOG
      LOG.println2("Открыт пункт меню", option);
    #endif

    switch (option) {
      case menu_DateTime:
        load_DateTime();
        break;
      case menu_DeltaTemp:
        load_DeltaTemp();
        break;
      case menu_Rabota:
        load_Rabota();
        break;
      case menu_Tlenie:
        load_Tlenie();
        break;
      case menu_intervalProduvka:
        load_intervalProduvka();
        break;
      case menu_timeoutRastopka:
        load_timeoutRastopka();
        break;
      case menu_timeoutTempFix:
        load_timeoutTempFix();
        break;
      case menu_timeoutTempFail:
        load_timeoutTempFail();
        break;
    }
  }
}

bool __MENU::EditOption() {
  switch (option) {
    case menu_DateTime:
      return edit_DateTime();
    case menu_DeltaTemp:
      return edit_DeltaTemp();
    case menu_Rabota:
      return edit_Rabota();
    case menu_Tlenie:
      return edit_Tlenie();
    case menu_intervalProduvka:
      return edit_intervalProduvka();
    case menu_timeoutRastopka:
      return edit_timeoutRastopka();
    case menu_timeoutTempFix:
      return edit_timeoutTempFix();
    case menu_timeoutTempFail:
      return edit_timeoutTempFail();
  }
  return false;
}

/*
bool __MENU::edit_Number02(char* suffix) {
  bool edited = false;
  if (KeyPressed == 'U') {
    data[part]++;
    if (data[part] >= max[part]) {
      data[part] = min[part];
    }
    edited=true;
  }
  if (KeyPressed == 'D') {
    data[part]--;
    if (data[part] < min[part]) {
      data[part] = max[part];
    }
    edited=true;
  }
  if (edited) {
    if (part!=0) {
      data[0] = data[1] + 10*data[2];
      edited = false;
    }
    if (data[0] < min[0]) {
      data[0] = min[0];
      edited = true;
    }
    if (data[0] > max[0]) {
      data[0] = max[0];
      edited = true;
    }
    if (edited) {
      data[1] = data[0] % 10;
      data[2] = (data[0] - data[1]) / 10;
    }
  }
  if (KeyPressed == 'R') {
    part--;
    if (part<0) part=2;
  }
  if (KeyPressed == 'O') {
    part++;
    if (part>2) part=0;
  }
  sprintf(buf, "%02i%s", data[0], suffix);
  if (LCD.blink) {
    if (part==0)
      buf[0]=buf[1]=' ';
    else
      buf[2-part]=' ';
  }
  LCD.println(buf);

  return true;
}
*/
bool __MENU::edit_Number02(char* suffix) {
  data[3]=0;min[3]=0;max[3]=0;
  return edit_Number03(suffix);
}

bool __MENU::edit_Number03(char* suffix) {
  bool edited = false;
  if (KeyPressed == 'U') {
    data[part]++;
    if (data[part] >= max[part]) {
      data[part] = min[part];
    }
    edited=true;
  }
  if (KeyPressed == 'D') {
    data[part]--;
    if (data[part] < min[part]) {
      data[part] = max[part];
    }
    edited=true;
  }
  if (edited) {
    if (part!=0) {
      data[0] = data[1] + 10*data[2] + 100*data[3];
      edited = false;
    }
    if (data[0] < min[0]) {
      data[0] = min[0];
      edited = true;
    }
    if (data[0] > max[0]) {
      data[0] = max[0];
      edited = true;
    }
    if (edited) {
      data[1] = data[0] % 10;
      data[2] = ((data[0] - data[1]) / 10) % 10;
      data[3] = ((data[0] - data[1]) / 10 - data[2]) / 10;
    }
  }
  if (KeyPressed == 'R') {
    part--;
    if (part<0) part=3;
  }
  if (KeyPressed == 'O') {
    part++;
    if (part>3) part=0;
  }
  sprintf(buf, "%03i%s", data[0], suffix);
  if (LCD.blink) {
    if (part==0)
      buf[0]=buf[1]=buf[2]=' ';
    else
      buf[3-part]=' ';
  }
  LCD.println(buf);

  return true;
}

bool __MENU::edit_Mode() {
  bool edited = false;
  if (KeyPressed == 'U') {
    data[part]++;
    if (data[part] >= max[part]) {
      data[part] = min[part];
    }
    edited=true;
  }
  if (KeyPressed == 'D') {
    data[part]--;
    if (data[part] < min[part]) {
      data[part] = max[part];
    }
    edited=true;
  }
  if (edited) {
    if (part == 4 || part == 5) {
      edited=false;
      data[3] = data[5] + 10*data[4];
    }
    if (data[3] < min[3]) {
      data[3] = min[3];
      edited=true;
    }
    if (data[3] > max[3]) {
      data[3] = max[3];
      edited=true;
    }
    if (edited) {
      data[5] = data[3] % 10;
      data[4] = (data[3] - data[5]) / 10;
    }
  }
  if (KeyPressed == 'O') {
    part--;
    if (part<0) part=5;
  }
  if (KeyPressed == 'R') {
    part++;
    if (part>5) part=0;
  }
  const char* status = (part==0 && LCD.blink ? "    " : (data[0]?"вкл ":"выкл"));
  uint8_t offset = strlen(status);
  sprintf(buf, "%s  %02i:%02i  %02i°", status, data[1], data[2], data[3]);
  if (LCD.blink) {
    if (part == 1) {
      buf[offset+2]=' ';
      buf[offset+3]=' ';
    }
    else if (part == 2) {
      buf[offset+5]=' ';
      buf[offset+6]=' ';
    }
    else if (part == 3) {
      buf[offset+9]=' ';
      buf[offset+10]=' ';
    }
    else if (part == 4) {
      buf[offset+9]=' ';
    }
    else if (part == 5) {
      buf[offset+10]=' ';
    }
  }
  LCD.println(buf);
  return true;
}


void __MENU::load_DateTime() {
  part = 0;
  data[0]=now.day();
  data[1]=now.month();
  data[2]=now.year()-2000;
  data[3]=now.hour();
  data[4]=now.minute();

  min[0]=1;
  min[1]=1;
  min[2]=24;
  min[3]=0;
  min[4]=0;

  max[0]=31;
  max[1]=12;
  max[2]=99;
  max[3]=23;
  max[4]=59;
}

bool __MENU::save_DateTime() {
  DateTime dt(2000+data[2], data[1], data[0], data[3], data[4], 0);
  if (!dt.isValid()) return false;
  rtc.adjust(dt);
  return true;
}

bool __MENU::edit_DateTime() {
  bool edited = false;
  if (KeyPressed == 'U') {
    data[part]++;
    if (data[part] >= max[part]) {
      data[part] = min[part];
    }
    edited=true;
  }
  if (KeyPressed == 'D') {
    data[part]--;
    if (data[part] < min[part]) {
      data[part] = max[part];
    }
    edited=true;
  }
  if (edited && part==1) {
    if (data[1] == 4 || data[1] == 6 || data[1] == 9 || data[1] == 11) {
      max[2] = 30;
    } else if (data[1] == 2) {
      if ((data[2] % 4) != 0 || (data[2] % 100) == 0) {
        max[2] = 29;
      } else {
        max[2] = 28;
      }
    } else {
      max[2] = 31;
    }
  }
  if (KeyPressed == 'O') {
    part--;
    if (part<0) part=4;
  }
  if (KeyPressed == 'R') {
    part++;
    if (part>=5) part=0;
  }
  if (KeyPressed == 'S') {
    if (save_DateTime()) {
      return false;
    }
  }
  sprintf(buf, "%02i.%02i.%02i %02i:%02i", data[0], data[1], data[2], data[3], data[4]);
  if (LCD.blink) {
    buf[part*3]=' ';
    buf[part*3+1]=' ';
  }
  LCD.println(buf);

  return true;
}


void __MENU::load_DeltaTemp() {
  part=1;
  data[0] = Kotel.getDeltaTemp() % 100;
  data[1] = data[0] % 10;
  data[2] = (data[0] - data[1]) / 10;

  min[0] = 0;
  min[1] = 0;
  min[2] = 0;
  max[0] = 15;
  max[1] = 9;
  max[2] = 9;
}

bool __MENU::save_DeltaTemp() {
  if (!Kotel.setDeltaTemp(data[0]))
    return false;
  return true;
}

bool __MENU::edit_DeltaTemp() {
  if (!edit_Number02("°"))
    return false;
  if (KeyPressed == 'S') {
    if (save_DeltaTemp()) {
      return false;
    }
  }
  return true;
}


void __MENU::load_Rabota() {
  part=0;
  data[0] = (Kotel.getRabotaTimerOn()) ? 1 : 0;
  data[1] = Kotel.getRabotaTimerHour();
  data[2] = Kotel.getRabotaTimerMinute();
  data[3] = Kotel.getRabotaTemp();
  data[5] = data[3] % 10;
  data[4] = (data[3] - data[5]) / 10;

  min[0] = 0;
  min[1] = 0;
  min[2] = 0;
  min[3] = 25;
  min[4] = 2;
  min[5] = 0;
  max[0] = 1;
  max[1] = 23;
  max[2] = 59;
  max[3] = 85;
  max[4] = 8;
  max[5] = 9;
}

bool __MENU::save_Rabota() {
  if (data[0] == 0) {
    if (!Kotel.getTlenieTimerOn())
      return false;
    if (!Kotel.setRabotaTimerOff())
      return false;
  } else {
    if (!Kotel.setRabotaTimer(data[1], data[2]))
      return false;
    if (!Kotel.setRabotaTemp(data[3]))
      return false;
  }
  return true;
}

bool __MENU::edit_Rabota() {
  if (!edit_Mode())
    return false;
  if (KeyPressed == 'S') {
    if (save_Rabota()) {
      return false;
    }
  }
  return true;
}


void __MENU::load_Tlenie() {
  part=0;
  data[0] = (Kotel.getTlenieTimerOn()) ? 1 : 0;
  data[1] = Kotel.getTlenieTimerHour();
  data[2] = Kotel.getTlenieTimerMinute();
  data[3] = Kotel.getTlenieTemp();
  data[5] = data[3] % 10;
  data[4] = (data[3] - data[5]) / 10;

  min[0] = 0;
  min[1] = 0;
  min[2] = 0;
  min[3] = 25;
  min[4] = 2;
  min[5] = 0;
  max[0] = 1;
  max[1] = 23;
  max[2] = 59;
  max[3] = 85;
  max[4] = 8;
  max[5] = 9;
}

bool __MENU::save_Tlenie() {
  if (data[0] == 0) {
    if (!Kotel.getRabotaTimerOn())
      return false;
    if (!Kotel.setTlenieTimerOff())
      return false;
  } else {
    if (!Kotel.setTlenieTimer(data[1], data[2]))
      return false;
    if (!Kotel.setTlenieTemp(data[3]))
      return false;
  }
  return true;
}

bool __MENU::edit_Tlenie() {
  if (!edit_Mode())
    return false;
  if (KeyPressed == 'S') {
    if (save_Tlenie()) {
      return false;
    }
  }
  return true;
}


void __MENU::load_intervalProduvka() {
  part=1;
  data[0] = Kotel.getIntervalProduvkaMinutes() % 100;
  data[1] = data[0] % 10;
  data[2] = (data[0] - data[1]) / 10;

  min[0] = 10;
  min[1] = 0;
  min[2] = 0;
  max[0] = 90;
  max[1] = 9;
  max[2] = 9;
}

bool __MENU::save_intervalProduvka() {
  if (!Kotel.setIntervalProduvkaMinutes(data[0]))
    return false;
  return true;
}


bool __MENU::edit_intervalProduvka() {
  if (!edit_Number02(" минут"))
    return false;
  if (KeyPressed == 'S') {
    if (save_intervalProduvka()) {
      return false;
    }
  }
  return true;
}



void __MENU::load_timeoutRastopka() {
  part=1;
  data[0] = Kotel.getTimeoutRastopkaMinutes() % 1000;
  data[1] = data[0] % 10;
  data[2] = ((data[0] - data[1]) / 10) % 10;
  data[3] = ((data[0] - data[1]) / 10 - data[2]) / 10;

  min[0] = 10;
  min[1] = 0;
  min[2] = 0;
  min[3] = 0;
  max[0] = 180;
  max[1] = 9;
  max[2] = 9;
  max[3] = 9;
}


bool __MENU::save_timeoutRastopka() {
  if (!Kotel.setTimeoutRastopkaMinutes(data[0]))
    return false;
  return true;
}


bool __MENU::edit_timeoutRastopka() {
  if (!edit_Number03(" минут"))
    return false;
  if (KeyPressed == 'S') {
    if (save_timeoutRastopka()) {
      return false;
    }
  }
  return true;
}



void __MENU::load_timeoutTempFix() {
  part=1;
  data[0] = Kotel.getTimeoutTempFixMinutes() % 1000;
  data[1] = data[0] % 10;
  data[2] = ((data[0] - data[1]) / 10) % 10;
  data[3] = ((data[0] - data[1]) / 10 - data[2]) / 10;

  min[0] = 10;
  min[1] = 0;
  min[2] = 0;
  min[3] = 0;
  max[0] = 180;
  max[1] = 9;
  max[2] = 9;
  max[3] = 9;
}


bool __MENU::save_timeoutTempFix() {
  if (!Kotel.setTimeoutTempFixMinutes(data[0]))
    return false;
  return true;
}


bool __MENU::edit_timeoutTempFix() {
  if (!edit_Number03(" минут"))
    return false;
  if (KeyPressed == 'S') {
    if (save_timeoutTempFix()) {
      return false;
    }
  }
  return true;
}



void __MENU::load_timeoutTempFail() {
  part=1;
  data[0] = Kotel.getTimeoutTempFailMinutes() % 1000;
  data[1] = data[0] % 10;
  data[2] = ((data[0] - data[1]) / 10) % 10;
  data[3] = ((data[0] - data[1]) / 10 - data[2]) / 10;

  min[0] = 10;
  min[1] = 0;
  min[2] = 0;
  min[3] = 0;
  max[0] = 180;
  max[1] = 9;
  max[2] = 9;
  max[3] = 9;

}


bool __MENU::save_timeoutTempFail() {
  if (!Kotel.setTimeoutTempFailMinutes(data[0]))
    return false;
  return true;
}


bool __MENU::edit_timeoutTempFail() {
  if (!edit_Number03(" минут"))
    return false;
  if (KeyPressed == 'S') {
    if (save_timeoutTempFail()) {
      return false;
    }
  }
  return true;
}



bool __MENU::Run() {

  if (!bIsActive) {
    if (KeyPressed == 'S') {
      Open();
    }
    return true;
  }

  if (stage == stage_Restart) {
    data[0]++;
    if (data[0] > 3) {
      stage = stage_SelectOption;
      printOption();
    }
  }

  if (stage == stage_SelectOption) {
    if (!SelectOption()) {
      return false;
    }
  }

  if (stage == stage_EditOption) {
    if (KeyPressed == 'L') {
      stage = stage_Restart;
      data[0]=99;
    }
    if (!EditOption()) {
      stage = stage_Restart;
      data[0]=0;
      LCD.println("** Сохранено ** ");
    }
  }

  return true;
}
