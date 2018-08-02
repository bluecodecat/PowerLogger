#ifdef HAST_RTC
#include "RTCutil.h"

#ifdef __STM32F1__
  #include <RTClock.h>                    // builtin RTC
  static RTClock rtc(RTCSEL_LSE);         // initialise clock source
  static tm_t now;
  static uint32_t unixtime;
#else
  #include <RTClib.h>                     // External RTC
#if   HAST_RTC == DS1307
  static RTC_DS1307 rtc;
#elif HAST_RTC == DS3231
  static RTC_DS3231 rtc;
#elif HAST_RTC == PCF8583
  static RTC_PCF8583 rtc;
#elif HAST_RTC == PCF8563
  static RTC_PCF8563 rtc;
#endif
  static DateTime now;
  static uint32_t unixtime;
#endif

void rtc_init(Print* out){
  if(rtc_now()<BUILD_TIME){
    rtc_now(BUILD_TIME);
    out->print(F("Set RTC to built time: "));
    out->println(BUILD_TIME);
  }
  out->print(F("PowerLogger start: "));
  out->print(rtc_fmt('D'));    // long date
  out->print(F(" "));
  out->print(rtc_fmt('T'));    // long time
  out->print(F("UTC @"));
  out->println(rtc_now());
}

uint32_t rtc_now(){
#ifdef __STM32F1__
  rtc.getTime(now);
  unixtime = rtc.getTime();
#else
  now = rtc.now();
  unixtime = now.unixtime();
#endif
  return unixtime;
}

uint32_t rtc_now(uint32_t time){
#ifdef __STM32F1__
  rtc.setTime(time);
#else
  rtc.adjust(DateTime(time));
#endif
  return rtc_now(); // update now/unixtime
}

char *rtc_fmt(const char fmt){
  static char str[16];
  switch (fmt) {
#ifdef __STM32F1__
  case 'D': // long date
    sprintf(str, "%04u-%02u-%02u", 2000+now.year, now.month, now.day);
    break;
  case 'd': // short date
    sprintf(str, "%02u%02u%02u", now.year, now.month, now.day);
    break;
  case 'T': // long time
    sprintf(str, "%02u:%02u:%02u", now.hour, now.minute, now.second);
    break;
  case 't': // short time
    sprintf(str, "%02u%02u", now.hour, now.minute);
    break;
  case 'C': // file.csv
    sprintf(str, "%02u%02u%02u.csv", now.year, now.month, now.day);
    break;
#else
  case 'D': // long date
    sprintf(str, "%04u-%02u-%02u", now.year(), now.month(), now.day());
    break;
  case 'd': // short date
    sprintf(str, "%02d%02u%02u", now.year()-2000, now.month(), now.day());
    break;
  case 'T': // long time
    sprintf(str, "%02u:%02u:%02u", now.hour(), now.minute(), now.second());
    break;
  case 't': // short time
    sprintf(str, "%02u%02u", now.hour(), now.minute());
    break;
  case 'C': // YYMMDD.csv
    sprintf(str, "%02d%02u%02u.csv", now.year()-2000, now.month(), now.day());
    break;
#endif
  }
  return str;
}
#endif
