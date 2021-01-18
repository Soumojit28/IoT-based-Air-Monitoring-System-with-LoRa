/*
IoT Based Air Pollution Station LoRa Station
By- Soumojit Ash. soumojitash@gmail.com
*/

//Required Librarys
#include  //ESP32
#include  //SPI
#include  //LoRa
#include  //JSON
#include //DHT Sensor
#include //Light Sensor
#include //RGB LED
#include //ADC
#include

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 2 /* Time ESP32 will go to sleep (in seconds) */

#define DHTPIN 4 //pin of DHT sensor
#define DHTTYPE DHT11 //type of DHT sensor

#define NUM_LEDS 1//No of RGB LED
#define DATA_PIN 15// Pin for LED

CRGB leds[NUM_LEDS];
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;
Adafruit_ADS1115 ads;

int ledPower = 32;

int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;

float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;

int16_t adc0, adc1, adc2;
char buffer[256];
float h;// floats for humidity and temperature
float t;
float lux;
float air;
float co;
float dust;

void get_sensor()
{
h=dht.readHumidity();//read data from sensors
t=dht.readTemperature();
if (isnan(h) || isnan(t) ) {
Serial.println(F("Failed to read from DHT sensor!"));
leds[0] = CRGB::Red;
FastLED.show();
return;
}
lux = lightMeter.readLightLevel();
adc0 = ads.readADC_SingleEnded(0);
adc1 = ads.readADC_SingleEnded(1);
//float air_sensor=((adc0*0.1875)/1000)*(5/3.3);
air= map(adc0,0,26666,100,0);
float co_sensor_volt=((adc1*0.1875)/1000)*(5/3.3);
Serial.print(co_sensor_volt);
Serial.print("\t");
float co_rs=((5.0-co_sensor_volt)/co_sensor_volt);
float co_ratio=co_rs/7200;
Serial.print(co_ratio);
Serial.print("\t");
float co_x= 1538.46*co_ratio;
Serial.print(co_x);
Serial.print("\t");
float co_ppm=pow(co_x,-1.709);
// Serial.print(co_ppm);
// Serial.print("\t");
co=co_ppm*1000;

delay(1000);
digitalWrite(ledPower,LOW); // power on the LED
delayMicroseconds(samplingTime);
adc2 = ads.readADC_SingleEnded(2);
// read the dust value

delayMicroseconds(deltaTime);
digitalWrite(ledPower,HIGH); // turn the LED off
delayMicroseconds(sleepTime);

calcVoltage = ((adc2*0.1875)/1000)*(5/3.3);

// linear eqaution taken from http://www.howmuchsnow.com/arduino/airquality/
dustDensity = ((0.17 * calcVoltage) - 0.1)*1000;
if(dustDensity<0)
{
dustDensity=0;
}
}

void setup() {
Serial.begin(9600);
while (!Serial);
FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
leds[0] = CRGB::Yellow;
FastLED.show();
Wire.begin();
ads.begin();
dht.begin();
lightMeter.begin();
pinMode(ledPower,OUTPUT);
LoRa.setPins(5, 14, 2);
if (!LoRa.begin(866E6)) {
Serial.println("Starting LoRa failed!");
leds[0] = CRGB::Red;
FastLED.show();
while (1);
}
LoRa.setSpreadingFactor(12);
LoRa.setSignalBandwidth(125E3);
}

void loop() {

leds[0] = CRGB::Yellow;
FastLED.show();
get_sensor();

DynamicJsonDocument doc(192);
doc["SID"] = "TECHNA1";
doc["hum"] = h;
doc["temp"] = t;
doc["light"] = lux;
doc["co"] = co;
doc["air"] = air;
doc["pm10"] = dustDensity;
serializeJson(doc, Serial);
serializeJson(doc, buffer);

leds[0] = CRGB::Green;
FastLED.show();
LoRa.beginPacket();
LoRa.print(buffer);
LoRa.endPacket();
Serial.println("Sent");
leds[0] = CRGB::Blue;
FastLED.show();
LoRa.sleep();

delay(1000);
}
