/**
 * @file newsScreen.cpp
 * @author Tina, Ørjan, Lasse og Adam
 */

#include "classes.h"
#include "mbed.h"
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <string>

#include "certificates_hosts.h"
#include "helper_functions.h"
#include "json.hpp"
#include "structs.h"

#define RESOURCE_S 100
#define MESSAGE_BUFFER_S 16

using namespace std::chrono;

/**
 * @brief Fetches news and prepare string to display on string
 * @param newsFetch_struct that holds pointers to all needed variables
 */

void newsFetch(newsFetch_struct *info) {
  // Sizes for buffers and display
  constexpr int BUFFER_SIZE = 200;
  static constexpr int DISPLAY_S = 16;

  // Variabes to help manage how often information is fetched
  static bool completed = false;
  static bool reset = false;
  static Timer t;
  static int second;
  static constexpr int minutesToWait = 15;

  // Display variables
  static char lineOne[MESSAGE_BUFFER_S];
  static char lineTwo[MESSAGE_BUFFER_S];
  static char spacing[] = "                ";
  static char firstMessage[] = "News loading       ";
  std::string title;
  std::string newsLine;

  // Tools for finding strings
  static std::string newsNeedle_B = "<title><![CDATA[";
  static std::string newsNeedle_E = "]]></title>";

  while (true) {
    if (*(info->screenN) == 5 && completed == false) {
      // Initial message displayed
      info->newsS->messMut.lock();
      info->newsS->setLine_one(firstMessage);
      info->newsS->setLine_two(spacing);
      info->newsS->messMut.unlock();

      // Get news information
      info->netMut->lock();
      std::string *resultString =
          getInformation_http_NOTJSON(BUFFER_SIZE, newshost, newsResource);
      info->netMut->unlock();

      // Find headlines and title, and place in a new string
      size_t needleS;
      size_t needleE;
      needleS = resultString->find(newsNeedle_E);
      if (needleS) {
        printf("Managed to find needle. Position %i\n", needleS);
      }

      title = resultString->substr(0, 7);

      int count = 3;
      for (int i = 0; i < count; i++) {
        needleS = resultString->find(newsNeedle_B, needleS);
        needleS += newsNeedle_B.size();
        if (needleS) {
          printf("\nManaged to find start needle. Position %i\n", needleS);
        }
        needleE = resultString->find(newsNeedle_E, needleS);
        if (needleE) {
          printf("Managed to find end needle. Position %i\n", needleE);
        }
        auto length = needleE - needleS;
        std::string text = resultString->substr(needleS, length);
        printf("News headline: %s\n", text.c_str());
        newsLine.append(text);
        newsLine.append(" --- ");
      }
      printf("\nRemaining stack space = %u bytes\n",
             osThreadGetStackSpace(ThisThread::get_id()));
      delete resultString;
      *(info->loading) = false;
      completed = true;
    }

    if (*(info->screenN) == 5 && completed == true) {
      // Start timer to refresh news
      t.start();

      // Place title in buffer
      snprintf(lineOne, MESSAGE_BUFFER_S, "%s%s", title.c_str(), spacing);

      // Place news in buffer
      size_t i = 0;
      size_t limit = newsLine.size() - DISPLAY_S;
      while (i <= limit) {
        second = (int)duration_cast<seconds>(t.elapsed_time()).count();
        if (*(info->screenN) != 5) {
          reset = true;
          break;
        }

        if (second == (minutesToWait * 60)) {
          reset = true;
          break;
        }

        // Bool value used so that no characters are skipped on display
        if (*(info->ready) == false) {
          std::string lineSub = newsLine.substr(i, DISPLAY_S);
          snprintf(lineTwo, MESSAGE_BUFFER_S, "%s", lineSub.c_str());
          info->newsS->messMut.lock();
          info->newsS->setLine_one(lineOne);
          info->newsS->setLine_two(lineTwo);
          info->newsS->messMut.unlock();
          *(info->ready) = true;
          i++;
        }
      }
    }

    // Reset function so that it is ready to be used again
    if (reset == true) {
      newsLine.clear();
      *(info->loading) = true;
      t.stop();
      t.reset();
      completed = false;
    }
  }
}