#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>

// WiFi & MQTT setup
const char *ssid = "Luciano 2.4Ghz"; // Enter your WiFi name
const char *password = "1000162519";  // Enter WiFi password
const char *mqtt_broker = "192.168.4.61";
const int mqtt_port = 1883;
void callback(char *topic, byte *payload, unsigned int length);
WiFiClient espClient;
PubSubClient client(espClient);

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// topics
const char *humidity = "sensor/humidity";
const char *temperature = "sensor/temperature";
const char *heat = "controller/heat";
const char *freeze = "controller/freeze";
const char *humidify = "controller/humidify";
const char *dry = "controller/dry";
const char *sound = "controller/sound";

// pins
#define SENSOR 13
#define HOT 0
#define COLD 2
#define HUMID 4
#define DRY 15
#define SOUND 12

// sensor setup
DHT dht = DHT(SENSOR, DHT11);

void setup() {
  Serial.begin(115200);
//  connect to wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
     Serial.println("Connecting to wifi");
     delay(1000);
  }
  Serial.println("Connected to wifi");
//   connect to broker
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
  client.subscribe(heat);
  client.subscribe(freeze);
  client.subscribe(humidify);
  client.subscribe(dry);
  client.subscribe(sound);
  
//  setup lights
  pinMode(HOT, OUTPUT);
  pinMode(COLD, OUTPUT);
  pinMode(HUMID, OUTPUT);
  pinMode(DRY, OUTPUT);
  pinMode(SOUND, OUTPUT);
  digitalWrite(HOT, LOW);
  digitalWrite(COLD, LOW);
  digitalWrite(HUMID, LOW);
  digitalWrite(DRY, LOW);
  digitalWrite(SOUND, LOW);

//  setup lcd
  lcd.init();                // initialize the lcd
  lcd.backlight();           // Turn on backlight
  lcd.print("Waiting for input...");// Print a message to the LCD

//  setup dht
  dht.begin();
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
  
  if(strcmp(topic,heat) == 0) {
    if ((char) payload[0] == '0') {
      digitalWrite(HOT, LOW);
    }
    if ((char) payload[0] == '1') {
      digitalWrite(HOT, HIGH);
    }
  }
  if(strcmp(topic,freeze) == 0) {
    if ((char) payload[0] == '0') {
      digitalWrite(COLD, LOW);
    }
    if ((char) payload[0] == '1') {
      digitalWrite(COLD, HIGH);
    }
  }
  if(strcmp(topic,humidify) == 0) {
    if ((char) payload[0] == '0') {
      digitalWrite(HUMID, LOW);
    }
    if ((char) payload[0] == '1') {
      digitalWrite(HUMID, HIGH);
    }
  }
  if(strcmp(topic,dry) == 0) {
    if ((char) payload[0] == '0') {
      digitalWrite(DRY, LOW);
    }
    if ((char) payload[0] == '1') {
      digitalWrite(DRY, HIGH);
    }
  }
  if(strcmp(topic,sound) == 0) {
    if ((char) payload[0] == '0') {
      digitalWrite(SOUND, LOW);
    }
    if ((char) payload[0] == '1') {
      digitalWrite(SOUND, HIGH);
    }
  }
}

void loop() {
  client.loop();
//  print sensor data to lcd
  float h = dht.readHumidity();
  if (!isnan(h)) {
    lcd.setCursor(0,0);
    lcd.print("Humid.: ");
    lcd.print(h);
    lcd.print("%");
    Serial.print("Humid.: ");
    Serial.print(h);
    Serial.print("% | ");
    lcd.print("        ");
    char hString[6];
    dtostrf(h,2,2,hString);
    char msg[7];
    sprintf(msg,"%s%s","H",hString);
    client.publish(humidity, msg);
  }
  float t = dht.readTemperature(true);
  if (!isnan(t)) {
    lcd.setCursor(0,1);
    lcd.print("Temp.: ");
    lcd.print(t);
    lcd.print("F");
    Serial.print("Temp.: ");
    Serial.print(t);
    Serial.println("F");
    lcd.print("        ");
    char tString[6];
    dtostrf(t,2,2,tString);
    char msg[7];
    sprintf(msg,"%s%s","T",tString);
    client.publish(temperature, msg);
  }
//  check connection status
  while (!client.connected()) {
    Serial.println("Client disconnected... reconnecting");
    client.connect(String(WiFi.macAddress()).c_str());
  }
//  print state
  Serial.print("Client State: ");
  Serial.println(client.state());
  delay(2000);
}
