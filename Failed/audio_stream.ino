#include <M5StickC.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <AudioTools.h>
#include <SPIFFS.h>


#define PIN_CLK  0
#define PIN_DATA 34
#define READ_LEN (2 * 1024)


uint16_t sample_rate = 44100;
uint8_t channels = 1;                                             // The stream will have 2 channels 
NoiseGenerator<int16_t> noise(32000);                             // subclass of SoundGenerator with max amplitude of 32000
GeneratedSoundStream<int16_t> in_stream(noise);                   // Stream generated from sine wave
File file;                                                   // final output stream
File recording;
const char filename[] = "/recording.wav";
EncodedAudioStream out_stream(&file, new WAVEncoder());             // encode as wav file
StreamCopy copier(out_stream, in_stream);                                // copies sound to out

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


void SPIFFSInit(){
  if(!SPIFFS.begin(true)){
    M5.Lcd.println("SPIFFS initialisation failed!");
    while(1) yield();
  }

  SPIFFS.remove(filename);
  file = SPIFFS.open(filename, FILE_WRITE);
  if(!file){
    M5.Lcd.println("File is not available!");
  }
  file.close();  // Added this line, might remove
}


void send_file_task () {  // void* arg
  M5.Lcd.print("*");
  http.begin(SERVER_URL);
  recording = SPIFFS.open("/recording.wav", "rb");
  http.addHeader("Content-Type", "text/plain");
  int httpCode = http.POST(recording.readString());
  http.end();
  recording.close();
}


void setup(){
  M5.begin();
  setup_wifi();
  SPIFFSInit();
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);  

  auto cfg = noise.defaultConfig();  // Added RX_MODE, probably need to remove.
  cfg.sample_rate = sample_rate;
  cfg.channels = channels;
  cfg.bits_per_sample = 16;
  noise.begin(cfg);
  in_stream.begin();
  file = SPIFFS.open("/recording.wav", "wb");
  // we need to provide the audio information to the encoder
  out_stream.begin(cfg);
  // open the output file
//  SD.begin(SdSpiConfig(PIN_CS, DEDICATED_SPI, SD_SCK_MHZ(2)));
//  audioFile = SD.open("/test/002.wav", O_WRITE | O_CREAT);
}

void loop(){
    copier.copy();
    send_file_task();
    recording = SPIFFS.open("/recording.wav", "rb");
    while (recording.available()) {
      M5.Lcd.print(recording.read());
    }
}
