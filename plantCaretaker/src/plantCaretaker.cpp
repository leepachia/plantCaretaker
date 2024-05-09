/* 
 * Project: Plant Caretaker
 * Authors: Pachia Lee and Jake Robbins
 * Date: 
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#define TOKEN "BBUS-2mjQJfs1kFlFgeWbcJXfKzGjY1wSEY"
#include "Ubidots.h"
#include <string>
#include <iostream>
using namespace std;

const int SOIL_SENSOR = A1;
const int LIGHT_SENSOR = A5;
const int MOTOR = D0;

Ubidots ubidots(TOKEN, UBI_HTTP); 

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Run the application and system concurrently in separate threads
SYSTEM_THREAD(ENABLED);

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
SerialLogHandler logHandler(LOG_LEVEL_INFO);

double soilMoistureLevel;
double lightIntensityFrequency;
int drynessThreshold = 1100;
time64_t lastWatered = 0;
bool wasWatered = false;

int waterPlant (String argument) {
  //turn the pin on, then off or something
  //like that.
  digitalWrite(MOTOR, HIGH);
  delay(1500);
  digitalWrite(MOTOR, LOW);
  lastWatered = millis();
  Particle.publish("plantWatered");
  wasWatered = true;
  return 1; //or something
}

int updateDrynessThreshold(String argument) {
  int newThreshold = atoi(argument);
  Serial.println(argument);
  double toRawNumber = ((double)newThreshold/100) * 1400;
  drynessThreshold = (int) toRawNumber + 1050;
  return drynessThreshold;
}

// setup() runs once, when the device is first turned on
void setup() {
  // Put initialization like pinMode and begin functions here
  pinMode(SOIL_SENSOR, AN_INPUT);
  pinMode(LIGHT_SENSOR, AN_INPUT);
  pinMode(MOTOR, OUTPUT);
  Serial.begin(9600);
  //This is how we would trigger something remotely.
  //I'd kinda forgotten about Particle.function()
  //I'm not sure how many cloud functions we'll need,
  //but it supports up to twelve
  Particle.function("waterPlant", waterPlant);
  Particle.function("calibrate", updateDrynessThreshold);
}

// loop() runs over and over again, as quickly as it can execute.
time32_t timer = 0;

bool motorSpinning = false;
time32_t printDelay = 0;
long interval = 300000;

int soilRollingAvg[20];
int oldestIndex = 0;
int soilAvg = 0;
time32_t soilTimer = 0;

void loop() {
  int lightValue = analogRead(LIGHT_SENSOR);
  double lightToPercent = ((double) lightValue / 4095) * 100;
  int lightIntensity = (int) lightToPercent;
  
  int dirtValue = 4095 - analogRead(SOIL_SENSOR);

  int soilMoisture = soilAvg - 1050;
  double soilMoistureToPercent = ((double)soilMoisture / 1400) * 100;
  soilMoisture = (int) soilMoistureToPercent;

  //this conditional polls the soil sensor 20 times a second (ideally),
  //and keeps the last 20 datapoints. This should help smooth out some sensor
  //noise.
  if (millis() - soilTimer >= 50) {
    soilTimer = millis();
    soilRollingAvg[oldestIndex] = dirtValue;
    if (oldestIndex < 19) {
      oldestIndex++;
    } else oldestIndex = 0;
    
  }


  //every second or so, use the array of datapoints to generate
  //an average value for the last second or so.
  //This value is what will be sent to the webserver.
  if (millis() - printDelay >= 1000) {
    int sum = 0;
    for (int i = 0; i < 20; i++) {
      sum += soilRollingAvg[i];
    }
    sum /= 20;
    soilAvg = sum;
    printDelay = millis();
    Serial.printlnf("Soil Sensor: %d, Light Sensor: %d, Threshold: %d" , soilAvg, lightValue, drynessThreshold);
  }

  
  if (millis() - timer > interval) {
    char json[256]; // Get the json string for ThingSpeak
    snprintf(json, sizeof(json), "{\"lightIntensity\":%d,\"soilMoisture\":%d,\"isWatered\":%d}", lightIntensity, soilMoisture,
    wasWatered);
    Particle.publish("sendPlantData", json); // Send the data to the webhook
    wasWatered = false; 
    timer = millis();
  }
  // Send info to ubidots
  /*
  ubidots.add("LightIntensity", lightIntensity);
  ubidots.add("SoilMoisture", soilMoisture);
  ubidots.send();
  */

  //The website for the soil moisture said that to calibrate it, you just stick it in water, measure that value,
  //then measure its value in the air. I got about 1500 for the water value and 3050 for the air value.
  //I'm going to do some trial and error to figure out what's a good amount.

  //I got ~1050 for the dry value
  //I got ~2400 for the wet value
  //so there's a range of ~1350 that's actually valuable to us

  if (dirtValue < drynessThreshold && millis() - lastWatered > 120000) {
    
    waterPlant("");
  }
}
