#include "mbed.h"
#include "classes.h"
#include "DFRobot_RGBLCD1602.h"
#include <chrono>
#include <cstdio>
#include <cstring>
#include <utility>
#include "structs.h"

#define BUFFER_SIZE 17
using namespace std::chrono;

void alarmFunc(alarmScreen_struct* info)
{
    DigitalIn timePick(PA_0, PullDown);
    AnalogIn alarmIn(PA_1);
    PwmOut buzzer(PA_7);
    buzzer.period(1.0 / 2000);

    static char buffer[BUFFER_SIZE];
    static char buffer2[BUFFER_SIZE];

    static constexpr int hourInterval = 24;
    static constexpr int minInterval = 60;

    static constexpr int alarmDurS = 60;
    static constexpr int snoozeDurS = 60;

    static int hour = 0;
    static int min = 0;
    static Timer alarmT;
    static Timer snoozeT;
    static int secondCounter = 0;
    
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

            // Alarm work alarm
            char alarmCheck[BUFFER_SIZE];
            time_t seconds = time(NULL);
            strftime(alarmCheck, BUFFER_SIZE, "%H:%M", localtime(&seconds));
            printf("\nTime check: %s and alarm time: %s", alarmCheck, info->alarmBuf);
            if(strcmp(alarmCheck,info->alarmBuf) == 0)
            {
                *(info->alarmAct) = true;
            }
        }

        // State 3 : Alarm active
        if (*(info->alarmAct) == true && *(info->alarmEn) == true &&
            *(info->alarmSn) == false && *(info->alarmMut) == false)
        {
            // Alarm sounds
            alarmT.start();
            buzzer.write(0.2);
            secondCounter = (int) duration_cast<seconds>(alarmT.elapsed_time()).count();
            printf("Alarm is now active. Time gone is %i\n", secondCounter);
            
            // Alarm mutes when it runs out
            if (secondCounter == alarmDurS)
            {
                buzzer.write(0.0);
                alarmT.stop();
                alarmT.reset();
                *(info->alarmAct) = false; 
            }
            

        }

        // State 4 : Alarm snoozed
        if (*(info->alarmAct) == false && *(info->alarmEn) == true &&
            *(info->alarmSn) == true && *(info->alarmMut) == false)
        {
            buzzer.write(0.0);
            alarmT.stop();
            alarmT.reset();
            
            snoozeT.start();
            secondCounter = (int) duration_cast<seconds>(snoozeT.elapsed_time()).count();
            printf("Alarm is now snoozed. Time gone is %i\n", secondCounter);

            // Snooze timer
            if (secondCounter == snoozeDurS) 
            {
                snoozeT.stop();
                snoozeT.reset();
                *(info->alarmAct) = true;
                *(info->alarmSn) = false;
            }
        }

        // State 5 : Alarm mute
        if (*(info->alarmEn) == true && *(info->alarmMut) == true)
        {
            buzzer.write(0.0);
            alarmT.stop();
            alarmT.reset();

            snoozeT.stop();
            alarmT.reset();
            *(info->alarmSn) = false;
            *(info->alarmAct) = false;
            *(info->alarmMut) = false;
        }

       

        // Place to display buffer (object)
        info->alarmS->messMut.lock();
        info->alarmS->setLine_one(buffer);
        info->alarmS->setLine_two(buffer2);
        info->alarmS->messMut.unlock();
    }
};
