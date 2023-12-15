#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Adafruit_NeoPixel.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

// MAC address of the sender ESP8266
uint8_t broadcastAddress[] = {0x08, 0x3A, 0x8D, 0xEE, 0xDB, 0x22}; //08:3a:8d:e3:ad:bc


// Add WiFi connection information
//char ssid[] = "RALT_RIoT";  // your network SSID (name)
//char pass[] = "raltriot";  // your network password
char ssid[] = "iPhone_CH";  // your network SSID (name)
char pass[] = "tapelemotdepasse";  // your network password

char ssid[] = "";  // your network SSID (name)
char pass[] = "";  // your network password

// MQTT connection details
#define MQTT_HOST "test.mosquitto.org"
#define MQTT_PORT 1883
#define MQTT_DEVICEID "d:hwu.esp8266.H00448888"
#define MQTT_USER "" // no need for authentication, for now
#define MQTT_TOKEN "" // no need for authentication, for now
#define MQTT_TOPIC "H00448888/evt/status/fmt/json"
#define MQTT_TOPIC_DISPLAY "H00448888/cmd/display/fmt/json"
#define MQTT_TOPIC_INTERVAL "H00448888/cmd/interval/fmt/json"
#define MQTT_TOPIC_OCCUPANCY "H00448888/cmd/occupancy/fmt/json"
// MQTT objects
void callback(char* topic, byte* payload, unsigned int length);
WiFiClient wifiClient;
PubSubClient mqtt(MQTT_HOST, MQTT_PORT, callback, wifiClient);

// variables to hold data
StaticJsonDocument<150> jsonDoc;
JsonObject payload = jsonDoc.to<JsonObject>();
JsonObject status = payload.createNestedObject("d");
static char msg[150];
int l;

// callback when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  //memcpy(&myData, incomingData, sizeof(myData));
  //Serial.print("Message Received : ");
  //Serial.println((char *)incomingData);
  strcpy(msg,(char *)incomingData);
  Serial.print("Message Confirmed : ");
  Serial.println(msg);
}

void callback(char* topic, byte* payload, unsigned int length) {
  DynamicJsonDocument doc(1024);
  // handle message arrived
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] : ");

  // Test which topic is arrived
  if((String)topic == "H00448888/cmd/interval/fmt/json"){
    payload[length] = 0; 
    Serial.println((char *)payload);
    deserializeJson(doc, (char *)payload);
    int interval = doc["Interval"];  
    l = interval;  
  }
 
}

void setup() {
 // Initialize Serial Monitor
 Serial.begin(115200);
 // Set device as a Wi-Fi Station
 WiFi.mode(WIFI_STA);
 WiFi.begin(ssid, pass);
 while (WiFi.status() != WL_CONNECTED) {
   delay(500);
   Serial.print(".");
 }
  Serial.println("");
  Serial.println("WiFi Connected");

 // Connect to MQTT broker
  if (mqtt.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN)) {
    Serial.println("MQTT Connected");
    mqtt.subscribe(MQTT_TOPIC_DISPLAY);
    mqtt.subscribe(MQTT_TOPIC_INTERVAL);
    mqtt.subscribe(MQTT_TOPIC_OCCUPANCY);

  } else {
    Serial.println("MQTT Failed to connect!");
    ESP.reset();
  }
 // Init ESP-NOW
 if (esp_now_init() != 0) {
   Serial.println("Error initializing ESP-NOW");
   return;
 }
 Serial.print(F("\nReceiver initialized : "));
 Serial.println(WiFi.macAddress());
 // Define receive function
 esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
 esp_now_register_recv_cb(OnDataRecv);
}
void loop() {
  mqtt.loop();
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN)) {
      Serial.println("MQTT Connected");
      mqtt.subscribe(MQTT_TOPIC_DISPLAY);
      mqtt.subscribe(MQTT_TOPIC_INTERVAL);
      mqtt.loop();
    } else {
      Serial.println("MQTT Failed to connect!");
      delay(5000);
    }
  }

  if (l == 1){
    if (!mqtt.publish(MQTT_TOPIC, msg)) {
      Serial.println("MQTT Publish failed");
    }

  }
  
  
}