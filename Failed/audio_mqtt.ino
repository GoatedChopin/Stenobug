// https://framagit.org/getlarge/mqtt-microphone/-/blob/master/microphone-mqtt.ino

#include "M5StickCPlus.h" // This line includes the M5StickPlus library.
#include <PubSubClient.h> // This line includes the PubSub library for MQTT.
#include <WiFi.h>         // This line includes the WiFi library (for WiFi).
#include <driver/i2s.h>


#define AUDIO_BUFFER_MAX 800

const char* ssid = "wifi";  // This line sets the SSID, WiFi name.
const char* password = "password";  // This line sets the password for connecting to WiFi.
const char* mqtt_server = "IP"; // This line sets the IP address of the MQTT server we'll be publishing to.

WiFiClient espClient; // This line declares the WiFi client we'll be using to create our PubSubClient later during Setup.
PubSubClient * client;  // This line declares a pointer, I believe. We've received literally zero instruction on the C / C++ programming languages.

unsigned long lastMsg = 0;  // This line This line declares a variable "lastMsg" of data type "long".
#define MSG_BUFFER_SIZE (500) // This line defines the message buffer size we'll be using in our PubSub.
char msg[MSG_BUFFER_SIZE];  // This line sets a msg with the previously defined MSG_BUFFER_SIZE variable.
int value = 0;  // This line creates the "value" variable with initial state of 0.

uint8_t audioBuffer[AUDIO_BUFFER_MAX];
uint8_t transmitBuffer[AUDIO_BUFFER_MAX];
uint32_t bufferPointer = 0;
bool transmitNow = false;

hw_timer_t * timer = NULL; // our timer
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux); // says that we want to run critical code and don't want to be interrupted
  int adcVal = adc1_get_voltage(ADC1_CHANNEL_0); // reads the ADC
  uint8_t value = map(adcVal, 0 , 4096, 0, 255);  // converts the value to 0..255 (8bit)
  
  audioBuffer[bufferPointer] = value; // stores the value
  bufferPointer++;
 
  if (bufferPointer == AUDIO_BUFFER_MAX) { // when the buffer is full
    bufferPointer = 0;
    memcpy(transmitBuffer, audioBuffer, AUDIO_BUFFER_MAX); // copy buffer into a second buffer
    transmitNow = true; // sets the value true so we know that we can transmit now
  }
  portEXIT_CRITICAL_ISR(&timerMux); // says that we have run our critical code
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
      client->subscribe("testTopic"); // This line subscribes to the "testTopic" topic.
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
  
  timer = timerBegin(0, 80, true); // 80 Prescaler
  timerAttachInterrupt(timer, &onTimer, true); // binds the handling function to our timer 
  timerAlarmWrite(timer, 125, true);
  timerAlarmEnable(timer);

}

void loop() {
  if (!client->connected()) { // This line checks to see if the client is connected.
    reconnect();  // This line calls the reconnect function.
  }
  client->loop(); // Not sure if this should be here.

  unsigned long now = millis(); // This line sets the now variable using the millis function (current time in milliseconds)
  if (transmitNow) { // checks if the buffer is full
          transmitNow = false;
    //      ++value;
    //      snprintf (msg, 75, "hello world #%ld", value);
          Serial.print(F("Publish message: "));
          //Serial.println(msg);
          //client.publish("outTopic", msg);
          //Serial.write((const uint8_t *)audioBuffer, sizeof(audioBuffer));
          client.publish("outTopic", (const uint8_t *)audioBuffer, sizeof(audioBuffer));
  }

}
