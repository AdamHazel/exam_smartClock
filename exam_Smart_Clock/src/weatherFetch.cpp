#include "DFRobot_RGBLCD1602.h"
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

void weatherFetch(weatherAuto_struct* info)
{
    // Desired resource
    static char resource[100];
    snprintf(resource, 100, "/v1/current.json?key=e67b72e911fc4802b19175420241205&q=%s,%s&aqi=no", info->latit->c_str(), info->longit->c_str());
    constexpr int BUFFER_SIZE = 200;

    // Variabes to help manage how often information is fetched
    static bool completed = false;
    static Timer t;
    static int second;
    static constexpr int minutesToWait = 15;

    // Variables to place weather information
    static std::string summary;
    static int temperature;
    static char weatherText[MESSAGE_BUFFER_S];
    static char temperatureText[MESSAGE_BUFFER_S];
    static char spacing[] = "                "; 
    static char firstMessage[] = "Data loading       ";

    info->weatherS->messMut.lock();
    info->weatherS->setLine_one(firstMessage);
    info->weatherS->setLine_two(spacing);
    info->weatherS->messMut.unlock();

    while (true)
    {
        // What should happen when screen is chosen or it is time to get new information
        if ( *(info->screenN) == 3 && completed == false)
        {
            info->netMut->lock();
            char *json_response = getInformation_http(BUFFER_SIZE, apiweatherhost, resource);
            info->netMut->unlock();
            
            printf("JSON RESPONSE AGAIN TO CHECK:\n\n%s", json_response);
            
            json document = json::parse(json_response);
            if (document.is_discarded()) {
                printf("The input is invalid JSON\n");
                continue;
            }

            temperature = document["current"]["temp_c"].get<int>();
            summary = document["current"]["condition"]["text"];

            snprintf(weatherText, MESSAGE_BUFFER_S, "%s%s", summary.c_str(), spacing);
            snprintf(temperatureText, MESSAGE_BUFFER_S, "%i degrees%s", temperature, spacing);

            info->weatherS->messMut.lock();
            info->weatherS->setLine_one(weatherText);
            info->weatherS->setLine_two(temperatureText);
            info->weatherS->messMut.unlock();
            
            completed = true;
            
        }

        // What should happen when information is gotten and the screen is not changed
        if (completed == true)
        {
            t.start();
            second = (int) duration_cast<seconds>(t.elapsed_time()).count();
            printf("Seconds going %i\n", second);
            if (second == (minutesToWait * 60)) 
            {
                t.stop();
                t.reset();
                completed = false;
            }
        }

        // What should happen when the screen is changed
        if (*(info->screenChng) == true)
        {
            t.stop();
            t.reset();
            completed = false;
        }

        
    }   
}