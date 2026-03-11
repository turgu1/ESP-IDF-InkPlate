/*
   Basic_monochorme example for e-radionica.com Inkplate devices
   For this example you will need only USB cable and Inkplate.
   Select "Inkplate 6(ESP32)" from Tools -> Board menu.
   Don't have "Inkplate 6(ESP32)" option? Follow our tutorial and add it:
   https://e-radionica.com/en/blog/add-inkplate-6-to-arduino-ide/

   This example will show you how you can draw some simple graphics using
   Adafruit GFX functions. Yes, Inkplate library is 100% compatible with GFX lib!
   Learn more about Adafruit GFX: https://learn.adafruit.com/adafruit-gfx-graphics-library )

   Want to learn more about Inkplate? Visit www.inkplate.io
   Looking to get support? Write on our forums: http://forum.e-radionica.com/en/
   15 July 2020 by e-radionica.com
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include <iostream>
#include <string>
#include <math.h>

#include "inkplate.hpp"
#include "logo.hpp"

Inkplate display(DisplayMode::INKPLATE_1BIT);

static const char *TAG = "Main";

uint16_t w;
uint16_t h;

// Small function that will write on the screen what function is currently in demonstration.
void displayCurrentAction(std::string text)
{
  display.setTextSize(2);
  display.setCursor(2, h - 20);
  display.print(text);
}

int random(int a, int b)
{
  // a -> 0
  // b -> RAND_MAX

  long long r = std::rand();
  return (a + (r * b) / RAND_MAX);
}

void mainTask(void *params)
{
  // Show a downcount of 10 seconds at the usb port
  for (int i = 3; i > 0; i--)
  {
    std::cout << "\r" << i << "..." << std::flush;
    ESP::delay(1000);
  }
  std::cout << std::endl
            << std::flush;

  ESP_LOGI(TAG, "Initialization.");

  display.begin();
  display.clearDisplay();
  display.display();

  w = display.width();
  h = display.height();

  ESP_LOGI(TAG, "Display size: width: %d, height: %d", w, h);

  display.drawRect(200, 200, 400, 300, BLACK); // Arguments are: start X, start Y, size X, size Y, color
  display.drawRect(180, 180, 440, 340, BLACK); // Arguments are: start X, start Y, size X, size Y, color
  display.drawRect(160, 160, 480, 380, BLACK); // Arguments are: start X, start Y, size X, size Y, color
  display.drawRect(140, 140, 520, 420, BLACK); // Arguments are: start X, start Y, size X, size Y, color

  display.drawFastHLine(100, 600, w - 200, BLACK); // Arguments are: starting X, starting Y, length, color
  display.drawFastHLine(100, 500, w - 200, BLACK); // Arguments are: starting X, starting Y, length, color
  display.drawFastHLine(100, 600, w - 200, BLACK); // Arguments are: starting X, starting Y, length, color
  display.drawFastHLine(100, 300, w - 200, BLACK); // Arguments are: starting X, starting Y, length, color
  display.drawFastHLine(100, 200, w - 200, BLACK); // Arguments are: starting X, starting Y, length, color
  display.drawFastHLine(100, 100, w - 200, BLACK); // Arguments are: starting X, starting Y, length, color

  display.setCursor(150, h / 2);
  display.setTextSize(4);

  ESP_LOGI(TAG, "Show Welcome Msg");

  display.print("Welcome to Inkplate 6V2!");

  display.display(); // Write hello message

  for (;;)
  {
    ESP::delay(5000);
  }
}

#define STACK_SIZE 20000

extern "C"
{

  void app_main()
  {
    TaskHandle_t xHandle = NULL;

    xTaskCreate(mainTask, "mainTask", STACK_SIZE, (void *)1, tskIDLE_PRIORITY, &xHandle);
    configASSERT(xHandle);
  }

} // extern "C"