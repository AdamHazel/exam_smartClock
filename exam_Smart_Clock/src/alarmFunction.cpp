#include "mbed.h"
#include "classes.h"
#include <cstdio>
#include <cstring>
#include <utility>
#include "structs.h"

#define BUFFER_SIZE 17

void alarmFunc(alarmScreen_struct* info)
{
    DigitalIn timePick(PA_0,PullNone);
    AnalogIn alarmIn(PA_1);

    static char buffer[BUFFER_SIZE];
    static char buffer2[BUFFER_SIZE];

    static constexpr int hourInterval = 24;
    static constexpr int minInterval = 60;
    static int hour = 0;
    static int min = 0;
    
    snprintf(buffer, BUFFER_SIZE, "Alarm test:");

    while (true)
    {
        if (*(info->screenN) == 1)
        {
            if (timePick.read() == 0)
            {
                hour = alarmIn.read() * hourInterval;
                printf("Alarm read value is %f\n", alarmIn.read());
            }
            if (timePick.read() == 1)
            {
                min = alarmIn.read() * minInterval; 
            }
            snprintf(buffer2, BUFFER_SIZE, "%02d : %02d", hour, min);
        }
        
        info->alarmS->messMut.lock();
        info->alarmS->setLine_one(buffer);
        info->alarmS->setLine_two(buffer2);
        info->alarmS->messMut.unlock();
        printf("%s\n", buffer2);
    }
};
