#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Wokwi-GUEST";
const char* password = ""; 

#define mqttServer "broker.emqx.io"
#define mqttPort  1883

WiFiClient espClient;
PubSubClient mqttClient(espClient);

const int ledPin = 4;
const int buttonPin = 32;

bool buttonState = false;
bool ledState = false;

void reconnect() {
    while (!mqttClient.connected()) {
      Serial.print("Connecting to MQTT...");
  
      String clientId = "ESP32_" + String(random(1000, 9999));
      if (mqttClient.connect(clientId.c_str())) {
        Serial.println("Connected!");
        mqttClient.subscribe("esp32/led"); 
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
    ledState = true;
  } 
  else if (mes == "off") {
    Serial.println("Turning LED OFF");
    ledState = false;
  }

  digitalWrite(ledPin, !ledState);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

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
  mqttClient.loop();

  bool currentButtonState = digitalRead(buttonPin) == LOW;
  
  if (currentButtonState && !buttonState) { 
    ledState = !ledState;
    String message;

    message = ledState ? "on" : "off"; 
    digitalWrite(ledPin, !ledState);
    delay(500);

    mqttClient.publish("esp32/led", message.c_str());
    Serial.print("Sent to MQTT: ");
    Serial.println(message);
  }

  buttonState = currentButtonState;
}