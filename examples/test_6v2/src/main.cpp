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

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <iostream>
#include <math.h>
#include <string>

#include "inkplate.hpp"

#include "nvs_flash.h"
#include <esp_chip_info.h>
#include <esp_flash.h>

Inkplate display(DisplayMode::INKPLATE_1BIT);

static const char *TAG = "Main";

uint16_t w;
uint16_t h;

void mainTask(void *params) {
  // Show a downcount of 10 seconds at the usb port
  for (int i = 3; i > 0; i--) {
    std::cout << "\r" << i << "..." << std::flush;
    ESP::delay(1000);
  }
  std::cout << std::endl << std::flush;

  ESP_LOGI(TAG, "Initialization.");

  display.begin();
  display.clearDisplay();

  w = display.width();
  h = display.height();

  ESP_LOGI(TAG, "Display size: width: %d, height: %d", w, h);

  display.drawRect(200, 200, 400, 300,
                   BLACK); // Arguments are: start X, start Y, size X, size Y, color
  display.drawRect(180, 180, 440, 340,
                   BLACK); // Arguments are: start X, start Y, size X, size Y, color
  display.drawRect(160, 160, 480, 380,
                   BLACK); // Arguments are: start X, start Y, size X, size Y, color
  display.drawRect(140, 140, 520, 420,
                   BLACK); // Arguments are: start X, start Y, size X, size Y, color

  display.setCursor(150, h / 2);
  display.setTextSize(4);

  ESP_LOGI(TAG, "Show Welcome Msg");

  display.print("Welcome to Inkplate 6V2!");

  display.display(); // Write hello message

  for (;;) {
    ESP::delay(5000);
  }
}

#define STACK_SIZE 60000

extern "C" {

void app_main() {
  auto err = nvs_flash_init();
  if (err != ESP_OK) {
    if ((err == ESP_ERR_NVS_NO_FREE_PAGES) || (err == ESP_ERR_NVS_NEW_VERSION_FOUND)) {
      ESP_LOGI(TAG, "Erasing NVS Partition... (Because of %s)", esp_err_to_name(err));
      if ((err = nvs_flash_erase()) == ESP_OK) {
        err = nvs_flash_init();
      }
    }
  }

  /* Print chip information */
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  printf("This is %s chip with %d CPU core(s), WiFi%s%s, ", CONFIG_IDF_TARGET, chip_info.cores,
         (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
         (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

  printf("silicon revision %d, ", chip_info.revision);

  uint32_t size_flash_chip;
  esp_flash_get_size(NULL, &size_flash_chip);

  printf("%" PRIu32 "MB %s flash\n", size_flash_chip / (1024 * 1024),
         (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

  printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

  heap_caps_print_heap_info(MALLOC_CAP_32BIT | MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM |
                            MALLOC_CAP_INTERNAL);

  TaskHandle_t xHandle = NULL;

  xTaskCreate(mainTask, "mainTask", STACK_SIZE, (void *)1, configMAX_PRIORITIES - 1, &xHandle);
  configASSERT(xHandle);
}

} // extern "C"