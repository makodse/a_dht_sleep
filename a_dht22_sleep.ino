/*a_dht22_sleep report dht temperature from sleep state. /makodse@gmail.com*/
#include "ota_secret.h"
#include <ESP8266WiFi.h>
//#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <stdlib.h>
#include "DHT.h"
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#define durationSleep  1200             // seconds

/*your wifi is setup in ota_secret.h, this is only loading it into the var*/
const char* ssid = STASSID;
const char* password = STAPSK;

 /*your mqtt server*/
const char* mqtt_server = "192.168.3.220";
/*mqtt*/
WiFiClient espClient;
PubSubClient client(espClient);

/*dht22*/
uint8_t DHTPin = D4;
DHT dht(DHTPin, DHTTYPE);

char res[12]; // used to process result

/*callback from mqtt example - not used but might be to get topic before decision. */
void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
 
  Serial.println();
  Serial.println("-----------------------");
 
}
/*reconnect from mqtt example */
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("vaxthus")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("vaxthus/in");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void setup() {
  /*serial*/
  Serial.begin(115200);
  Serial.println("Booting");
  /*wifi and OTA, NIH (from OTA example)*/
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
     ESP.deepSleep(durationSleep * 1000000);
    ESP.restart();
  }
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  /*pinmode settings */
  pinMode(DHTPin, INPUT);
  dht.begin(); 
  /*mqtt*/
  client.setServer(mqtt_server, 1883);
  //client.publish("vaxthus/t", "test");
  //client.subscribe("esp/test");
  /*added some pauses after test, the dht 22 need some rest before read?*/
  delay(1000);
   if (!client.connected()) {
    reconnect();
  }
  delay(1000);
  /*might not be needed*/
  client.loop();
  float temperatur, fuktighet;
  client.publish("vaxthus/t", "test");
  //client.subscribe("esp/test");
  temperatur= dht.readTemperature();
  fuktighet= dht.readHumidity();
  //Serial.println(temperatur);
  //no decimal reported, adjust it in dtostrf()
  dtostrf(temperatur, 0, 0, res);
  client.publish("vaxthus/temp", res);
  dtostrf(fuktighet, 0, 0, res);
  client.publish("vaxthus/fukt", res);
  client.loop();
  //pause to give the esp some time before sleep.... optimize this later what works and make it go to sleep faster.
  delay(1000);
  ESP.deepSleep(durationSleep * 1000000);
}


void loop() {
}
