////////////////////////////////////////////////////////////////////////////////
// This code is uploaded on the NodeMCU module
// It send data to the Arduino UNO
//
// Daily data obtained through CoronavirusAPI-France :
// https://github.com/florianzemma/CoronavirusAPI-France
//
// Uses Rui Santos code example found here :
// https://RandomNerdTutorials.com/esp8266-nodemcu-http-get-post-arduino/
////////////////////////////////////////////////////////////////////////////////

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include <SoftwareSerial.h>
// We send data to Arduino through Serial1 (pin D4)

// Set your personal network credentials
const char* ssid = "Your Network SSID";
const char* password = "Your Network password";

// Set the COVID tracker API URL
String serverName = "http://coronavirusapifr.herokuapp.com/data/live/france";
String RawDailyData;

// Set timer to fetch data every 5 seconds (5000 ms)
unsigned long timerDelay = 5000;
// Once everything is running and debug done, you can set it to longer times since the numbers are not refreshing that fast.
unsigned long lastTime = 0;

// ---------   SETUP START -----------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}
// ---------   SETUP END -------------------------------------------------------

// ---------   LOOP START ------------------------------------------------------
void loop() {

// ---------   Send an HTTP GET request depending on timerDelay
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED) {
      WiFiClient client;
      HTTPClient http;
      http.begin(client, serverName);
      int httpCode = http.GET();
      if (httpCode > 0) {
        // Get the JSON as a text string and send it to Arduino through Serial1
        RawDailyData = http.getString();
        Serial1.println(RawDailyData);
        delay(1000);    }
      else {
        Serial.println("Error : ");
        Serial.println(httpCode); }
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }

    lastTime = millis(); // update timer count
  } // End of timber

}
// ---------   LOOP END --------------------------------------------------------
