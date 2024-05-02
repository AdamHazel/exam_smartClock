#include "mbed.h"
#include "classes.h"
#include <cstdio>
#include <cstring>
#include <utility>
#include "structs.h"

#define BUFFER_SIZE 17

void alarmFunc(alarmScreen_struct* info)
{
    DigitalIn timePick(PA_0,PullDown);
    AnalogIn alarmIn(PA_1);

    static char buffer[BUFFER_SIZE];
    static char buffer2[BUFFER_SIZE];

    static constexpr int hourInterval = 24;
    static constexpr int minInterval = 60;
    static int hour = 12;
    static int min = 54;
    
    snprintf(buffer, BUFFER_SIZE, "Alarm:");

    while (true)
    {
        // State 1 : To set alarm clock
        if (*(info->screenN) == 1 && *(info->alarmAct) == false && *(info->alarmEn) == false &&
            *(info->alarmSn) == false && *(info->alarmMut) == false)
        {
            if (timePick.read() == 0)
            {
                hour = alarmIn.read() * hourInterval;
                printf("Alarm read value is %f\n", alarmIn.read());
            }
            if (timePick.read() == 1)
            {
                if ( (alarmIn.read() * minInterval) > 1.5)
                    min = alarmIn.read() * minInterval;
                else
                    min = 0;
                printf("Alarm read value is %f\n", alarmIn.read()); 
            }
            snprintf(buffer2, BUFFER_SIZE, "%02d:%02d", hour, min);
        }

        // State 2 : Alarm enabled
        if (*(info->alarmAct) == false && *(info->alarmEn) == true &&
            *(info->alarmSn) == false && *(info->alarmMut) == false)
        {
            snprintf(buffer2, BUFFER_SIZE, "%02d:%02d", hour, min);
            snprintf(info->alarmBuf, BUFFER_SIZE, "%s", buffer2);
            //info->alarmBuf[strlen(buffer2)] = '\0';

            // Set alarm
        }

        
        info->alarmS->messMut.lock();
        info->alarmS->setLine_one(buffer);
        info->alarmS->setLine_two(buffer2);
        info->alarmS->messMut.unlock();
    }
};
