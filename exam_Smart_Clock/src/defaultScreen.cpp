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
        
        // Used for clearing rest of lcd screen
        static char spacing[BUFFER_SIZE] = "                ";  

        static char buffer[BUFFER_SIZE];
        static char buffer2[BUFFER_SIZE];
        strftime(buffer, BUFFER_SIZE, "%a %d %b %H:%M", localtime(&seconds));

        // State 1: When disabled is enabled
        if (*(info->alarmAct) == false && *(info->alarmEn) == false &&
            *(info->alarmSn) == false && *(info->alarmMut) == false)
        {
            snprintf(buffer2, BUFFER_SIZE, "%s",spacing);
        }
        
        // State 2: When alarm is enabled
        if (*(info->alarmAct) == false && *(info->alarmEn) == true &&
            *(info->alarmSn) == false)
        {
            snprintf(buffer2, BUFFER_SIZE, "Alarm      %s%s", info->alarmBuf,spacing);
        }

        // State 3: When alarm is active
        if (*(info->alarmAct) == true && *(info->alarmEn) == true &&
            *(info->alarmSn) == false && *(info->alarmMut) == false)
        {
            snprintf(buffer2, BUFFER_SIZE, "Alarm (A)  %s%s", info->alarmBuf,spacing);
        }

        // State 4 : Alarm snoozed
        if (*(info->alarmAct) == false && *(info->alarmEn) == true &&
            *(info->alarmSn) == true && *(info->alarmMut) == false)
        {
            snprintf(buffer2, BUFFER_SIZE, "Alarm (S)  %s%s", info->alarmBuf,spacing);
        }

        

        info->defaultS->messMut.lock();
        info->defaultS->setLine_one(buffer);
        info->defaultS->setLine_two(buffer2);
        info->defaultS->messMut.unlock();
    }
}