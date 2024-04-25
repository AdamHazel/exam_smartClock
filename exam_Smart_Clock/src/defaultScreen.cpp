#include "mbed.h"
#include "classes.h"
#include <cstdio>

#define BUFFER_SIZE 17
void defaultScreen(std::pair<screen*, bool*> *info)
{
    while(true)
    {
        time_t seconds = time(NULL);

        static char buffer[BUFFER_SIZE];
        static char buffer2[BUFFER_SIZE];
        strftime(buffer, BUFFER_SIZE, "%a %d %b %H:%M", localtime(&seconds));
        if(*(info->second) == false)
            snprintf(buffer2, BUFFER_SIZE, "Alarm off");
        if(*(info->second) == true)
            snprintf(buffer2, BUFFER_SIZE, "Alarm on");

        info->first->messMut.lock();
        info->first->setLine_one(buffer);
        info->first->setLine_two(buffer2);
        info->first->messMut.unlock();
    }
}