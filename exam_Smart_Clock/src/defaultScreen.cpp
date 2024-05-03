#include "mbed.h"
#include "classes.h"
#include <cstdio>

#include "structs.h"

#define BUFFER_SIZE 17
void defaultScreen(defaultScreen_struct* info)
{
    while(true)
    {
        time_t seconds = time(NULL);

        static char buffer[BUFFER_SIZE];
        static char buffer2[BUFFER_SIZE];
        strftime(buffer, BUFFER_SIZE, "%a %d %b %H:%M", localtime(&seconds));

        // State 2: When alarm is enabled
        if (*(info->alarmAct) == false && *(info->alarmEn) == true &&
            *(info->alarmSn) == false && *(info->alarmMut) == false)
        {
            snprintf(buffer2, BUFFER_SIZE, "Alarm  %s", info->alarmBuf);
        }

        // State 3: When alarm is active
        if (*(info->alarmAct) == true && *(info->alarmEn) == true &&
            *(info->alarmSn) == false && *(info->alarmMut) == false)
        {
            snprintf(buffer2, BUFFER_SIZE, "Alarm (A) %s", info->alarmBuf);
        }

        //State 5 : 

        

        info->defaultS->messMut.lock();
        info->defaultS->setLine_one(buffer);
        info->defaultS->setLine_two(buffer2);
        info->defaultS->messMut.unlock();
    }
}