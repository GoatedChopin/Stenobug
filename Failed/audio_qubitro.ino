// https://github.com/m5stack/M5StickC-Plus/blob/master/examples/Basics/Micophone/Micophone.ino

#include <M5StickCPlus.h>
#include <driver/i2s.h>
#include <PubSubClient.h> // This line includes the PubSub library for MQTT.
#include <WiFi.h>         // This line includes the WiFi library (for WiFi).

#define PIN_CLK     0
#define PIN_DATA    34
#define READ_LEN    (2 * 256)
#define GAIN_FACTOR 3

uint8_t BUFFER[READ_LEN] = {0};

int16_t *adcBuffer = NULL;

const char* ssid = "wifi";  // This line sets the SSID, WiFi name.
const char* password = "password";  // This line sets the password for connecting to WiFi.
const char* mqtt_server = "IP"; // This line sets the IP address of the MQTT server we'll be publishing to.

WiFiClient espClient; // This line declares the WiFi client we'll be using to create our PubSubClient later during Setup.
PubSubClient * client;  // This line declares a pointer, I believe. We've received literally zero instruction on the C / C++ programming languages.


unsigned long lastMsg = 0;  // This line This line declares a variable "lastMsg" of data type "long".
#define MSG_BUFFER_SIZE (500) // This line defines the message buffer size we'll be using in our PubSub.
char msg[MSG_BUFFER_SIZE];  // This line sets a msg with the previously defined MSG_BUFFER_SIZE variable.
int value = 0;  // This line creates the "value" variable with initial state of 0.

void publishSignal() {
//    client->publish("audio", adcBuffer, READ_LEN, false)
    client->beginPublish("audio", 160, false);
    for (int n = 0; n < 160; n++) {
        uint8_t val = adcBuffer[n] >= 0 ? adcBuffer[n] : -adcBuffer[n];
        client->write(val);
    }
    client->endPublish();
}

void mic_record_task(void *arg) {
    size_t bytesread;
    for (int n = 0; n < 160; n++) {
        i2s_read(I2S_NUM_0, (char *)BUFFER, READ_LEN, &bytesread,
                 (100 / portTICK_RATE_MS));
        adcBuffer = (int16_t *)BUFFER;
        vTaskDelay(100 / portTICK_RATE_MS);
    }
    publishSignal();
}

void i2sInit() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
        .sample_rate = 44100,
        .bits_per_sample =
            I2S_BITS_PER_SAMPLE_16BIT,  // is fixed at 12bit, stereo, MSB
        .channel_format = I2S_CHANNEL_FMT_ALL_RIGHT,
#if ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(4, 1, 0)
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
#else
        .communication_format = I2S_COMM_FORMAT_I2S,
#endif
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count    = 2,
        .dma_buf_len      = 128,
    };

    i2s_pin_config_t pin_config;

#if (ESP_IDF_VERSION > ESP_IDF_VERSION_VAL(4, 3, 0))
    pin_config.mck_io_num = I2S_PIN_NO_CHANGE;
#endif

    pin_config.bck_io_num   = I2S_PIN_NO_CHANGE;
    pin_config.ws_io_num    = PIN_CLK;
    pin_config.data_out_num = I2S_PIN_NO_CHANGE;
    pin_config.data_in_num  = PIN_DATA;

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_set_clk(I2S_NUM_0, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}


void setup_wifi() {
  delay(10);  // This line delays the program by a short period
  // We start by connecting to a WiFi network
  M5.Lcd.println(); // This line prints a newline to the screen.
  M5.Lcd.print("Connecting to "); // This line prints "Connecting to" to the screen.
  M5.Lcd.println(ssid); // This line prints "utexas-iot" to the screen.

  //WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); // This line begins the wifi connection using our ssid and password.

  while (WiFi.status() != WL_CONNECTED) { // This line is a while loop. We stay in it until we're connected to the WiFi.
    delay(500); // This line delays the program a short amount.
    M5.Lcd.print(".");  // This line prints a period to the screen.
  }

  randomSeed(micros()); // This line sets the random seed for our program using the micros() function.

  M5.Lcd.println(""); // This line prints a newline to the screen.
  M5.Lcd.println("WiFi connected"); // This line confirms we're connected to WiFi by printing "WiFi connected" to the screen.
  M5.Lcd.println("IP address: "); // This line prints "IP address: " to the screen.
  M5.Lcd.println(WiFi.localIP()); // This line prints our IP address to the screen.
}


