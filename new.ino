#include <Wire.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <TFT_eSPI.h>
#include "Audio.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <RTClib.h>

#define I2C_SDA         8
#define I2C_SCL         9

#define SPI_MOSI        11
#define SPI_SCK         12
#define TFT_SC          10
#define TFT_DC          15
#define TFT_RST         16
#define TFT_BL          17

#define SD_CS           14
#define SD_MISO         13

#define I2C_BCLK         4
#define I2S_LRC          5
#define I2S_DOUT         6

#define MPU_INT_PIN      7
#define SLEEP_TIMEOUT_MS  120000

TFT_eSPI tft = TFT_eSPI();
Audio audio;
Adafruit_MPU6050 mpu;
RTC_DS3231 rtc;

unsigned long lastActivityTime = 0;
bool isPlaying = false;
string currentSong = "/track1.mp3";

void setupDisplay() {
   tft.init();
   tft.setRotation(1);
   tft.fillScreen(TFT_BLACK);

   pinMode(TFT_BL, OUTPUT);
   digitalWrite(TFT_BL, HIGH);

   tft.setTextColor(TFT_WHITE, TFT_BLACK);
   tft.setTextSize(2);
   tft.setCursor(10,10);
   tft.println("Booting System...");
}

void setupAudioAndSD() {
  SPI.begin(SPI_SCK, SD_MISO, SPI_CS);
  if (!SD.begin(SD_CS))  
}
