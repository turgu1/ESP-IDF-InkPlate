#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "logging.hpp"

#include <iostream>
#include <string>

#include "inkplate.hpp"
#include "logo.hpp"

Inkplate display(DisplayMode::INKPLATE_1BIT);

static const char * TAG = "Main";

// Small function that will write on the screen what function is currently in demonstration.
void displayCurrentAction(std::string text)
{
  display.setTextSize(2);
  display.setCursor(2, 580);
  display.print(text);
}

void wait_a_bit() { vTaskDelay(5000 / portTICK_PERIOD_MS); }

void mainTask(void * params) 
{
  // Show a downcount of 10 seconds at the usb port
  for (int i = 10; i > 0; i--) {
    std::cout << "\r" << i << "..." << std::flush;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  std::cout << std::endl << std::flush;

  ESP_LOGI(TAG, "Initialization.");

  display.begin();
  display.clearDisplay();
  display.display();
  display.setCursor(150, 320);
  display.setTextSize(4);

  #if defined(INKPLATE_6)
    display.print("Welcome to Inkplate 6!");
  #else
    display.print("Welcome to Inkplate 10!");
  #endif

  display.display(); // Write hello message
  wait_a_bit();

  for (;;) { 
    // Example will demostrate funcionality one by one. You always first set everything in the frame buffer and
    // afterwards you show it on the screen using display.display().

    // Let's start by drawing a pixel at x = 100 and y = 50 location
    display.clearDisplay(); // Clear everytning that is inside frame buffer in ESP32
    displayCurrentAction(
        "Drawing a pixel"); // Function which writes small text at bottom left indicating what's currently done
                            // NOTE: you do not need displayCurrentAction function to use Inkplate!
    display.drawPixel(100, 50, BLACK); // Draw one black pixel at X = 100, Y = 50 position in BLACK color (must be black
                                        // since Inkplate is in BW mode)
    display.display(); // Send image to display. You need to call this one each time you want to transfer frame buffer
                        // to the screen.
    wait_a_bit();

    // Display some bitmap on screen. We are going to display e-radionica logo on display at location X = 200, Y = 200
    // Image is 576x100 pixels and we want to every pixel of this bitmap to be black.
    display.clearDisplay();
    display.drawImage(logo, 100, 250, 576, 100,
                      BLACK); // Arguments are: array variable name, start X, start Y, size X, size Y, color
    displayCurrentAction("Drawing e-radionica.com logo");
    display.display();
    wait_a_bit();

  } // Loop forever
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