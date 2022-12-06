#include <M5StickC.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "AudioTools.h"


I2SStream i2sStream; // Access I2S as stream
CsvStream<int16_t> csvStream(Serial);
StreamCopy copier(csvStream, i2sStream); // copy i2sStream to csvStream

int16_t audio_buffer[128] = {0};
int pointer = 0;


HTTPClient http;
String SERVER_URL = "IP";


void setup_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin("wifi", "password");
  M5.Lcd.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    M5.Lcd.print('.');
    delay(1000);
  }
}


// Arduino Setup
void setup(void) {
    M5.begin();
    setup_wifi();
    Serial.begin(115200);
    AudioLogger::instance().begin(Serial, AudioLogger::Info);
    
    auto cfg = i2sStream.defaultConfig(RX_MODE);
    cfg.i2s_format = I2S_STD_FORMAT; // or try with I2S_LSB_FORMAT
    cfg.bits_per_sample = 16;
    cfg.channels = 2;
    cfg.sample_rate = 44100;
    cfg.is_master = true;
     // this module nees a master clock if the ESP32 is master
    cfg.use_apll = false;  // try with yes
    //cfg.pin_mck = 3; 
    i2sStream.begin(cfg);

    // make sure that we have the correct channels set up
    csvStream.begin(cfg);

}

void send_audio () {  // void* arg
  M5.Lcd.print("*");
  http.begin(SERVER_URL);
  http.addHeader("Content-Type", "text/plain");
  int httpCode = http.POST((char *) audio_buffer);
  http.end();
}

// Arduino loop - copy data
void loop() {
    copier.copy();
    while (Serial.available() && pointer < sizeof(audio_buffer)) {
      audio_buffer[pointer] = Serial.read();
      pointer++;
    }
    if (pointer == sizeof(audio_buffer)) {
      pointer = 0;
      send_audio();
    }
    M5.Lcd.print((char *) audio_buffer);
}
