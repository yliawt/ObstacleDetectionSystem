#include <Arduino.h>  // Arduino core library
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "PageIndex.h"
#define BUZZER_PIN D7
#define TRIGGER_PIN D5  // Define trigger pin for ultrasonic sensor
#define ECHO_PIN D6     // Define echo pin for ultrasonic sensor

String ssid, password, deviceId, message;
boolean buzzerState = true;
long duration;                // Duration of the sound wave travel time
float distanceCm;             // Calculated distance in cm
float updatedRangeValue = 5;  // Default detection range
int webType;                  // Global variable to store web type

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  readData();  // Load ssid, password, deviceId, and buzzerState from EEPROM
  delay(100);
  pinMode(ECHO_PIN, INPUT);      // Set the echoPin as an Input
  pinMode(TRIGGER_PIN, OUTPUT);  // Set the trigPin as an Output
  pinMode(BUZZER_PIN, OUTPUT);   // Set buzzer pin as output

  if (testWiFi()) {
    Serial.println("Connected to WiFi!");
    Serial.println("Web Type 1");
    Serial.println("");
    webType = 1;                    // Set web type to 1
    digitalWrite(BUZZER_PIN, LOW);  // Ensure the buzzer is off initially
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
    webType = 0;  // Set web type to 0
    launchWeb(0);
    Serial.println("");
  }
}

void loop() {
  if (webType == 1) {
    checkUltrasonic();  // Only check ultrasonic sensor when webType is 1
  }
  server.handleClient();
}

void checkUltrasonic() {
  digitalWrite(TRIGGER_PIN, LOW);   // Clear the trigPin
  delayMicroseconds(2);             // Delay for stability
  digitalWrite(TRIGGER_PIN, HIGH);  // Set the trigPin on HIGH state for 10 microseconds
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);  // Reset the trigPin

  // Read the echoPin to get the sound wave travel time
  duration = pulseIn(ECHO_PIN, HIGH);
  // Calculate the distance
  distanceCm = duration * 0.034 / 2;

  if (distanceCm < updatedRangeValue) {  // Check if the distance is less than the updated range value
    Serial.println("Item Nearby!");      // Print message if web type is 1
    digitalWrite(BUZZER_PIN, LOW);  // Turn on the buzzer
    message="Item within Range!";
  } else {
    digitalWrite(BUZZER_PIN, HIGH);  // Turn off the buzzer
    message="Nothing Range!";
  }

  Serial.print("Distance: ");  // Print message if web type is 1
  Serial.print(distanceCm);    // Print distance if web type is 1
  Serial.println(" cm");       // Print units if web type is 1
  Serial.println("");          // Print blank line if web type is 1

  delay(1000);  // Delay to avoid excessive triggering and noise
}

void launchWeb(int webtype) {
  createWebServer(webtype);
  server.begin();
}

void createWebServer(int webtype) {
  if (webtype == 0) {
    server.on("/", []() {
      String configpage = FPSTR(CONFIG_page);
      configpage.replace("%%SSID%%", ssid);
      configpage.replace("%%PASSWORD%%", password);
      configpage.replace("%%DEVICEID%%", deviceId);
      configpage.replace("%%BUZZERON%%", buzzerState ? "checked" : "");
      configpage.replace("%%BUZZEROFF%%", !buzzerState ? "checked" : "");

      server.send(200, "text/html", configpage);
    });

    server.on("/setting", []() {
  ssid = server.arg("ssid");
  password = server.arg("password");
  deviceId = server.arg("deviceId");
  buzzerState = (server.arg("buzzerState") == "1");
  digitalWrite(BUZZER_PIN, buzzerState ? HIGH : LOW);  // Set relay status (HIGH = ON, LOW = OFF)
  writeData(ssid, password, deviceId, buzzerState);

  String rebootpage = FPSTR(REBOOT_page);
  server.send(200, "text/html", rebootpage);

  Serial.println("Reboot for effect");
  Serial.println("");
});

  }
  if (webtype == 1) {
    server.on("/", []() {
      String mainpage = FPSTR(MAIN_page);
      mainpage.replace("%%DISTANCECM%%", String(distanceCm));
      mainpage.replace("%%RANGEVALUE%%", String(updatedRangeValue));
      mainpage.replace("%%MESSAGE%%", message);
      server.send(200, "text/html", mainpage);
    });

    server.on("/update", HTTP_GET, []() {
      String range = server.arg("range");
      float newRange = range.toFloat();
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
void writeData(String a, String b, String c, bool d) {
  Serial.println("");
  Serial.println("Writing to EEPROM");
  for (int i = 0; i < 20; i++) {
    if (i < a.length()) {
      EEPROM.write(i, a[i]);
    } else {
      EEPROM.write(i, 0);  // Null character to pad the remaining space
    }
  }
  for (int i = 20; i < 40; i++) {
    if (i - 20 < b.length()) {
      EEPROM.write(i, b[i - 20]);
    } else {
      EEPROM.write(i, 0);  // Null character to pad the remaining space
    }
  }
  for (int i = 40; i < 60; i++) {
    if (i - 40 < c.length()) {
      EEPROM.write(i, c[i - 40]);
    } else {
      EEPROM.write(i, 0);  // Null character to pad the remaining space
    }
  }
  EEPROM.write(80, d ? 1 : 0);  // Save buzzerState as a single byte (1 for true, 0 for false)
  EEPROM.commit();
  Serial.println("Write successful.");
  Serial.println("");
}

void readData() {
  Serial.println("");
  Serial.println("Reading from EEPROM....");
  char ssidArr[21];      // 20 characters + null terminator
  char passwordArr[21];  // 20 characters + null terminator
  char deviceIdArr[21];  // 20 characters + null terminator

  for (int i = 0; i < 20; i++) {
    ssidArr[i] = char(EEPROM.read(i));
  }
  ssidArr[20] = '\0';  // Null terminate the SSID string

  for (int i = 20; i < 40; i++) {
    passwordArr[i - 20] = char(EEPROM.read(i));
  }
  passwordArr[20] = '\0';  // Null terminate the password string

  for (int i = 40; i < 60; i++) {
    deviceIdArr[i - 40] = char(EEPROM.read(i));
  }
  deviceIdArr[20] = '\0';  // Null terminate the device ID string

  ssid = String(ssidArr);
  password = String(passwordArr);
  deviceId = String(deviceIdArr);
  buzzerState = EEPROM.read(80) == 1;  // Read the buzzerState as a boolean

  Serial.println("");
  Serial.println("WiFi ssid from EEPROM: " + ssid);
  Serial.println("WiFi password from EEPROM: " + password);
  Serial.println("Device ID from EEPROM: " + deviceId);
  Serial.println("");
  Serial.println("Reading successful.");
}
