#include "freertos/FreeRTOS.h"
#include "freertos/task.h"s
#include "nvs_flash.h"
#include "esp_log.h"

#include <iostream>
#include <string>
#include <math.h>

#include "inkplate.hpp"
#include "network_client.hpp"

#include "secur.hpp"

Inkplate display(DisplayMode::INKPLATE_1BIT);

static const char * TAG = "Main";

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

void delay(int sec = 5) { vTaskDelay((sec * 1000) / portTICK_PERIOD_MS); }

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
  display.setRotation(0);

  w = display.width();
  h = display.height();

  ESP_LOGI(TAG, "Display size: width: %d, height: %d", w, h);

  display.setCursor(50, h / 2);
  display.setTextSize(4);

  #if defined(INKPLATE_6)
    display.print("Network test for Inkplate 6!");
  #elif defined(INKPLATE_6PLUS)
    display.print("Network test for Inkplate 6PLUS!");
  #else
    display.print("Network test for Inkplate 10!");
  #endif

  display.display(); // Write hello message
  delay();

  if (display.joinAP(YOUR_SSID, YOUR_PASSWORD)) {
    int32_t size;
    network_client.downloadFile("https://api.varipass.org/?action=sgraph&id=kbg3eQfA&width=400&height=300&eink=true", &size);
  }

  for (;;) {
    ESP_LOGI(TAG, "Completed...");
    delay(15);
  }

}


#define STACK_SIZE 10000

extern "C" {

  void app_main()
  {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    TaskHandle_t xHandle = NULL;

    xTaskCreate(mainTask, "mainTask", STACK_SIZE, (void *) 1, tskIDLE_PRIORITY, &xHandle);
    configASSERT(xHandle);
  }

} // extern "C"