#include "inkplate.hpp"

Inkplate display(DisplayMode::INKPLATE_1BIT);

static const char * TAG = "Main";

volatile bool handler_called = false;

void touch_screen_handler(void * arg)
{
  handler_called = true;
}

void mainTask(void * params) 
{
  // put your setup code here, to run once:
  display.begin(false, touch_screen_handler);

  // NOTE!!!
  // Touchscreen cooridinates are automatically swapped and adjusted when screen is rotated
  display.setRotation(2);
  display.fillTriangle(10, 10, 20, 40, 40, 20, BLACK);
  display.setTextSize(3);
  display.setCursor(60, 60);
  display.print("(0,0) position");
  display.display();

  for (;;) {

    // Check if there is any touch detected
    
    if (display.lightSleep(1, TouchScreen::PIN_TOUCHSCREEN_INTERRUPT, 0)) {
      ESP_LOGI(TAG, "Light Sleep Timeout...");
    }
    else {
      ESP_LOGI(TAG, "Interupt Occured...");
    };
    ESP::delay(1000);

    if (handler_called) {
      ESP_LOGI(TAG, "Handler called...");
      handler_called = false;
    }

    if (display.tsAvailable()) {
      uint8_t n;
      TouchScreen::TouchPositions x, y;

      // See how many fingers are detected (max 2) and copy x and y position of each finger on touchscreen

      n = display.tsGetData(x, y);
      if (n != 0) {
        ESP_LOGI(TAG, "%d finger%c ", n, n > 1 ? 's' : ' ');
        for (int i = 0; i < n; i++) {
          ESP_LOGI(TAG, "%d: X=%d Y=%d ", i, x[i], y[i]);
        }
      }
      else {
        x[0] = 0;
        x[1] = 0;
        y[0] = 0;
        y[1] = 0;
        
        ESP_LOGI(TAG, "Release");
      }
    }
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