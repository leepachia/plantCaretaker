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

int waterPlant (String argument) {
  //turn the pin on, then off or something
  //like that.
  digitalWrite(MOTOR, HIGH);
  delay(1500);
  digitalWrite(MOTOR, LOW);
  lastWatered = millis();
  Particle.publish("plantWatered");
  return 1; //or something
}

int updateDrynessThreshold(String argument) {
  int newThreshold = atoi(argument);
  drynessThreshold = newThreshold;
  return newThreshold;
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

void loop() {

  int lightIntensity = analogRead(LIGHT_SENSOR);
  int soilMoisture = 4095 - analogRead(SOIL_SENSOR);
  if (millis() - printDelay >= 1000) {
    printDelay = millis();
    Serial.printlnf("Soil Sensor: %d, Light Sensor: %d, Device ID: %d" , soilMoisture, lightIntensity, System.deviceID());
  }
  char json[256]; // Get the json string for ThingSpeak
  snprintf(json, sizeof(json), "{\"lightIntensity\":%d,\"soilMoisture\":%d}", lightIntensity, soilMoisture);
  if (millis() - timer > 30000) {
    Particle.publish("sendPlantData", json); // Send the data to the webhook
    timer = millis();
  }
  // Send info to ubidots
  ubidots.add("LightIntensity", lightIntensity);
  ubidots.add("SoilMoisture", soilMoisture);
  ubidots.send();

  /*if (motorSpinning) {
    digitalWrite(MOTOR, LOW);
    motorSpinning = false;
  } else {
    digitalWrite(MOTOR, HIGH);
    motorSpinning = true;
    delay(2000);
  }
  */

  //The website for the soil moisture said that to calibrate it, you just stick it in water, measure that value,
  //then measure its value in the air. I got about 1500 for the water value and 3050 for the air value.
  //I'm going to do some trial and error to figure out what's a good amount.

  //I got ~1050 for the dry value
  //I got ~2400 for the wet value
  //so there's a range of ~1350 that's actually valuable to us

  if (soilMoisture < drynessThreshold && millis() - lastWatered > 120000) {
    
    waterPlant("");
  }
}
