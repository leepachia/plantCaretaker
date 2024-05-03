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

// setup() runs once, when the device is first turned on
void setup() {
  // Put initialization like pinMode and begin functions here

  //This is how we would trigger something remotely.
  //I'd kinda forgotten about Particle.function()
  //I'm not sure how many cloud functions we'll need,
  //but it supports up to twelve
  Particle.function("waterPlant", waterPlant);
}

int waterPlant (String argument) {
  //turn the pin on, then off or something
  //like that.
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  lightIntensityFrequency = 20;
  soilMoistureLevel = 10;

  char json[256]; // Get the json string for ThingSpeak
  snprintf(json, sizeof(json), "{\"lightIntensity\":%.1f,\"soilMoisture\":%.2f}", lightIntensityFrequency, soilMoistureLevel);
  Particle.publish("sendPlantStats", json); // Send the data to the webhook

  // Send info to ubidots
  ubidots.add("LightIntensity", lightIntensityFrequency);
  ubidots.add("SoilMoisture", soilMoistureLevel);
  ubidots.send();

  delay(1000);
}
