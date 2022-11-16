//set dimmer
#include <RBDdimmer.h>
const int zeroCrossPin = 35;
const int acdPin = 21;
int MIN_POWER = 0;
int MAX_POWER = 100;
int power_min = 25;
int power_max = 100;
int power_moderate = 50;
int power_off =0;
dimmerLamp acd(acdPin, zeroCrossPin);

//set tft
#include "SPI.h"
#include "TFT_eSPI.h"
#include <SoftwareSerial.h>

//set pms5003
SoftwareSerial mySerial(32, 33);  // RX, TX
unsigned int pm1 = 0;
unsigned int pm2_5 = 0;
unsigned int pm10 = 0;

//set dht
#include "DHT.h"
#define DHTPIN 25
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//set TFT
#define TFT_GREY 0x7BEF
TFT_eSPI myGLCD = TFT_eSPI();  // Invoke custom library
unsigned long runTime = 0;


void setup() {
  Serial.println(F("DHTxx test!"));
  dht.begin();
  Serial.begin(921600);
  acd.begin(NORMAL_MODE, ON);
  while (!Serial)
    ;
  mySerial.begin(921600);
  // Setup the LCD
  myGLCD.init();
  myGLCD.setRotation(1);
}


void loop() {
  //dht11 sensor
  delay(2000);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));


  //PMS5003
  int index = 0;
  char value;
  char previousValue;

  while (mySerial.available()) {
    value = mySerial.read();
    if ((index == 0 && value != 0x42) || (index == 1 && value != 0x4d)) {
      Serial.println("Cannot find the data header.");
      break;
    }

    if (index == 4 || index == 6 || index == 8 || index == 10 || index == 12 || index == 14) {
      previousValue = value;
    } else if (index == 5) {
      pm1 = 256 * previousValue + value;
      Serial.print("{ ");
      Serial.print("\"pm1\": ");
      Serial.print(pm1);
      Serial.print(" ug/m3");
      Serial.print(", ");
    } else if (index == 7) {
      pm2_5 = 256 * previousValue + value;
      Serial.print("\"pm2_5\": ");
      Serial.print(pm2_5);
      Serial.print(" ug/m3");
      Serial.print(", ");
    } else if (index == 9) {
      pm10 = 256 * previousValue + value;
      Serial.print("\"pm10\": ");
      Serial.print(pm10);
      Serial.print(" ug/m3");
    } else if (index > 15) {
      break;
    }
    index++;
  }
  while (mySerial.available()) mySerial.read();
  Serial.println(" }");
  delay(1000);

  //show data on tft
  myGLCD.fillScreen(TFT_BLACK);
  myGLCD.setTextColor(TFT_WHITE);

  // Circle
  myGLCD.fillCircle(80, 90, 70, TFT_GREEN);
  myGLCD.fillCircle(260, 70, 50, TFT_SKYBLUE);
  myGLCD.fillCircle(200, 170, 50, TFT_ORANGE);


  //PM2.5
  myGLCD.drawString("PM 2.5", 60, 40, 2);
  myGLCD.drawNumber(pm2_5, 65, 73, 4);
  myGLCD.drawString(" ug./m3", 60, 123, 2);


  //Humid
  myGLCD.drawString("Humid", 245, 35, 2);
  myGLCD.drawString(String(h), 230, 65, 4);
  myGLCD.drawString(" %", 250, 95, 2);

  //Temp
  myGLCD.drawString("Temp", 185, 130, 2);
  myGLCD.drawString(String(t), 170, 160, 4);
  myGLCD.drawString("celsius", 180, 190, 2);
  delay(1000);

  //set fan dimmer

  if (pm2_5 == 0) {
    acd.setPower(power_off);
    Serial.print("FanValue -> ");
    Serial.print(acd.getPower());
    Serial.println("%");
    delay(1000);
  }
  else if (pm2_5 <= 12) {
    acd.setPower(power_min);
    Serial.print("FanValue -> ");
    Serial.print(acd.getPower());
    Serial.println("%");
    delay(1000);
  }
  else if (pm2_5 <= 35) {
    acd.setPower(power_moderate);
    Serial.print("FanValue -> ");
    Serial.print(acd.getPower());
    Serial.println("%");
    delay(1000);
  }
  else {
    acd.setPower(power_max);
    Serial.print("FanValue -> ");
    Serial.print(acd.getPower());
    Serial.println("%");
    delay(1000);
  }
}