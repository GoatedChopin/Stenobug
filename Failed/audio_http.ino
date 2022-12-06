#include <M5StickC.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <driver/i2s.h>

#define PIN_CLK  0
#define PIN_DATA 34
#define READ_LEN (2 * 1024)
#define SAMPLING_FREQUENCY 44100
#define headerSize 44  // WAVE header size
//#define RECORD_TIME (3)
//#define WAVE_SIZE (1 * SAMPLING_FREQUENCY * 16 / 8 * RECORD_TIME)

byte WAVE[READ_LEN + headerSize] = {0};
bool time_to_send = false;

int flash_wr_size = 0;
uint8_t BUFFER[READ_LEN] = {0};
uint16_t *adcBuffer = NULL;
size_t BUFFER_SIZE_T = 2048;

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

void i2sInit(){
   i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
    .sample_rate =  44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // is fixed at 12bit, stereo, MSB
    .channel_format = I2S_CHANNEL_FMT_ALL_RIGHT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,
    .dma_buf_len = 128,
   };

   i2s_pin_config_t pin_config;
   pin_config.bck_io_num   = I2S_PIN_NO_CHANGE;
   pin_config.ws_io_num    = PIN_CLK;
   pin_config.data_out_num = I2S_PIN_NO_CHANGE;
   pin_config.data_in_num  = PIN_DATA;
  
   i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
   i2s_set_pin(I2S_NUM_0, &pin_config);
   i2s_set_clk(I2S_NUM_0, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}

void wavHeader(byte* header, int wavSize){
  header[0] = 'R';
  header[1] = 'I';
  header[2] = 'F';
  header[3] = 'F';
  unsigned int fileSize = wavSize + headerSize - 8;
  header[4] = (byte)(fileSize & 0xFF);
  header[5] = (byte)((fileSize >> 8) & 0xFF);
  header[6] = (byte)((fileSize >> 16) & 0xFF);
  header[7] = (byte)((fileSize >> 24) & 0xFF);
  header[8] = 'W';
  header[9] = 'A';
  header[10] = 'V';
  header[11] = 'E';
  header[12] = 'f';
  header[13] = 'm';
  header[14] = 't';
  header[15] = ' ';
  header[16] = 0x10;
  header[17] = 0x00;
  header[18] = 0x00;
  header[19] = 0x00;
  header[20] = 0x01;
  header[21] = 0x00;
  header[22] = 0x01;
  header[23] = 0x00;
  header[24] = 0x80;
  header[25] = 0x3E;
  header[26] = 0x00;
  header[27] = 0x00;
  header[28] = 0x00;
  header[29] = 0x7D;
  header[30] = 0x00;
  header[31] = 0x00;
  header[32] = 0x02;
  header[33] = 0x00;
  header[34] = 0x10;
  header[35] = 0x00;
  header[36] = 'd';
  header[37] = 'a';
  header[38] = 't';
  header[39] = 'a';
  header[40] = (byte)(wavSize & 0xFF);
  header[41] = (byte)((wavSize >> 8) & 0xFF);
  header[42] = (byte)((wavSize >> 16) & 0xFF);
  header[43] = (byte)((wavSize >> 24) & 0xFF);
  for (int i=44; i<READ_LEN+44; i++) {
    header[i] = BUFFER[i-44];
  }
}

void mic_record_task (void* arg){     
  while(1){
    M5.Lcd.print(".");
//    flash_wr_size = 0;
//    while (flash_wr_size < READ_LEN) {
//      
//    }
    i2s_read(I2S_NUM_0,(char*)BUFFER, BUFFER_SIZE_T, &BUFFER_SIZE_T,(100/portTICK_RATE_MS));  // was 100/portTICK_RATE_MS
    adcBuffer = (uint16_t *)BUFFER;
    vTaskDelay(100 / portTICK_RATE_MS);
    wavHeader(WAVE, READ_LEN + headerSize);
    time_to_send=true;
  }
}

void send_file_task () {  // void* arg
  M5.Lcd.print("*");
  setup_wifi();
  http.begin(SERVER_URL);
  http.addHeader("Content-Type", "text/plain");
  int httpCode = http.POST((char *) WAVE);
  http.end();
}

void setup() {
  M5.begin();
//  SPIFFSInit();
  i2sInit();
  xTaskCreatePinnedToCore(mic_record_task,"mic_record_task",2048,NULL,1,NULL,1);
}

void loop() {
  if (time_to_send) {
    time_to_send = false;
    send_file_task();
  }
}

  int httpCode = http.POST((char *) WAVE);
  http.end();
}

void setup() {
  M5.begin();
//  SPIFFSInit();
  i2sInit();
  xTaskCreatePinnedToCore(mic_record_task,"mic_record_task",2048,NULL,1,NULL,1);
}

void loop() {
  if (time_to_send) {
    time_to_send = false;
    send_file_task();
  }
}
