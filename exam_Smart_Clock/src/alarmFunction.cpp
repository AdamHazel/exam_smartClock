#include "mbed.h"
#include "classes.h"
#include <cstdio>
#include <cstring>
#include <utility>
#include "structs.h"

#define BUFFER_SIZE 17

void alarmFunc(std::pair<screen*, bool*> *info)
{
    while (true)
    {
        static char buffer[BUFFER_SIZE];
        static char buffer2[BUFFER_SIZE];
        snprintf(buffer, BUFFER_SIZE, "Alarm screen");
        snprintf(buffer2, BUFFER_SIZE, "Shows time");

        info->first->messMut.lock();
        info->first->setLine_one(buffer);
        info->first->setLine_two(buffer2);
        info->first->messMut.unlock();
    }
};
