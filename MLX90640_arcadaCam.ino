#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_MLX90640.h>

// Wi-Fi connection information
const char* ssid = "MedAI-146";
const char* password = "ITD#R146";

// Create WebServer and MLX90640 objects
WebServer server(80);
Adafruit_MLX90640 mlx;

// Thermal image dimensions
const int WIDTH = 32;
const int HEIGHT = 24;
float frame[WIDTH * HEIGHT];

// Function to connect to Wi-Fi
void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - startTime > 20000) { // Wait for 20 seconds
      Serial.println("Failed to connect to WiFi");
      return;
    }
  }
  Serial.println("Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Function to get thermal data as a JSON string
String getThermalData() {
  mlx.getFrame(frame);
  String data = "[";
  for (int i = 0; i < WIDTH * HEIGHT; i++) {
    data += String(frame[i]);
    if (i < WIDTH * HEIGHT - 1) {
      data += ",";
    }
  }
  data += "]";
  return data;
}

void setup() {
  Serial.begin(115200);
  
  // Connect to Wi-Fi
  connectToWiFi();

  // Initialize MLX90640
  if (!mlx.begin()) {
    Serial.println("Failed to find MLX90640 sensor");
    while (1);
  }
  
  // Configure MLX90640 sensor
  mlx.setMode(MLX90640_CHESS);
  mlx.setResolution(MLX90640_ADC_16BIT);

  // Configure web server
  server.on("/", HTTP_GET, []() {
    String html = "<html><body><pre id='data'></pre><script>function updateData() {fetch('/thermal').then(response => response.json()).then(data => {document.getElementById('data').textContent = JSON.stringify(data, null, 2);});}function saveData() {fetch('/save');}setInterval(updateData, 500);</script></body></html>";
    server.send(200, "text/html", html);
  });

  server.on("/thermal", HTTP_GET, []() {
    String data = getThermalData();
    server.send(200, "application/json", data);
  });

  server.on("/save", HTTP_GET, []() {
    String data = getThermalData();
    server.send(200, "application/json", data);
  });

  server.begin();
}

void loop() {
  server.handleClient();
}
