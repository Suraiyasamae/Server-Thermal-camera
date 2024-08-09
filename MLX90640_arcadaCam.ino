#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_MLX90640.h>

// ข้อมูลการเชื่อมต่อ Wi-Fi
const char* ssid = "MedAI-146";
const char* password = "ITD#R146";

// สร้างอ็อบเจ็กต์ WebServer และ MLX90640
WebServer server(80);
Adafruit_MLX90640 mlx;

// ขนาดของภาพความร้อน
const int WIDTH = 32;
const int HEIGHT = 24;
float frame[WIDTH * HEIGHT];

// ฟังก์ชันในการเชื่อมต่อ Wi-Fi
void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - startTime > 20000) { // ลองรอ 20 วินาที
      Serial.println("Failed to connect to WiFi");
      return;
    }
  }
  Serial.println("Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// ฟังก์ชันในการส่งข้อมูลความร้อนเป็นข้อความ JSON
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
  
  // เชื่อมต่อ Wi-Fi
  connectToWiFi();

  // เริ่มต้น MLX90640
  if (!mlx.begin()) {
    Serial.println("Failed to find MLX90640 sensor");
    while (1);
  }
  
  // ตั้งค่าเซ็นเซอร์ MLX90640
  mlx.setMode(MLX90640_CHESS);
  mlx.setResolution(MLX90640_ADC_16BIT);

  // กำหนดการทำงานของเว็บเซิร์ฟเวอร์
  server.on("/", HTTP_GET, []() {
    String html = "<html><body><h1></h1><pre id='data'></pre><script>function updateData() {fetch('/thermal').then(response => response.json()).then(data => {document.getElementById('data').textContent = JSON.stringify(data, null, 2);});}setInterval(updateData, 500);</script></body></html>";
    server.send(200, "text/html", html);
  });

  server.on("/thermal", HTTP_GET, []() {
    String data = getThermalData();
    server.send(200, "application/json", data);
  });

  server.begin();
}

void loop() {
  server.handleClient();
}
