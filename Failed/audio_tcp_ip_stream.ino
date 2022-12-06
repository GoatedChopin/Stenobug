#include <M5StickC.h>
#include "AudioTools.h"
#include <WiFi.h>

uint16_t sample_rate = 16000;
uint8_t channels = 1;  // The stream will have 2 channels
SineWaveGenerator<int16_t> sineWave( 32000);  // subclass of SoundGenerator with max amplitude of 32000
GeneratedSoundStream<int16_t> sound( sineWave);  // Stream generated from sine wave
WiFiClient client;                  
MeasuringStream clientTimed(client);
StreamCopy copier(clientTimed, sound, 256);  // copies sound into i2s
const char *ssid = "wifi";
const char *password = "password";
const char *client_address = "IP"; // update based on your receive ip
uint16_t port = 5021;

void connectWifi() {
  // connect to WIFI
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print("*");
  }
  Serial.println();
  Serial.println(WiFi. localIP());

  // Performance Hack              
  client.setNoDelay(true);
  esp_wifi_set_ps(WIFI_PS_NONE);
}

void connectIP() {
  if (!client.connected()){
    while (!client.connect(client_address, port)) {
      Serial.println("trying to connect...");
      delay(5000);
    }    
  }
}

void setup() {
  M5.begin();
  M5.Lcd.print("Starting TCP/IP connection");
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);

  connectWifi();
  // Setup sine wave
  sineWave.begin(channels, sample_rate, N_B4);
  Serial.println("started...");
}

void loop() { 
  connectIP();  // e.g if client is shut down we try to reconnect
  copier.copy();
  M5.Lcd.print(".");
}
