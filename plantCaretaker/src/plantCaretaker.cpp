/* 
 * Project: Plant Caretaker
 * Authors: Pachia Lee and Jake Robbins
 * Date: 
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"

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
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  lightIntensityFrequency = 20;
  soilMoistureLevel = 10;

  char json[256]; // Get the json string for ThingSpeak
  snprintf(json, sizeof(json), "{\"lightIntensity\":%.1f,\"soilMoistureLevel\":%.2f}", lightIntensityFrequency, soilMoistureLevel);
  Particle.publish("sendPlantStats", json); // Send the data to the webhook

  delay(1000);
}
