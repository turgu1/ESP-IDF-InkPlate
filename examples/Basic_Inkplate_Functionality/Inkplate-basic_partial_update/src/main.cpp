/*
   Basic_partial_update example for e-radionica Inkplate 6
   For this example you will need only USB cable and Inkplate 6
   Select "Inkplate 6(ESP32)" from Tools -> Board menu.
   Don't have "Inkplate 6(ESP32)" option? Follow our tutorial and add it:
   https://e-radionica.com/en/blog/add-inkplate-6-to-arduino-ide/

   In this example we will show  how to use partial update functionality of Inkplate 6 e-paper display.
   It will scroll text that is saved in char array
   NOTE: Partial update is only available on 1 Bit mode (BW) and it is not recommended to use it on first refresh after
   power up. It is recommended to do a full refresh every 5-10 partial refresh to maintain good picture quality.

   Want to learn more about Inkplate? Visit www.inkplate.io
   Looking to get support? Write on our forums: http://forum.e-radionica.com/en/
   15 July 2020 by e-radionica.com
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "inkplate.hpp"            //Include Inkplate library to the sketch
Inkplate display(DisplayMode::INKPLATE_1BIT); // Create an object on Inkplate library and also set library into 1-bit mode (BW)

static const char * TAG = "Main";

// Char array where you can store your text that will be scrolled.
#if INKPLATE_6
  const char text[] = "This is partial update on Inkplate 6 e-paper display! :)";
  int max = 9;
#elif INKPLATE_6PLUS
  const char text[] = "This is partial update on Inkplate 6PLUS e-paper display! :)";
  int max = 999;
#else
  const char text[] = "This is partial update on Inkplate 10 e-paper display! :)";
  int max = 9;
#endif

// This variable is used for moving the text (scrolling)
int offset;
int w, h;

// Variable that keeps count on how much screen has been partially updated
int n = 0;
void mainTask(void * param)
{
  display.begin();                    // Init Inkplate library (you should call this function ONLY ONCE)
  display.clearDisplay();             // Clear frame buffer of display
  display.display();                  // Put clear image on display

  w = display.width();
  h = display.height();

  offset = w;

  ESP_LOGI(TAG, "Display size: width: %d, height: %d", w, h);

  display.setTextColor(BLACK, WHITE); // Set text color to be black and background color to be white
  display.setTextSize(4);             // Set text to be 4 times bigger than classic 5x7 px text
  display.setTextWrap(false);         // Disable text wraping

  for(;;) {
    display.clearDisplay();         // Clear content in frame buffer
    display.setCursor(offset, h / 2); // Set new position for text
    display.print(text);            // Write text at new position
    if (n > max)
    {                      // Check if you need to do full refresh or you can do partial update
        display.display(); // Do a full refresh
        n = 0;
    }
    else
    {
        display.partialUpdate(); // Do partial update
        n++;                     // Keep track on how many times screen has been partially updated
    }
    offset -= 20; // Move text into new position
    if (offset < 0)
        offset = w; // Text is scrolled till the end of the screen? Get it back on the start!
    ESP::delay(200);   // Delay between refreshes.
  }
}


#define STACK_SIZE 10000

extern "C" {

  void app_main()
  {
    TaskHandle_t xHandle = NULL;

    xTaskCreate(mainTask, "mainTask", STACK_SIZE, (void *) 1, tskIDLE_PRIORITY, &xHandle);
    configASSERT(xHandle);
  }

} // extern "C"