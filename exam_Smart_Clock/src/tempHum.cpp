#include "mbed.h"
#include "HTS221Sensor.h"
#include "classes.h"
#include <cstdio>

#define MESSAGE_BUFFER_S 17

void tempHum(screen *scr)
{
    
    DevI2C i2c(PB_11, PB_10);
    HTS221Sensor hts221(&i2c);

    //Start up sensor
    if (hts221.init(NULL) != 0)
        printf("Failed to initialise HTS221 device\n");
    else
        printf("HTS221 device initialised\n");
    
    if (hts221.enable() != 0)
        printf("Failed to power up HTS221 device\n");
    else
        printf("HTS221 device powered up!");

    //Variables to record values
    float hum;
    float temp;

    while (true)
    {
        hts221.get_humidity(&hum);
        hts221.get_temperature(&temp);

        char tempInfo[MESSAGE_BUFFER_S];
        char humInfo[MESSAGE_BUFFER_S];

        snprintf(tempInfo, MESSAGE_BUFFER_S, "Temp: %.1f C", temp);
        snprintf(humInfo, MESSAGE_BUFFER_S, "Humidity: %.0f", hum);

        scr->messMut.lock();
        scr->setLine_one(tempInfo);
        scr->setLine_two(humInfo);
        scr->messMut.unlock();
    }
};