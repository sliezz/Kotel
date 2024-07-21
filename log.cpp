#include "log.h"

__LOG LOG;

#ifdef USE_SERIAL_LOG

extern HardwareSerial Serial;

void __LOG::setup() {
  Serial.begin(9600);
  _println(log_title, sizeof(log_title));
  Serial.println();
}

void __LOG::nl() {
  Serial.println("");
}
void __LOG::print(String str) {
  Serial.print(str);
}

void __LOG::print(long num) {
  Serial.print(num);
}

//void __LOG::print(char ch) {
//  Serial.print(ch);
//}

void __LOG::print(DateTime dt) {
  char buf[24];
  strcpy(buf, "DD.MM.YYYY hh:mm:ss");
  Serial.print(dt.toString(buf));
}

void __LOG::println(String str) {
  Serial.println(str);
}

void __LOG::println2(String str1, String str2) {
  print(str1);
  print(str2);
  nl();
}

void __LOG::println2(String str1, long str2) {
  print(str1);
  print(str2);
  nl();
}

//void __LOG::println2(String str1, char str2) {
//  print(str1);
//  print(str2);
//  nl();
//}

void __LOG::println2(String str1, DateTime str2) {
  print(str1);
  print(str2);
  nl();
}

void __LOG::println3(String str1, String str2, String str3) {
  print(str1);
  print(str2);
  print(str3);
  nl();
}

void __LOG::println3(String str1, long str2, String str3) {
  print(str1);
  print(str2);
  print(str3);
  nl();
}

void __LOG::println4(String str1, String str2, String str3, String str4) {
  print(str1);
  print(str2);
  print(str3);
  print(str4);
  nl();
}

void __LOG::println5(String str1, String str2, String str3, String str4, String str5) {
  print(str1);
  print(str2);
  print(str3);
  print(str4);
  print(str5);
  nl();
}


void __LOG::_print(const char* str, int str_size) {
  for (int i=0; i<str_size-1; i++)   {
    uint8_t c = pgm_read_byte_near(str+i);
    //if (c==0) return;
    Serial.write(c);
  }
}

void __LOG::_println(const char* str, int str_size) {
  _print(str, str_size);
  nl();
}

void __LOG::_println2(const char* str1, int str1_size, String str2) {
  _print(str1, str1_size);
  print(str2);
  nl();
}

void __LOG::_println2(const char* str1, int str1_size, long str2) {
   _print(str1, str1_size);
  print(str2);
  nl();
}

//void __LOG::_println2(const char* str1, int str1_size, char str2) {
//   _print(str1, str1_size);
//  print(str2);
//  nl();
//}

void __LOG::_println2(const char* str1, int str1_size, DateTime str2) {
   _print(str1, str1_size);
  print(str2);
  nl();
}

void __LOG::_println3(const char* str1, int str1_size, String str2, String str3) {
   _print(str1, str1_size);
  print(str2);
  print(str3);
  nl();
}

void __LOG::_println3(const char* str1, int str1_size, long str2, String str3) {
   _print(str1, str1_size);
  print(str2);
  print(str3);
  nl();
}

void __LOG::_println4(const char* str1, int str1_size, String str2, String str3, String str4) {
   _print(str1, str1_size);
  print(str2);
  print(str3);
  print(str4);
  nl();
}

void __LOG::_println5(const char* str1, int str1_size, String str2, String str3, String str4, String str5) {
   _print(str1, str1_size);
  print(str2);
  print(str3);
  print(str4);
  print(str5);
  nl();
}

#else

void __LOG::setup() {}

void __LOG::print(String str) {}
void __LOG::println(String str) {}
void __LOG::println2(String str1, String str2) {}
void __LOG::println2(String str1, long str2) {}
void __LOG::println2(String str1, DateTime str2) {}
void __LOG::println3(String str1, String str2, String str3) {}
void __LOG::println3(String str1, long str2, String str3) {}
void __LOG::println4(String str1, String str2, String str3, String str4) {}
void __LOG::println5(String str1, String str2, String str3, String str4, String str5) {}

void __LOG::_print(const char* str, int str_size) {}
void __LOG::_println(const char* str, int str_size) {}
void __LOG::_println2(const char* str1, int str1_size, String str2) {}
void __LOG::_println2(const char* str1, int str1_size, long str2) {}
void __LOG::_println2(const char* str1, int str1_size, DateTime str2) {}
void __LOG::_println3(const char* str1, int str1_size, String str2, String str3) {}
void __LOG::_println3(const char* str1, int str1_size, long str2, String str3) {}
void __LOG::_println4(const char* str1, int str1_size, String str2, String str3, String str4) {}
void __LOG::_println5(const char* str1, int str1_size, String str2, String str3, String str4, String str5) {}

#endif