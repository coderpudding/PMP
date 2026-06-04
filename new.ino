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
#define TFT_CS          10
#define TFT_DC          15
#define TFT_RST         16
#define TFT_BL          17
#define SD_CS           14
#define SD_MISO         13
#define I2S_BCLK        4
#define I2S_LRC         5
#define I2S_DOUT        6
#define MPU_INT_PIN     7
#define TOUCH_ADDR      0x15
#define MAX_TRACKS      50
#define SLEEP_TIMEOUT_MS    120000
#define SHAKE_THRESHOLD     15
#define SHAKE_DURATION      3
#define WAKE_DEBOUNCE_MS    500
#define TOUCH_DEBOUNCE_MS   300
#define UI_REFRESH_MS       500
#define CLR_BG          0x0000
#define CLR_PANEL       0x18E3
#define CLR_ACCENT      0x07FF
#define CLR_ACCENT2     0xF81F
#define CLR_WHITE       0xFFFF
#define CLR_GRAY        0x7BEF
#define CLR_DARK        0x2104
#define CLR_GREEN       0x07E0
#define CLR_RED         0xF800
#define CLR_ORANGE      0xFD20
#define CLR_PROGRESS_BG 0x3186
#define CLR_VOL_BG      0x3186
#define BTN_PREV_X      40
#define BTN_PREV_Y      170
#define BTN_PLAY_X      140
#define BTN_PLAY_Y      170
#define BTN_NEXT_X      240
#define BTN_NEXT_Y      170
#define BTN_R           28
#define VOL_X           30
#define VOL_Y           220
#define VOL_W           200
#define VOL_H           12
#define PROG_X          20
#define PROG_Y          138
#define PROG_W          280
#define PROG_H          8
TFT_eSPI tft = TFT_eSPI();
Audio audio;
Adafruit_MPU6050 mpu;
RTC_DS3231 rtc;
String trackList[MAX_TRACKS];
int trackCount = 0;
int currentTrack = 0;
bool isPlaying = false;
int volume = 15;
unsigned long lastActivityTime = 0;
unsigned long lastUIUpdate = 0;
unsigned long songStartTime = 0;
unsigned long songPausedAt = 0;
unsigned long pauseOffset = 0;
bool needsFullRedraw = true;
int lastDrawnSecond = -1;
int lastDrawnProgress = -1;
String lastDrawnTrackName = "";
bool lastDrawnPlayState = false;
int lastDrawnVolume = -1;
String lastDrawnTime = "";
bool rtcAvailable = false;
bool sdAvailable = false;
bool mpuAvailable = false;
bool readTouch(int &x, int &y) {
  Wire.beginTransmission(TOUCH_ADDR);
  Wire.write(0x02);
  Wire.endTransmission();
  Wire.requestFrom(TOUCH_ADDR, 5);
  if (Wire.available() < 5) return false;
  uint8_t numPoints = Wire.read();
  if (numPoints == 0 || numPoints > 5) return false;
  uint8_t xHigh = Wire.read();
  uint8_t xLow  = Wire.read();
  uint8_t yHigh = Wire.read();
  uint8_t yLow  = Wire.read();
  x = ((xHigh & 0x0F) << 8) | xLow;
  y = ((yHigh & 0x0F) << 8) | yLow;
  return true;
}
bool insideCircle(int tx, int ty, int cx, int cy, int r) {
  int dx = tx - cx;
  int dy = ty - cy;
  return (dx * dx + dy * dy) <= (r * r);
}
bool insideRect(int tx, int ty, int rx, int ry, int rw, int rh) {
  return tx >= rx && tx <= rx + rw && ty >= ry && ty <= ry + rh;
}
void scanTracks() {
  trackCount = 0;
  File root = SD.open("/");
  if (!root) return;
  while (true) {
    File entry = root.openNextFile();
    if (!entry) break;
    if (entry.isDirectory()) { entry.close(); continue; }
    String name = String(entry.name());
    name.toLowerCase();
    if (name.endsWith(".mp3") || name.endsWith(".wav") || name.endsWith(".flac")) {
      trackList[trackCount] = "/" + String(entry.name());
      trackCount++;
      if (trackCount >= MAX_TRACKS) break;
    }
    entry.close();
  }
  root.close();
}
String getTrackDisplayName(int index) {
  if (index < 0 || index >= trackCount) return "No Tracks";
  String name = trackList[index];
  int lastSlash = name.lastIndexOf('/');
  if (lastSlash >= 0) name = name.substring(lastSlash + 1);
  int dot = name.lastIndexOf('.');
  if (dot > 0) name = name.substring(0, dot);
  if (name.length() > 22) name = name.substring(0, 19) + "...";
  return name;
}
String getTimeString() {
  if (!rtcAvailable) return "--:--";
  DateTime now = rtc.now();
  char buf[12];
  int hour12 = now.hour() % 12;
  if (hour12 == 0) hour12 = 12;
  sprintf(buf, "%d:%02d %s", hour12, now.minute(), now.hour() < 12 ? "AM" : "PM");
  return String(buf);
}
