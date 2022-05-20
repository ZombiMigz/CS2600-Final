#include <WiFi.h>
#include <PubSubClient.h>

// WiFi
const char *ssid = "Luciano 2.4Ghz"; // Enter your WiFi name
const char *password = "1000162519";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "192.168.4.61";
const char *topic = "esp32/test";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

int on = 0;

#define LED 15

void setup() {
 // Set software serial baud to 115200;
 Serial.begin(115200);
 // connecting to a WiFi network
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.println("Connecting to WiFi..");
 }
 Serial.println("Connected to the WiFi network");
 //connecting to a mqtt broker
 client.setServer(mqtt_broker, mqtt_port);
 client.setCallback(callback);
 while (!client.connected()) {
     String client_id = "esp32-client-";
     client_id += String(WiFi.macAddress());
     Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
     if (client.connect(client_id.c_str())) {
         Serial.println("Public emqx mqtt broker connected");
     } else {
         Serial.print("failed with state ");
         Serial.print(client.state());
         delay(2000);
     }
 }
 // publish and subscribe
 client.publish(topic, "Hi EMQX I'm ESP32 ^^");
 client.subscribe(topic);

// setup led
  pinMode(LED,OUTPUT);
  digitalWrite(LED,LOW);
}

void callback(char *topic, byte *payload, unsigned int length) {
 Serial.print("Message arrived in topic: ");
 Serial.println(topic);
 Serial.print("Message:");
 for (int i = 0; i < length; i++) {
     Serial.print((char) payload[i]);
 }
 Serial.println();
 Serial.println("-----------------------");

 if ((char) payload[0] == '1') {
  on = 1;
  digitalWrite(LED,HIGH);
 }
 if ((char) payload[0] == '2') {
  on = 2;
  digitalWrite(LED,LOW);
 }
}

void loop() {
 client.loop();
 if (on == 1) {
  client.publish(topic,"Light is on");
 } else {
  client.publish(topic,"Light is off");
 }
 delay(2000);
}
