#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Wokwi-GUEST";
const char* password = ""; 

#define mqttServer ""
#define mqttPort  1883

WiFiClient espClient;
PubSubClient mqttClient(espClient);

const int ledPin = 4;
const int buttonPin = 32;

bool buttonState = false;  // สถานะปุ่มก่อนหน้า
bool ledState = false;     // สถานะ LED

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqttClient.connect("ESP32_Client")) {
      Serial.println("Connected!");
      mqttClient.subscribe("esp32/led");  // Subscribe หัวข้อที่ต้องการ
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void callback(char *topic, byte *payload, unsigned int length) {
  String mes = "";
  for (int i = 0; i < length; i++) {
    mes += (char)payload[i];
  }
  
  Serial.print("Received Message: ");
  Serial.println(mes);

  if (mes == "on") {
    Serial.println("Turning LED ON");
    digitalWrite(ledPin, HIGH);
    ledState = true;
  } 
  else if (mes == "off") {
    Serial.println("Turning LED OFF");
    digitalWrite(ledPin, LOW);
    ledState = false;
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); // ตั้งค่า INPUT_PULLUP (ปุ่มกดใช้ LOW เป็นกด)

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);

  reconnect();
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop(); // Keep MQTT connection alive

  bool currentButtonState = digitalRead(buttonPin) == LOW; // ปุ่มกดเป็น LOW
  
  if (currentButtonState && !buttonState) {  // ตรวจจับการเปลี่ยนสถานะปุ่ม (Press)
    ledState = !ledState; // Toggle LED state
    String message;

    if (ledState) {
      message = "on";
      delay(500);
    } else if(!ledState) {
      message = "off";
      delay(500);
    }
    
    mqttClient.publish("esp32/led", message.c_str()); // ส่งค่าไปยัง MQTTX
    Serial.print("Sent to MQTT: ");
    Serial.println(message);
  }

  buttonState = currentButtonState; // อัปเดตสถานะปุ่ม
}
