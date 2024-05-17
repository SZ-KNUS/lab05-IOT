#include <ArduinoJson.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

// Includes AWS certificates, endpoint values, WIFI password
#include "secret.h"

#define AWS_IOT_PUBLISH_TOPIC   "testtopic/KNUS-12-06/1"
#define AWS_IOT_SUBSCRIBE_TOPIC "testtopic/KNUS-12-06/+"
//Pin Id
const int oneWireBus = 12;
String temperatureInC;

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

void connectToAWS() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }

    net.setCACert(AWS_CA_CERTIFICATE);
    net.setCertificate(AWS_CERTIFICATE);
    net.setPrivateKey(AWS_PRIVATE_CERTIFICATE);

    client.setServer(AWS_IOT_ENDPOINT, 8883);
    client.setCallback(formInfoMessages);

    while (!client.connect(THINGNAME)) {
        Serial.print(".");
        delay(100);
    }

    client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

    Serial.println("Connected to AWS IoT");
}

void publishMessage() {
    StaticJsonDocument<200> doc;
    char jsonBuffer[512];

    doc["temperature"] = temperatureInC;

    serializeJson(doc, jsonBuffer);
    client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void formInfoMessages(char* topic, byte* payload, unsigned int length) {
    Serial.print("The message from the topic: ");
    Serial.println(topic);

    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);

    const char* message = doc["message"];
    Serial.println(message);
}

void setup() {
    Serial.begin(115200);
    sensors.begin();

    Serial.print("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }

    Serial.println("Connected!");
    connectToAWS();
}

void loop() {
    sensors.requestTemperatures();
    float temperatureC = sensors.getTempCByIndex(0);
    temperatureInC = String(temperatureC);

    Serial.print(temperatureC);
    Serial.println("ºC");

    delay(5000);
    publishMessage();
    client.loop();
    delay(5000);
}