void callback(char* topic, byte* payload, unsigned int length) {
  M5.Lcd.print("Message arrived [");  // This line prints "Message arrived [" to the screen.
  M5.Lcd.print(topic);  // This line prints the topic parameter to the screen.
  M5.Lcd.print("] "); // This line prints "]" to the screen.
  for (int i = 0; i < length; i++) {  // This line is a for loop. We iterate from zero to length-1.
    M5.Lcd.print((char)payload[i]); // This line prints the contents of the payload parameter to the screen one chunk at a time.
  }
  M5.Lcd.println(); // This line prints a newline to the screen.
}


void reconnect() {
  // Loop until we’re reconnected
  while (!client->connected()) {  // This line is a while loop. We stay in it untill the MQTT connection is established.
    M5.Lcd.print("Attempting MQTT connection...");  // This line prints "Attempting MQTT connection..." to the screen.
    String clientId = "ESP8266Client - MyClient"; // This line sets the variable clientId to "ESP8266Client - MyClient".
    // Attempt to connect
    // Insert your password
    if (client->connect(clientId.c_str(), "username", "password")) {  // This line checks to see if the client is connected with the correct clientId, username, and password.
      M5.Lcd.println("connected");  // This line prints "connected" to the screen.
      // Once connected, publish an announcement…
      client->publish("testTopic", "hello world");  // This line publishes "hello world" to the "testTopic" topic.
      // … and resubscribe
      client->subscribe("audio"); // This line subscribes to the "testTopic" topic.
    } else {
      M5.Lcd.print("failed, rc = ");  // This line prints "failed, rc = " to the screen.
      M5.Lcd.print(client->state());  // This line prints the client's state to the screen.
      M5.Lcd.println(" try again in 5 seconds");  // This line prints " try again in 5 seconds" to the screen.
      // Wait 5 seconds before retrying
      delay(5000);  // This line delays the program by a short period.
    }
  }
}


void setup() {
  delay(500); // This line delays the program by a short period.
  // When opening the Serial Monitor, select 9600 Baud
  M5.begin(); // This line initializes the M5Stick
  delay(500); // This line delays the program by a short period.
  setup_wifi(); // This line calls the setup_wifi function

  client = new PubSubClient(espClient); // This line sets the client variable to a new instance of the PubSubClient object, using the espClient variable.

  client->setServer(mqtt_server, 1883); // This line sets the server we're connecting to to the mqtt_server variable defined above, at port 1883.
  client->setCallback(callback);  // This line sets the client's callback to the callback function from above.

  // mic setup
  i2sInit();
  xTaskCreate(mic_record_task, "mic_record_task", 2048, NULL, 1, NULL);
  
}

void loop() {
  if (!client->connected()) { // This line checks to see if the client is connected.
    reconnect();  // This line calls the reconnect function.
  }
  // fill the audio buffer
  vTaskDelay(1000 / portTICK_RATE_MS);
//  mic_record_task(NULL);
  client->loop(); // Maintain the connection.

//  unsigned long now = millis(); // This line sets the now variable using the millis function (current time in milliseconds)
//  if (now - lastMsg > 2000) { // This line checks to see if the current time is more than 2000 milliseconds greater than the previous message's time.
//    lastMsg = now;  // This line updates lastMsg to now.
//    ++value;  // This line increments value
//    // snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value); // This line composes a string with the same text that would be printed if format was used on printf, but instead of being printed, the content is stored as a C string in the buffer pointed by s (taking n as the maximum buffer capacity to fill).
//    M5.Lcd.print("Publishing audio");  // This line prints "Publish message: " to the screen.
//    // M5.Lcd.println(msg);  // This line prints the msg variable to the screen.
//    client->publish("audio", adcBuffer);  // This line publishes the msg to the testTopic topic.
//  }
}


// **************************************************************
