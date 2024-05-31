//importing required libraries, defining
#include <Arduino.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "PageIndex.h"

//defining pin and variables
#define TRIGGER_PIN D5
#define ECHO_PIN D6
#define BUZZER_PIN D7

String ssid, password, deviceId, message;
boolean buzzerState = true;
long duration;    
float distanceCm; 
float updatedRangeValue = 5; 
int webType; 

// Create an instance of the web server on port 80
ESP8266WebServer server(80);

void setup() {
  //initiate serial and eeprom
  Serial.begin(115200);
  EEPROM.begin(512);
  // Load ssid, password, deviceId, and buzzerState from EEPROM
  readData();
  delay(100);
  //set pin as input and output
  pinMode(ECHO_PIN, INPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  //test wifi connection (WebType 1 -> Connected; WebType 0 -> APMode)
  if (testWiFi()) {
    Serial.println("Connected to WiFi!");
    Serial.println("Web Type 1");
    Serial.println("");
    webType = 1;
    digitalWrite(BUZZER_PIN, LOW);  //off
    launchWeb(1);
  } else {
    const char* ssidap = "NodeMCU-AP";
    const char* passap = "";
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssidap, passap);
    Serial.println("Connection Failed! Launching AP Mode!");
    Serial.print("http://");
    Serial.println(WiFi.softAPIP());
    Serial.println("Web Type 0");
    webType = 0;
    launchWeb(0);
    Serial.println("");
  }
}

void loop() {
  if (webType == 1) {
    checkUltrasonic();  //ultrasonic to work only in WebType 1
  }
  server.handleClient();  // Handle client requests
}

//set trigpin + calculate distance + update message and buzzer
void checkUltrasonic() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH); 
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW); 

  // Read echoPin->sound wave travel time + calculate distance
  duration = pulseIn(ECHO_PIN, HIGH);
  distanceCm = duration * 0.034 / 2;

  //message and buzzer changes
  if (distanceCm < updatedRangeValue) {
    Serial.println("Item Nearby!");
    digitalWrite(BUZZER_PIN, LOW);
    message = "Item within Range!";
  } else {
    digitalWrite(BUZZER_PIN, HIGH);
    message = "Nothing Range!";
  }

  //serial print
  Serial.print("Distance: ");
  Serial.print(distanceCm);
  Serial.println(" cm");
  Serial.println("");

  delay(1000);  // Delay to avoid excessive triggering and noise
}

//create and start web server
void launchWeb(int webtype) {
  createWebServer(webtype);
  server.begin();
}

void createWebServer(int webtype) {
  if (webtype == 0) {
    server.on("/", []() {
      //webserver code and parameters
      String configpage = FPSTR(CONFIG_page);
      configpage.replace("%%SSID%%", ssid);
      configpage.replace("%%PASSWORD%%", password);
      configpage.replace("%%DEVICEID%%", deviceId);
      configpage.replace("%%BUZZERON%%", buzzerState ? "checked" : "");
      configpage.replace("%%BUZZEROFF%%", !buzzerState ? "checked" : "");
      server.send(200, "text/html", configpage);
    });

    server.on("/setting", []() {
      //save to eeprom
      ssid = server.arg("ssid");
      password = server.arg("password");
      deviceId = server.arg("deviceId");
      buzzerState = (server.arg("buzzerState") == "1");
      digitalWrite(BUZZER_PIN, buzzerState ? HIGH : LOW);  // Set buzzerstate (HIGH = ON, LOW = OFF)
      writeData(ssid, password, deviceId, buzzerState);
      //webpage code
      String rebootpage = FPSTR(REBOOT_page);
      server.send(200, "text/html", rebootpage);
      //serial print
      Serial.println("Reboot for effect");
      Serial.println("");
    });
  }

  if (webtype == 1) {
    server.on("/", []() {
      //webpage code and parameter
      String mainpage = FPSTR(MAIN_page);
      mainpage.replace("%%DISTANCECM%%", String(distanceCm));
      mainpage.replace("%%RANGEVALUE%%", String(updatedRangeValue));
      mainpage.replace("%%MESSAGE%%", message);
      server.send(200, "text/html", mainpage);
    });

    //update latest range + limit value from 0 to 400
    server.on("/update", HTTP_GET, []() {
      float newRange = server.arg("range").toFloat();
      if (newRange > 0 && newRange <= 400) {
        updatedRangeValue = newRange;
        Serial.print("Updated Detection Range: ");
        Serial.println(updatedRangeValue);
      } else {
        Serial.println("Invalid range value. Please enter a value between 1 and 400.");
      }
      server.sendHeader("Location", "/");
      server.send(303);
    });

    server.on("/distance", HTTP_GET, []() {
      server.send(200, "text/plain", String(distanceCm));
    });
  }
}

//check wifi conectivity
boolean testWiFi() {
  WiFi.begin(ssid.c_str(), password.c_str());
  int c = 0;
  while (c < 10) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println(WiFi.status());
      Serial.println(WiFi.localIP());
      Serial.println("");
      return true;
    }
    Serial.print(".");
    delay(500);
    c++;
  }
  Serial.println("Connection time out");
  Serial.println("");
  return false;
}

//write data to eeprom
void writeData(String a, String b, String c, bool d) {
  Serial.println("");
  Serial.println("Writing to EEPROM");
  for (int i = 0; i < 20; i++) {
    if (i < a.length()) {
      EEPROM.write(i, a[i]);
    } else {
      EEPROM.write(i, 0);
    }
  }
  for (int i = 20; i < 40; i++) {
    if (i - 20 < b.length()) {
      EEPROM.write(i, b[i - 20]);
    } else {
      EEPROM.write(i, 0);
    }
  }
  for (int i = 40; i < 60; i++) {
    if (i - 40 < c.length()) {
      EEPROM.write(i, c[i - 40]);
    } else {
      EEPROM.write(i, 0);
    }
  }
  EEPROM.write(80, d ? 1 : 0);  // Save buzzerState as single byte (1 for true, 0 for false)
  EEPROM.commit();
  Serial.println("Write successful.");
  Serial.println("");
}

//read data from eeprom
void readData() {
  Serial.println("");
  Serial.println("Reading from EEPROM....");
  char ssidArr[21];
  char passwordArr[21];
  char deviceIdArr[21];

  for (int i = 0; i < 20; i++) {
    ssidArr[i] = char(EEPROM.read(i));
  }
  ssidArr[20] = '\0';

  for (int i = 20; i < 40; i++) {
    passwordArr[i - 20] = char(EEPROM.read(i));
  }
  passwordArr[20] = '\0';

  for (int i = 40; i < 60; i++) {
    deviceIdArr[i - 40] = char(EEPROM.read(i));
  }
  deviceIdArr[20] = '\0';

  ssid = String(ssidArr);
  password = String(passwordArr);
  deviceId = String(deviceIdArr);
  buzzerState = EEPROM.read(80) == 1;

  Serial.println("");
  Serial.println("WiFi ssid from EEPROM: " + ssid);
  Serial.println("WiFi password from EEPROM: " + password);
  Serial.println("Device ID from EEPROM: " + deviceId);
  Serial.println("");
  Serial.println("Reading successful.");
}
