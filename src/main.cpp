#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "logging.hpp"

#include <iostream>

#include "inkplate.hpp"

static const char * TAG = "Main";

void mainTask(void * params) 
{
  for (int i = 10; i > 0; i--) {
    std::cout << "\r" << i << "..." << std::flush;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  std::cout << std::endl << std::flush;

  if (e_ink.setup()) {
    ESP_LOGI(TAG, "EInk initialized!");

    static Graphics graphics;

    e_ink.clean();

    ESP_LOGI(TAG, "Set Display Mode...");
    graphics.selectDisplayMode(3);
    ESP_LOGI(TAG, "writeLine...");
    graphics.writeLine(10, 10, 200, 200, 0);
    graphics.writeFillRect(400, 400, 150, 150, 0);
    ESP_LOGI(TAG, "Show...");
    graphics.show();
  }
  
  while (1) {
    std::cout << "Job completed..." << std::endl << std::flush;
    vTaskDelay(10000 / portTICK_PERIOD_MS);
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