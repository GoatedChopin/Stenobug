#include <M5StickCPlus.h> 
#include <WiFi.h>
#include <HTTPClient.h>

/*
UPLOAD DATA OVER WIFI
Jeff Thompson | 2017 | jeffreythompson.org
The great Arduino Wifi101 library includes lots of examples
of how to *get* data from the internet, but no examples on how
to upload!
Here, we use HTTP POST commands to send sensor readings to
a server. A simple PHP script parses those readings and
updates a text file.
   <?php
     $dir =   $_POST["dir"];
     $speed = $_POST["speed"];
     $batt =  $_POST["batt"];
     $data =  $dir . ',' . $speed . ',' . $batt;
     $file =  './current.txt';
     file_put_contents($file, $data);
   ?>
Based on this example, and many hours of Google searching:
https://cdn.instructables.com/FLH/GRS2/HXB3UZG6/FLHGRS2HXB3UZG6.LARGE.jpg
*/

// server to connect to and relative path to PHP script
char server[] =    "url";
//String phpScript = "/path/update.php";

// wifi network and password
char ssid[] = "wifi";
char pass[] = "password";

int status = WL_IDLE_STATUS;
int keyIndex = 0;
HTTPClient client;


void setup() {

  M5.begin();
  WiFi.mode(WIFI_STA);
  WiFi.begin("wifi", "password");
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    M5.Lcd.print('.');
    delay(1000);
  }
  // some data to send (fake wind sensor readings)
  float windDir =   380.0;
  float windSpeed = 2.2;
  float battLevel = 3.85;

  // format datapoints as PHP arguments
  // this means easy parsing later in the PHP script
  String data = "&dir=" + String(windDir, 2) + "&speed=" + String(windSpeed, 2) + "&batt=" + String(battLevel, 2);


  // post data using HTTP POST
  M5.Lcd.println("Connecting to server...");

    M5.Lcd.println("- connected");

    M5.Lcd.println("Posting sensor data...");
    client.print("POST ");
    client.print("/echo");
    client.println(" HTTP/1.1");
    client.println("IP");
//    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded; charset=UTF-8");
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.print(data);
    M5.Lcd.println("- done");

  // disconnect when done
  M5.Lcd.println("Disconnecting from server...");
  client.stop();
  M5.Lcd.println("- bye!");
}

void loop() {
  // you could also move your HTTP POST code here, if you
  // wanted to post repeatedly
}
