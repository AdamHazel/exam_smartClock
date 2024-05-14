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

#define RESOURCE_S 200
#define PLACE_S 100
#define JSON_NOEXCEPTION
#define MESSAGE_BUFFER_S 17

using json = nlohmann::json;
using namespace std::chrono;

void weatherbyChoice(weatherChoice_struct* info)
{
    //Bufferd serial
    BufferedSerial input(USBTX, USBRX);
    input.set_baud(115200);
    
    // Buffers
    static char resource[RESOURCE_S];

    static char place[PLACE_S + 1];
    constexpr int BUFFER_SIZE = 200;

    // Controls
    static bool placeGotten = false;
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


    info->weatherC->messMut.lock();
    info->weatherC->setLine_one(spacing);
    info->weatherC->setLine_two(spacing);
    info->weatherC->messMut.unlock();

    while (true)
    {
        if (placeGotten == false && *(info)->screenN == 4)
        {
            memset(place, 0, PLACE_S);
            char oneLetter = '\0';

            for (int i = 0; i < PLACE_S; i++)
            {
                input.read(&oneLetter, sizeof(oneLetter));
                if(oneLetter == '\r' || oneLetter == '\n')
                {
                    place[i] = '\0';
                    break;
                } else {
                    place[i] = oneLetter;
                }
            }
            placeGotten = true;
        }

        if (placeGotten == true && completed == false)
        {
            // Set place in resource
            snprintf(resource, RESOURCE_S, "/v1/current.json?key=e67b72e911fc4802b19175420241205&q=%s&aqi=no", place);
            info->netMut->lock();
            char *json_response = getInformation_http(BUFFER_SIZE, apiweatherhost, resource);
            info->netMut->unlock();

            // Parsing JSON
            //printf("JSON RESPONSE AGAIN TO CHECK:\n\n%s", json_response);
            
            json document = json::parse(json_response);
            if (document.is_discarded()) {
                printf("The input is invalid JSON\n");
                continue;
            }

            if (document["current"]["temp_c"].is_number_float() && document["current"]["condition"]["text"].is_string())
            {
                summary = document["current"]["condition"]["text"];
                temperature = document["current"]["temp_c"].get<int>();

                snprintf(weatherText, MESSAGE_BUFFER_S, "%s%s", summary.c_str(), spacing);
                snprintf(temperatureText, MESSAGE_BUFFER_S, "%i degrees%s", temperature, spacing);
            } else {
                snprintf(weatherText, MESSAGE_BUFFER_S, "Error%s", spacing);
                snprintf(temperatureText, MESSAGE_BUFFER_S, "No info found%s", spacing);
            }

            info->weatherC->messMut.lock();
            info->weatherC->setLine_one(weatherText);
            info->weatherC->setLine_two(temperatureText);
            info->weatherC->messMut.unlock();
            
            printf("\n\n\nThe city is: %s\n", place);
            completed = true;
        }

        if (placeGotten == true && completed == true ) 
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

        if (*(info->screenN) != 4)
        {
            t.stop();
            t.reset();
            completed = false;
            placeGotten = false;   
        }
    } 
}