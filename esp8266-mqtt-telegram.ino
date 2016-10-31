// Library for MQTT
#include <PubSubClient.h>
// Library for wifi connection
#include <ESP8266WiFi.h>
// Library for RF communication
#include <RCSwitch.h>

// This code is sent by the sensor RF when open is detected
const char* SENSOR_OPEN  = "1010";
// This code is sent by the sensor RF when close is detected
const char* SENSOR_CLOSE = "1110";
// Wifi SSID
const char* ssid = "XXX";
// Wifi password
const char* password = "XXX";
// IP MQTT server 
const char* mqtt_server = "XXX";
// This pin turn on when sensor is opened
int ledPin = 13; // GPIO13
// This pin receive the signal from RF receiver
int rxPin = 5;
// Flag sensor open
int isOpen = 0;
// Initialize the RF receiver
RCSwitch mySwitch = RCSwitch();
// Initialize the client library
WiFiClient client;
// MQTT client
PubSubClient mqttClient(client);

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str(), "nodemcu", "sarnita")) {
      Serial.println("connected");
      mqttClient.subscribe("sensor1");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  mySwitch.enableReceive(rxPin);
  
  WiFi.begin(ssid, password);
  Serial.println("");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    Serial.print(WiFi.status());
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  mqttClient.setServer(mqtt_server, 1883);
}

void loop() {
  int rfValue = -1;
  char *b = NULL;
  if (mySwitch.available()) {
    b = dec2binWzerofill(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength());
    mySwitch.resetAvailable();
  }

  if (!mqttClient.connected()) {
    reconnect();
  }
  
  mqttClient.loop();
  
  if (b != NULL) {
    if (!memcmp(b+20,SENSOR_OPEN,4) && !isOpen) {
      digitalWrite(ledPin, HIGH);
      mqttClient.publish("sensor1", "OPEN");
      Serial.print("OPEN");
      isOpen = 1;
    } else if (!memcmp(b+20,SENSOR_CLOSE,4) && isOpen) {
      digitalWrite(ledPin, LOW);
      mqttClient.publish("sensor1", "CLOSE");
      Serial.print("CLOSE");
      isOpen = 0;
    }  
  }
}

