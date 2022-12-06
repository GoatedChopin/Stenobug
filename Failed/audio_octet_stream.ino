#include <M5StickCPlus.h> 
#include <WiFi.h>
#include <HTTPClient.h>

// Code for the http client initialization
// ************************************

WiFiClientSecure client;
HTTPClient http;
String serverPath = "/echo";
String SERVER_URL = "IP";
String SERVER_PORT = "5000";
//String DATASET_REPOSITORY_URL = "https://INSERT_CLOUD_RUN_URL.app/whisper/";
String recordingName = "recording.wav";
String contentType = "application/octet-stream";
String mock_data = "This is a mock message for the audio";


void setup(){ 
  M5.begin(); 
  WiFi.mode(WIFI_STA);
  WiFi.begin("wifi", "password");
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    M5.Lcd.print('.');
    delay(1000);
  }
//  Serial.println(WiFi.localIP());
  M5.Lcd.print(WiFi.localIP());
  M5.Lcd.print("\n");

  int str_len = SERVER_URL.length() + 1; // Length (with one extra character for the null terminator)
  char SERVER_URL_char [str_len];    // Prepare the character array (the buffer) 
  SERVER_URL.toCharArray(SERVER_URL_char, str_len);    // Copy it over 
  String boundary = "7MA4YWxkTrZu0gW";

  // Make a HTTP request and add HTTP headers    
  // post header
  String postHeader = "POST " + serverPath + " HTTP/1.1\r\n";
  postHeader += "Host: " + SERVER_URL + ":" + String(SERVER_PORT) + "\r\n";
  postHeader += "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n";
  postHeader += "Keep-Alive: 300\r\n";  // Not sure if this part will work out of the box, might need to remove these two lines.
  postHeader += "Connection: keep-alive\r\n";

  // dataset header
  String datasetHead = "--" + boundary + "\r\n";
  datasetHead += "Content-Disposition: form-data; name=\"file\"; filename=\"" + recordingName + "\"\r\n";
  datasetHead += "Content-Type: " + contentType + "\r\n\r\n"; // Where is contentType defined? *** Seems like a problem point -> should be 'application/octet-stream'

  // request tail
  String tail = "\r\n--" + boundary + "--\r\n\r\n";

  // content length
  int contentLength = datasetHead.length() + mock_data.length() + tail.length();
  postHeader += "Content-Length: " + String(contentLength, DEC) + "\n\n";

  // send post header
  int postHeader_len = postHeader.length() + 1; 
  char charBuf0[postHeader_len];
  postHeader.toCharArray(charBuf0, postHeader_len);
  client.print(charBuf0);

  // send request buffer
  char charBuf1[datasetHead.length() + 1];
  datasetHead.toCharArray(charBuf1, datasetHead.length() + 1);
  client.print(charBuf1);
  
  // create buffer
  const int bufSize = 2048;
  byte clientBuf[bufSize];
  int clientCount = 0;

for (int i = 0; i < mock_data.length(); i++) {
    char c = mock_data[i];
    clientBuf[clientCount] = byte(c);
    clientCount++;
    if (clientCount > (bufSize - 1)) {
        client.write((const uint8_t *)clientBuf, bufSize);
        clientCount = 0;
    }
  }

  if (clientCount > 0) {
    client.write((const uint8_t *)clientBuf, clientCount);
  }

  // send tail
  char charBuf3[tail.length() + 1];
  tail.toCharArray(charBuf3, tail.length() + 1);
  client.print(charBuf3);

  M5.Lcd.println("POST Request Sent");
}
 
void loop() {
}
