#include <Wire.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <TFT_eSPI.h>
#include "Audio.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <RTClib.h>
#define I2C_SDA 8
#define I2C_SCL 9
#define SPI_MOSI 11
#define SPI_SCK 12
#define SD_CS 14
#define SD_MISO 13
#define I2S_BCLK 4
#define I2S_LRC 5
#define I2S_DOUT 6
#define MPU_INT_PIN 7
#define TOUCH_ADDR 0x15
#define MAX_TRACKS 50
#define SLEEP_TIMEOUT_MS 120000
TFT_eSPI tft = TFT_eSPI();
Audio audio;
Adafruit_MPU6050 mpu;
RTC_DS3231 rtc;
String tracks[MAX_TRACKS];
int trackCount = 0;
int cur = 0;
bool playing = false;
unsigned long lastTouch = 0;
unsigned long lastActive = 0;
bool readTouch(int &x, int &y) {
  Wire.beginTransmission(TOUCH_ADDR);
  Wire.write(0x02);
  Wire.endTransmission();
  Wire.requestFrom(TOUCH_ADDR, 5);
  if (Wire.available() < 5) return false;
  uint8_t n = Wire.read();
  if (n == 0 || n > 5) return false;
  uint8_t xH = Wire.read(), xL = Wire.read();
  uint8_t yH = Wire.read(), yL = Wire.read();
  x = ((xH & 0x0F) << 8) | xL;
  y = ((yH & 0x0F) << 8) | yL;
  return true;
}
void scanTracks() {
  trackCount = 0;
  File root = SD.open("/");
  while (File f = root.openNextFile()) {
    String name = String(f.name());
    name.toLowerCase();
    if (name.endsWith(".mp3") || name.endsWith(".wav")) {
      tracks[trackCount++] = "/" + String(f.name());
      if (trackCount >= MAX_TRACKS) break;
    }
    f.close();
  }
  root.close();
}
String displayName(int i) {
  if (i < 0 || i >= trackCount) return "No Tracks";
  String s = tracks[i];
  s = s.substring(s.lastIndexOf('/') + 1);
  s = s.substring(0, s.lastIndexOf('.'));
  if (s.length() > 20) s = s.substring(0, 17) + "...";
  return s;
}
void drawUI() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  String name = displayName(cur);
  tft.setCursor((320 - name.length() * 6) / 2, 50);
  tft.print(name);
  tft.setTextSize(1);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setCursor(130, 70);
  tft.print(String(cur + 1) + " / " + String(trackCount));
  tft.fillRoundRect(20, 110, 80, 60, 8, 0x18E3);
  tft.fillRoundRect(120, 110, 80, 60, 8, 0x07FF);
  tft.fillRoundRect(220, 110, 80, 60, 8, 0x18E3);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, 0x18E3);
  tft.setCursor(38, 130);
  tft.print("PREV");
  tft.setTextColor(TFT_BLACK, 0x07FF);
  tft.setCursor(132, 130);
  tft.print(playing ? "PAUSE" : "PLAY");
  tft.setTextColor(TFT_WHITE, 0x18E3);
  tft.setCursor(238, 130);
  tft.print("NEXT");
}
void playTrack(int i) {
  if (i < 0 || i >= trackCount) return;
  cur = i;
  audio.connecttoFS(SD, tracks[cur].c_str());
  playing = true;
  lastActive = millis();
  drawUI();
}
void goToDeepSleep() {
  digitalWrite(17, LOW);
  audio.stopSong();
  mpu.getMotionInterruptStatus();
  esp_sleep_enable_ext1_wakeup(1ULL << MPU_INT_PIN, ESP_EXT1_WAKEUP_ANY_HIGH);
  esp_deep_sleep_start();
}
void setup() {
  Serial.begin(115200);
  delay(500);
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT1) {
    delay(500);
  }
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  pinMode(17, OUTPUT);
  digitalWrite(17, HIGH);
  Wire.begin(I2C_SDA, I2C_SCL);
  rtc.begin();
  pinMode(MPU_INT_PIN, INPUT);
  if (mpu.begin()) {
    mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
    mpu.setMotionDetectionThreshold(15);
    mpu.setMotionDetectionDuration(3);
    mpu.setInterruptPinLatch(true);
    mpu.setInterruptPinPolarity(false);
    mpu.setMotionInterrupt(true);
  }
  SPI.begin(SPI_SCK, SD_MISO, SPI_MOSI, SD_CS);
  if (!SD.begin(SD_CS)) {
    tft.setTextSize(2);
    tft.setTextColor(TFT_RED);
    tft.setCursor(40, 100);
    tft.print("SD Card Error!");
    while (true) delay(1000);
  }
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(15);
  scanTracks();
  if (trackCount == 0) {
    tft.setTextSize(2);
    tft.setTextColor(TFT_RED);
    tft.setCursor(40, 100);
    tft.print("No tracks on SD!");
    while (true) delay(1000);
  }
  lastActive = millis();
  drawUI();
}
void loop() {
  audio.loop();
  int tx, ty;
  if (readTouch(tx, ty) && millis() - lastTouch > 300) {
    lastTouch = millis();
    lastActive = millis();
    if (ty >= 110 && ty <= 170) {
      if (tx >= 20 && tx <= 100) {
        audio.stopSong();
        cur = (cur - 1 + trackCount) % trackCount;
        playTrack(cur);
      }
      else if (tx >= 120 && tx <= 200) {
        if (!playing && !audio.isRunning()) {
          playTrack(cur);
        } else {
          audio.pauseResume();
          playing = !playing;
          drawUI();
        }
      }
      else if (tx >= 220 && tx <= 300) {
        audio.stopSong();
        cur = (cur + 1) % trackCount;
        playTrack(cur);
      }
    }
  }
  if (!audio.isRunning() && playing) {
    cur = (cur + 1) % trackCount;
    playTrack(cur);
  }
  if (!audio.isRunning() && !playing) {
    if (millis() - lastActive > SLEEP_TIMEOUT_MS) goToDeepSleep();
  }
}
void audio_info(const char *info) { Serial.println(info); }
void audio_id3data(const char *info) { Serial.println(info); }
void audio_eof_mp3(const char *info) { Serial.println(info); }
