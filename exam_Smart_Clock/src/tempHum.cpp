/**
 * @file tempHum.cpp
 * @author Tina, Ørjan, Lasse og Adam
 */

#include "HTS221Sensor.h"
#include "classes.h"
#include "mbed.h"
#include <cstdio>

#define MESSAGE_BUFFER_S 17

/**
 * @brief Fetches temperature and humidity values and prepares data to be
 * displayed on LCD display
 * @param pointer to screen object
 */
void tempHum(screen *scr) {
  // Initialise objects needed for sensor
  DevI2C i2c(PB_11, PB_10);
  HTS221Sensor hts221(&i2c);

  // Initialise buffers and strings needed to prepare display information
  static char tempInfo[MESSAGE_BUFFER_S];
  static char humInfo[MESSAGE_BUFFER_S];
  static char spacing[MESSAGE_BUFFER_S] = "                ";

  // Start up sensor
  if (hts221.init(NULL) != 0)
    printf("Failed to initialise HTS221 device\n");
  else
    printf("HTS221 device initialised\n");

  if (hts221.enable() != 0)
    printf("Failed to power up HTS221 device\n");
  else
    printf("HTS221 device powered up!");

  // Variables to record values
  float hum;
  float temp;

  while (true) {
    hts221.get_humidity(&hum);
    hts221.get_temperature(&temp);

    snprintf(tempInfo, MESSAGE_BUFFER_S, "Temp: %.1f C%s", temp, spacing);
    snprintf(humInfo, MESSAGE_BUFFER_S, "Humidity: %.0f", hum);

    // Send prepared information to screen object so that it can be displayed
    scr->messMut.lock();
    scr->setLine_one(tempInfo);
    scr->setLine_two(humInfo);
    scr->messMut.unlock();
  }
};