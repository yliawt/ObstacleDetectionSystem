#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80); // create WebServer object

const char* ssidap = "Obstacle Detector WiFi"; // Access point SSID
const char* passap = ""; // Access point password

const int trigPin = 12;
const int echoPin = 14;
const int buzzerPin = 13; // Buzzer connected to GPIO 13

// Define sound velocity in cm/uS
#define SOUND_VELOCITY 0.034

long duration;
float distanceCm;
float updatedRangeValue = 5; // Default value for the detection range

void setup() {
  // Starts the serial communication
  Serial.begin(115200); 
 
  // Set up WiFi access point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssidap, passap); // Set SSID and password for the access point
  Serial.println("Connect to WiFi");
  Serial.println("WiFi name: Obstacle Detector WiFi");
  Serial.println(WiFi.softAPIP()); // Print access point IP address
  
  // Create web server routes
  createWebServer();
  server.begin();

  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(buzzerPin, OUTPUT); // Sets the buzzerPin as an Output
}

void loop() {
  server.handleClient(); // Handle web server requests
  
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_VELOCITY / 2;
  
  // Check if the distance is less than the updated range value from the web page
  if (distanceCm < updatedRangeValue) {
    Serial.println("Item Nearby!");
    digitalWrite(buzzerPin, HIGH); // Turn on the buzzer
  } else {
    digitalWrite(buzzerPin, LOW); // Turn off the buzzer
  }

  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distanceCm);
  Serial.println(" cm");
  Serial.println("");

  delay(1000); // Delay to avoid excessive triggering and noise
}

void createWebServer() {
  server.on("/", HTTP_GET, []() {
    String content = "<!DOCTYPE html><html><head><title>Obstacle Detector System</title>";
    content += "<script>";
    content += "function updateDistance() {";
    content += "  var xhttp = new XMLHttpRequest();";
    content += "  xhttp.onreadystatechange = function() {";
    content += "    if (this.readyState == 4 && this.status == 200) {";
    content += "      var distance = parseFloat(this.responseText);";
    content += "      document.getElementById('distance').innerText = distance;";
    content += "      if (distance < " + String(updatedRangeValue) + ") {";
    content += "        document.getElementById('alert').innerText = 'Item Nearby! Detected within " + String(updatedRangeValue) + "';"; 
    content += "      } else {";
    content += "        document.getElementById('alert').innerText = '';";
    content += "      }";
    content += "    }";
    content += "  };";
    content += "  xhttp.open('GET', '/distance', true);";
    content += "  xhttp.send();";
    content += "}";
    content += "setInterval(updateDistance, 1000);"; // Update every second
    content += "</script></head><body style='background-color: #f0f0f0;'>";
    content += "<div style='text-align: center; margin-top: 50px;'>";
    content += "<h1 style='color: #333;'>Obstacle Detection System</h1>";
    content += "<p style='color: #666;'>Detection Range (cm): <span style='color: #f00; font-weight: bold;'>" + String(updatedRangeValue) + "</span></p>";
    content += "<p style='color: #666;'>Distance (cm): <span id='distance' style='color: #00f; font-weight: bold;'>Updating...</span></p>";
    content += "<p id='alert' style='color: #f00; font-weight: bold;'></p>";
    content += "<form action='/update' method='GET'>";
    content += "<label for='range' style='color: #333;'>Detection Range (cm):</label><br>";
    content += "<input type='number' id='range' name='range' min='1' max='400' value='" + String(updatedRangeValue) + "'><br>";
    content += "<input type='submit' value='Update Settings'>";
    content += "</form>";
    content += "</div>";
    content += "</body></html>";
    server.send(200, "text/html", content); // Send response
  });

  server.on("/update", HTTP_GET, []() {
    String range = server.arg("range");

    // Convert range to float and update global variable if needed
    float newRange = range.toFloat();
    if (newRange > 0 && newRange <= 400) {
      updatedRangeValue = newRange; // Update the global variable
      Serial.print("Updated Detection Range: ");
      Serial.println(updatedRangeValue);
    } else {
      Serial.println("Invalid range value. Please enter a value between 1 and 400.");
    }

    // Redirect back to the main page with updated settings
    server.sendHeader("Location", "/");
    server.send(303); // Send See Other status code for redirect
  });

  server.on("/distance", HTTP_GET, []() {
    // Respond with the current distance value
    server.send(200, "text/plain", String(distanceCm));
  });
}
