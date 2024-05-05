#include <Arduino.h> // Arduino core library
#include <ESP8266WiFi.h> // ESP8266 WiFi library
#include <ESP8266WebServer.h> // ESP8266 WebServer library

ESP8266WebServer server(80); // Create a WebServer object listening on port 80

const char* ssidap = "Obstacle Detector WiFi"; // Set the access point SSID
const char* passap = ""; // Set the access point password

const int echoPin = 14; // Echo pin connected to GPIO14 (D5)
const int trigPin = 12; // Trigger pin connected to GPIO12 (D6)
const int buzzerPin = 13; // Buzzer connected to GPIO13 (D7)

// Define the speed of sound in cm/uS
#define SOUND_VELOCITY 0.034

long duration; //  Duration of the sound wave travel time
float distanceCm; // Calculated distance in cm
float updatedRangeValue = 5; // Default detection range

void setup() {
  Serial.begin(115200); // Start serial communication at 115200 baud rate

  WiFi.mode(WIFI_AP); // Set WiFi mode to access point
  WiFi.softAP(ssidap, passap); // Set SSID and password for the access point
  Serial.println("Connect to WiFi"); // Print message 
  Serial.println("WiFi name: Obstacle Detector WiFi"); // Print access point SSID
  Serial.println(WiFi.softAPIP()); // Print access point IP address
  
  createWebServer(); // Create web server routes
  server.begin(); // Start web server
  
  pinMode(echoPin, INPUT); // Set the echoPin as an Input
  pinMode(trigPin, OUTPUT); // Set the trigPin as an Output
  pinMode(buzzerPin, OUTPUT); // Set the buzzerPin as an Output
}

void loop() {
  server.handleClient(); // Handle web server requests
  
  digitalWrite(trigPin, LOW); // Clear the trigPin
  delayMicroseconds(2); // Delay for stability
  digitalWrite(trigPin, HIGH); // Set the trigPin on HIGH state for 10 microseconds
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW); // Reset the trigPin
  
  duration = pulseIn(echoPin, HIGH); // Read the echoPin to get the sound wave travel time
  
  distanceCm = duration * SOUND_VELOCITY / 2; // Calculate the distance
  
  if (distanceCm < updatedRangeValue) { // Check if the distance is less than the updated range value
    Serial.println("Item Nearby!"); // Print message
    digitalWrite(buzzerPin, HIGH); // Turn on the buzzer
  } else {
    digitalWrite(buzzerPin, LOW); // Turn off the buzzer
  }

  Serial.print("Distance: "); // Print message
  Serial.print(distanceCm); // Print distance
  Serial.println(" cm"); // Print units
  Serial.println(""); // Print blank line

  delay(1000); // Delay to avoid excessive triggering and noise
}

void createWebServer() { // Declare function
  server.on("/", HTTP_GET, []() { // Define route for root page
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
   
   server.send(200, "text/html", content); // Send HTTP response with status code 200, content type text/html, and the provided content

server.on("/update", HTTP_GET, []() { // Define route "/update" for HTTP GET requests with a lambda function
    String range = server.arg("range"); // Get the value of the "range" parameter from the request URL

    // Convert range to float and update global variable if needed
    float newRange = range.toFloat(); // Convert string to float
    if (newRange > 0 && newRange <= 400) { // Check if the new range value is within the valid range
      updatedRangeValue = newRange; // Update the global variable with the new range value
      Serial.print("Updated Detection Range: "); // Print message to Serial Monitor
      Serial.println(updatedRangeValue); // Print the updated detection range value
    } else {
      Serial.println("Invalid range value. Please enter a value between 1 and 400."); // Print error message to Serial Monitor
    }

    // Redirect back to the main page with updated settings
    server.sendHeader("Location", "/"); // Set the location header to the root ("/") URL
    server.send(303); // Send HTTP response with status code 303 (See Other) for redirect
  });

  server.on("/distance", HTTP_GET, []() { // Define route "/distance" for HTTP GET requests with a lambda function
    // Respond with the current distance value
    server.send(200, "text/plain", String(distanceCm)); // Send HTTP response with status code 200, content type text/plain, and the current distance value
  });
}
