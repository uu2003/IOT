#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Adafruit_NeoPixel.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>



// Add GPIO pins used to connect devices
const int buttonPin = 4;  // the pin that the pushbutton is attached to

// MAC address of the receiver ESP8266
uint8_t broadcastAddress[] = {0x08, 0x3A, 0x8D, 0xE3, 0xB9, 0x4B}; //0x08, 0x3A, 0x8D, 0xE3, 0xB9, 0x4B, 0x08, 0x3A, 0x8D, 0xEE, 0xDB, 0x22
unsigned long start_time = 0;
unsigned long end_time = 0;
unsigned long duration = 0;
unsigned long duration_sec = 0;
unsigned long duration_min = 0;
unsigned long duration_totale = 0;
int buttonPushCounter = 0;  // counter for the number of button presses
int buttonState = 1;        // current state of the button
int lastButtonState = 1;    // previous state of the button


unsigned long previousTime=0;
String chair = "A";
String chair_status;
int occupancy = 1;
// variables to hold data
StaticJsonDocument<150> jsonDoc;
JsonObject payload = jsonDoc.to<JsonObject>();
JsonObject status = payload.createNestedObject("d");
static char msg[150];

// callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t status) {
 Serial.print(F("\r\n Master packet sent:\t"));
 Serial.println(status == 0 ? "Delivery Success" : "Delivery Fail");
}
void setup() {

 // Init Serial Monitor
 Serial.begin(115200);
 // Set device as a Wi-Fi Station
 WiFi.mode(WIFI_STA);
 // initialize the button pin as a input:
 pinMode(buttonPin, INPUT);
 // Init ESP-NOW
 if (esp_now_init() != 0) {
   Serial.println(F("Error initializing ESP-NOW"));
   return;
 }
 Serial.print(F("\nReciever initialized : "));
 Serial.println(WiFi.macAddress());
 
 // Define Send function
 esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
 esp_now_register_send_cb(OnDataSent);
 // Register peer
 esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

}
void loop() {
  // read the pushbutton input pin:
  buttonState = digitalRead(buttonPin);

  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    // if the state has changed, increment the counter
    if (buttonState == HIGH) {
      // if the current state is HIGH then the button went from off to on:
      buttonPushCounter++;
      start_time = millis();
      chair_status  = "Occupied";
      occupancy = 50;
      Serial.println("on");
      Serial.print("number of button pushes: ");
      Serial.println(buttonPushCounter);
    } else {
      // if the current state is LOW then the button went from on to off:
      end_time = millis();
      chair_status = "Unoccupied";
      occupancy = 1;
      Serial.println("off");
      Serial.print("Time spent : ");
      duration = end_time - start_time;
      duration_sec = duration / 1000;
      duration_min = duration_sec / 60;
      if (duration_min > 1){
        Serial.print(duration_min);
        Serial.print(" minutes ");
      }
      if (duration_sec > 1){
        Serial.print(duration_sec);
        Serial.print(" seconds ");
        Serial.print(duration % 1000);
        Serial.println(" milliseconds ");}
      else {
        Serial.print(duration);
        Serial.println(" milliseconds ");}
      duration_totale += duration_min*60 + duration_sec;
      Serial.print(duration_totale);
      if (duration_totale > 60){
        duration_totale += duration_totale / 60;
        Serial.print(duration_totale);
      }
    }
    // Delay a little bit to avoid bouncing
    delay(50);
  }
  // save the current state as the last state, for next time through the loop
  lastButtonState = buttonState;

  status["chaira"] = chair;
  status["statusa"] = chair_status;
  status["timespenta"] = duration_totale;
  status["occupieda"] = buttonPushCounter;
  //status["occupancya"] = occupancy;
  serializeJson(jsonDoc, msg, 150);
  Serial.println(msg);



  if((millis() -previousTime)>1000){
     // Send message via ESP-NOW
     uint8_t result = esp_now_send(broadcastAddress, (uint8_t *) msg, sizeof(msg));
     if (result == 0) {
       Serial.println(F("Sent with success"));
     }
     else {
       Serial.println(F("Error sending the data"));
     }
     previousTime=millis();
  }
  

}