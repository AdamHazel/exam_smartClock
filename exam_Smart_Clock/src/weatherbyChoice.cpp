#include "classes.h"
#include "mbed.h"
#include <cstdio>
#include <string>
#include <chrono>
#include <string>

#include "structs.h"
#include "certificates_hosts.h"
#include "helper_functions.h"
#include "json.hpp"

#define MESSAGE_BUFFER_S 17

using json = nlohmann::json;
using namespace std::chrono;

void weatherbyChoice(weatherChoice_struct* info)
{
    // Buffers
    static char resource[100];
    constexpr int BUFFER_SIZE = 200;

    /*

    Step 1:
        Ask user for input
    
    Step 2: 
        Get weather based on input
            if input contains error, catch it, return to step 1
        Start timer for updating
            When timer is up, return to step 1

    Step 3: 
        If user changes screen:
            Stop timer, reset timer, return to step one
    */
}