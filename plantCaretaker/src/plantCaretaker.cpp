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

const int SOIL_SENSOR = A1;
const int LIGHT_SENSOR = A5;
const int MOTOR = D2;

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

int waterPlant (String argument) {
  //turn the pin on, then off or something
  //like that.
  return 1; //or something
}

// setup() runs once, when the device is first turned on
void setup() {
  // Put initialization like pinMode and begin functions here
  pinMode(SOIL_SENSOR, AN_INPUT);
  pinMode(LIGHT_SENSOR, AN_INPUT);
  Serial.begin(9600);
  //This is how we would trigger something remotely.
  //I'd kinda forgotten about Particle.function()
  //I'm not sure how many cloud functions we'll need,
  //but it supports up to twelve
  Particle.function("waterPlant", waterPlant);
}

// loop() runs over and over again, as quickly as it can execute.
time32_t timer = 0;
bool motorSpinning = false;
void loop() {

  int lightIntensity = analogRead(LIGHT_SENSOR);
  int soilMoisture = analogRead(SOIL_SENSOR);

  Serial.printlnf("Soil Sensor: %d, Light Sensor: %d", moistness, brightness);
  
  char json[256]; // Get the json string for ThingSpeak
  snprintf(json, sizeof(json), "{\"lightIntensity\":%.1f,\"soilMoisture\":%.2f}", lightIntensityFrequency, soilMoistureLevel);
  if (timer > 30000) {
    Particle.publish("sendPlantData", json); // Send the data to the webhook
    timer = millis();
  }
  // Send info to ubidots
  ubidots.add("LightIntensity", brightness);
  ubidots.add("SoilMoisture", moistness);
  ubidots.send();

  if (motorSpinning) {
    digitalWrite(MOTOR, LOW);
    motorSpinning = false;
  } else {
    digitalWrite(MOTOR, HIGH);
    motorSpinning = true;
  }
  

  delay(1000);
}
