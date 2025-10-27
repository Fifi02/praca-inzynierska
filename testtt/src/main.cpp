#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// ---------- WiFi credentials ----------
const char* WIFI_SSID = "Biustonosze_sciagac_prosze";
const char* WIFI_PASSWORD = "Sexyhelena14";

// ---------- HiveMQ Cloud credentials ----------
const char* MQTT_BROKER = "f532a99e26a04b1c9d148e8359c5d3e5.s1.eu.hivemq.cloud"; // e.g., mycluster.s1.eu.hivemq.cloud
const int   MQTT_PORT = 8883;
const char* MQTT_USER = "ESP32_test";
const char* MQTT_PASS = "2M7VhEnJYs6HY!n";
const char* MQTT_CLIENT_ID = "ESP32Client-01";
const char* PUBLISH_TOPIC = "test/esp32/pub";
const char* SUBSCRIBE_TOPIC = "esp32/test/sub";
const char* POWITANIE = "Moje uszanowanko. Miłego dnia i smacznej kawusi :-)";
WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");

  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  mqttClient.publish(PUBLISH_TOPIC,POWITANIE);
}


void setupWiFi() {
  Serial.print("Connecting to WiFi: ");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    // Generate a unique client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0, 1000));

    Serial.print("Connecting to HiveMQ Cloud with client ID: ");
    Serial.println(clientId);

    if (mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
      Serial.println("✅ MQTT connected");

      // Subscribe to the topic
      mqttClient.subscribe(SUBSCRIBE_TOPIC);
      Serial.print("Subscribed to: ");
      Serial.println(SUBSCRIBE_TOPIC);

      // Publish an initial message
      mqttClient.publish(PUBLISH_TOPIC, "ESP32 connected (test)");
    } else {
      Serial.print("❌ MQTT connection failed, state=");
      Serial.println(mqttClient.state());
      delay(5000);
    }
  }
}


void setup() {
  Serial.begin(115200);
  setupWiFi();

  // 👇 use insecure TLS for testing
  wifiClient.setInsecure();

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback); // set callback for incoming messages
}

void loop() {
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  String msg = String("Temperature: ") + random(20, 30) + "°C";
  mqttClient.publish(PUBLISH_TOPIC, msg.c_str());
  Serial.println("Published: " + msg);
  delay(5000);
}