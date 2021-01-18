/*
IoT Based Air Pollution Station LoRa Gateway
By- Soumojit Ash. soumojitash@gmail.com
*/
 

//Required Librarys
#include <WiFi.h> //ESP32
#include <PubSubClient.h> //MQTT
#include <SPI.h> //SPI
#include <LoRa.h> //LoRa
#include <ArduinoJson.h> //JSON
#include <Adafruit_GFX.h> //OLED
#include <Adafruit_SSD1306.h>//OLED

//Defines
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define MQTT_MAX_PACKET_SIZE 256
#define OLED_RESET 4// Reset pin # (or -1 if sharing Arduino reset pin)

//Varribles
const char* ssid = “Soumojit”; //SSID
const char* password = “********”; //Password
const char* mqtt_server = “makathon.technatorium.com”; //MQTT server address

String full_string;
String final_string;
char incoming;
int rssi;
float snr;
char char_array[256];

//Objects
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
WiFiClient espClient;
PubSubClient client(espClient);

//Function to display in OLED
void oled_display(char text[256]) {
display.clearDisplay();
display.setTextSize(1);
display.setTextColor(WHITE);
display.setCursor(0, 0);
display.print(text);
display.display();
}

void setup_wifi() { //Wifi Setup

delay(10);// We start by connecting to a WiFi network
Serial.println();
Serial.print(“Connecting to “);
Serial.println(ssid);
oled_display(“Connecting…..”);
WiFi.begin(ssid, password);

while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(“.”);
}

randomSeed(micros());

Serial.println(“”);
Serial.println(“WiFi connected”);
oled_display(“Wifi Connected!!!”);
Serial.println(“IP address: “);
Serial.println(WiFi.localIP());
}

//Reconnect to MQTT
void reconnect() {
// Loop until we’re reconnected
while (!client.connected()) {
Serial.print(“Attempting MQTT connection…”);
oled_display(“Attempting MQTT connection…”);
// Create a random client ID
String clientId = “ESP8266Client-“;
clientId += String(random(0xffff), HEX);
// Attempt to connect
if (client.connect(clientId.c_str())) {
Serial.println(“connected”);
oled_display(“MQTT Connected!!!”);
// … and resubscribe
} else {
Serial.print(“failed, rc=”);
oled_display(“Failed MQTT Connection!”);
Serial.print(client.state());
Serial.println(” try again in 5 seconds”);
// Wait 5 seconds before retrying
delay(5000);
}
}
}

void setup() {
Serial.begin(9600);
display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
oled_display(“LoRa Gateway \nInitializing….”);
delay(1000);
setup_wifi();

client.setServer(mqtt_server, 1883);//MQTT server and port

LoRa.setPins(5, 14, 2);
if (!LoRa.begin(866E6)) {
Serial.println(“Starting LoRa failed!”);
oled_display(“Starting LoRa failed!”);
while (1);
}
LoRa.setSpreadingFactor(12);
LoRa.setSignalBandwidth(125E3);

LoRa.disableInvertIQ();
}
void loop() {

if (!client.connected()) {
reconnect();
}

client.loop();//Keep MQTT connected

int packetSize = LoRa.parsePacket();
full_string = “”;
if (packetSize) { // received a packet
Serial.print(“Received packet ‘”);
oled_display(“Packet Received”);
while (LoRa.available()) {
incoming = (char)LoRa.read();
full_string += incoming;
}
rssi = LoRa.packetRssi();
Serial.print(full_string);
Serial.print(“‘ with RSSI “);
Serial.println(rssi);// print RSSI of packet

DynamicJsonDocument doc(192); //Allocating space for JSON document
deserializeJson(doc, full_string);
doc[“rssi”] = rssi; //Adding value of RSSI
serializeJson(doc, char_array);
serializeJson(doc, Serial);

oled_display(char_array);

bool responce=client.publish(“Gateway/CH1”,char_array);
Serial.println(responce);
}
}
