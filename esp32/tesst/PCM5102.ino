#include "AudioTools.h"

#define I2S_BCK  26
#define I2S_LRCK 25
#define I2S_DOUT 27

AudioInfo info(44100, 2, 16);              // 44.1kHz, stereo, 16-bit
SineWaveGenerator<int16_t> sineWave(8000); // amplitudo 8000 (maks 32767)
GeneratedSoundStream<int16_t> sound(sineWave);
I2SStream i2s;
StreamCopy copier(i2s, sound);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Tes PCM5102: nada 440 Hz");

  auto cfg = i2s.defaultConfig(TX_MODE);
  cfg.copyFrom(info);
  cfg.pin_bck  = I2S_BCK;
  cfg.pin_ws   = I2S_LRCK;
  cfg.pin_data = I2S_DOUT;
  i2s.begin(cfg);

  sineWave.begin(info, 440);
}

void loop() {
  copier.copy();
}
