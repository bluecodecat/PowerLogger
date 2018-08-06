/*
Based on

- DisplayReadings example fom INA library
  https://github.com/SV-Zanshin/INA

- QuickStart example form the SdFat library
  https://github.com/greiman/SdFat

- Object example on the CircularBuffer library
  https://github.com/rlogiacco/CircularBuffer

- Terminal example from the U8g2 library
  https://github.com/olikraus/u8g2/
  
- BlinkMachine esmaple from the OneButton library
  https://github.com/mathertel/OneButton
*/

#include <Arduino.h>
#include "config.h"                       // project configuration
#include "Record.h"                       // secret sauce ;-)

#include <SdFat.h>
SdFat SD;                                 // File system object.
File CSV;                                 // File object for filename

#include "RTCutil.h"
#ifdef HAST_RTC
  #define FILENAME rtc_fmt('C')           // 'YYMMDD.csv'
#else
  #define FILENAME "INA.csv"
#endif

#include "U8X8util.h"
#ifdef HAST_U8X8
  #define TERNIMAL u8x8log
#else
  #define TERNIMAL Serial
#endif

void sd_dump(){
  if (buffer.isEmpty()) { return; }
  
  rtc_now();    // new date for filename
  bool newfile = !SD.exists(FILENAME);
  CSV = SD.open(FILENAME, FILE_WRITE);
  if (!CSV) { SD.initErrorHalt(&TERNIMAL); } // errorcode/message to TERNIMAL
  if (newfile){
    buffer.first()->header(&CSV);
  }
  while (!buffer.isEmpty()) {
    buffer.shift()->print(&CSV);
  }
  CSV.close();  
}

#include <OneButton.h>
#ifndef TACT_BUTTON
  #error "Undefined TACT_BUTTON flag. Try with -DTACT_BUTTON=3"
#endif
OneButton button(TACT_BUTTON, true);      // INPUT_PULLUP

bool recording = false;                   // off when starting
void recording_toggle(){
  if (recording) { sd_dump(); }           // dump buffer to SD card
  recording = not recording;              // pause/resume buffering
}

void backlight_toggle(){
#ifdef HAS_BACKLIGHT
  static bool backlight = false;          // true when starting  
  backlight = not backlight;
  if (backlight) {
    // something
  } else {
    // something    
  }
#endif
}

void safe_shutdown(){
  TERNIMAL.println(F("Safe shutdown started"));
  sd_dump();                              // dump buffer to SD card
  recording = false;                      // pause buffering
#ifdef HAS_SOFTPOWER
  TERNIMAL.println(F("Powering down"));
  pinMode(TACT_BUTTON, OUTPUT);
  digitalWrite(TACT_BUTTON, LOW);
#else
  TERNIMAL.println(F("You can now safely remove power"));
#endif
}

void setup() {
  Serial.begin(57600);                    // for ATmega328p 3.3V 8Mhz
  while(!Serial){ SysCall::yield(); }     // Wait for USB Serial

  rtc_init(&TERNIMAL);                    // update RTC if needed

  if (!SD.begin(chipSelect, SPI_SPEED)) {
    SD.initErrorHalt(&TERNIMAL);          // errorcode/message to TERNIMAL
  }

  Record::init(&TERNIMAL, FILENAME);      // init/config INA devices
  
  button.attachClick(recording_toggle);        // pause/resume buffering
  button.attachDoubleClick(backlight_toggle);  // switch backlight on/off
  button.attachLongPressStart(safe_shutdown);  // dump buffen and power down
}

void loop() {
  static uint32_t last = 0;
  if (millis()-last<FREQUENCY) {
    // check for button presses untill is time for a new Record
    button.tick();
    delay(10);
    return;
  }
  last = millis();

  // measurements from all INA devices
  Record* record = new Record(last);
  record->splash(&TERNIMAL);

  // buffer new record
  if (recording) { buffer.unshift(record); }

  // dump buffer to CSV file
  if (buffer.isFull()) { sd_dump(); }
  
  // press while reading/writing do not count (????)
  button.reset();
}
