#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "AudioTools.h"
#include "BluetoothA2DPSink.h"

// ========== OLED 0.96" ==========
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_ADDR 0x3C

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ========== PCM5102A / I2S ==========
#define I2S_BCK  26
#define I2S_LRCK 25
#define I2S_DOUT 27

I2SStream i2s;
BluetoothA2DPSink a2dp_sink(i2s);

// ========== Rotary Encoder ==========
#define ROT_CLK 32
#define ROT_DT  33
#define ROT_SW  18

int volumeLevel = 70;
int lastCLK;
bool muted = false;

unsigned long lastButtonTime = 0;

// ========== OLED UI ==========
void drawUI() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("XANNY AUDIO");

  display.drawLine(0, 11, 127, 11, SSD1306_WHITE);

  display.setCursor(0, 18);
  display.println("Bluetooth A2DP");

  display.setCursor(0, 30);
  display.print("Device: XANNY iPod");

  display.setCursor(0, 42);
  display.print("Volume: ");
  display.print(muted ? 0 : volumeLevel);
  display.print("%");

  // Volume bar
  display.drawRect(0, 54, 128, 9, SSD1306_WHITE);
  int barWidth = muted ? 0 : map(volumeLevel, 0, 100, 0, 124);
  display.fillRect(2, 56, barWidth, 5, SSD1306_WHITE);

  display.display();
}

// ========== Volume ==========
void applyVolume() {
  int outputVolume = muted ? 0 : volumeLevel;
  // A2DP volume range: 0-127
  a2dp_sink.set_volume(map(outputVolume, 0, 100, 0, 127));
  drawUI();
}

void setup() {
  Serial.begin(115200);
  delay(300);

  // ---------- OLED ----------
  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED tidak ditemukan!");
  } else {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(20, 18);
    display.println("XANNY");
    display.setTextSize(1);
    display.setCursor(18, 45);
    display.println("Bluetooth Audio");
    display.display();
    delay(1200);
  }

  // ---------- Rotary Encoder ----------
  pinMode(ROT_CLK, INPUT_PULLUP);
  pinMode(ROT_DT, INPUT_PULLUP);
  pinMode(ROT_SW, INPUT_PULLUP);
  lastCLK = digitalRead(ROT_CLK);

  // ---------- I2S (PCM5102A) ----------
  auto cfg = i2s.defaultConfig(TX_MODE);
  cfg.pin_bck  = I2S_BCK;
  cfg.pin_ws   = I2S_LRCK;
  cfg.pin_data = I2S_DOUT;
  cfg.sample_rate = 44100;
  cfg.bits_per_sample = 16;
  cfg.channels = 2;
  cfg.buffer_count = 8;
  cfg.buffer_size = 512;

  if (!i2s.begin(cfg)) {
    Serial.println("I2S gagal diinisialisasi!");
  }

  // ---------- Bluetooth A2DP ----------
  a2dp_sink.start("XANNY iPod");
  delay(300);
  applyVolume();

  Serial.println("XANNY iPod siap dipasangkan.");
}

void loop() {
  // Rotary encoder: ubah volume
  int currentCLK = digitalRead(ROT_CLK);

  if (currentCLK != lastCLK && currentCLK == LOW) {
    if (digitalRead(ROT_DT) != currentCLK) {
      volumeLevel += 2;
    } else {
      volumeLevel -= 2;
    }

    volumeLevel = constrain(volumeLevel, 0, 100);
    muted = false;
    applyVolume();
  }
  lastCLK = currentCLK;

  // Tekan rotary untuk mute / unmute
  if (digitalRead(ROT_SW) == LOW) {
    if (millis() - lastButtonTime > 300) {
      muted = !muted;
      applyVolume();
      lastButtonTime = millis();
    }
  }

  delay(1);
}
