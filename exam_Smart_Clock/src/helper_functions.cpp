#include "mbed.h"
#include "DFRobot_RGBLCD1602.h"


void lcd_initialise(DFRobot_RGBLCD1602 &a) 
{
    a.init();
    a.display();
    a.clear();
    printf("LCD initialised\n");
}

void screenCheck(bool &screenChn, DFRobot_RGBLCD1602 &a, int &screenN)
{
    if(screenChn == true) {
        a.clear();
        printf("Screen number is %i\n",screenN);
        screenChn = false;
    }
}