// Copyright (c) 2020 Guy Turcotte
//
// MIT License. Look at file licenses.txt for details.

#pragma once

#include "esp_log.h"

#include <cinttypes>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "driver/gpio.h"
#include "driver/adc.h"

static const uint8_t HIGH = 1;
static const uint8_t LOW  = 0;

/**
 * @brief ESP-IDF support methods
 * 
 * These are class methods that simplify access to some esp-idf specifics.
 * The class is taylored for the needs of the low-level driver classes of the InkPlate-6
 * software.
 */
class ESP
{
  private:
    static constexpr char const * TAG = "ESP";
    
  public:
    static inline long millis() { return (unsigned long) (esp_timer_get_time() / 1000); }

    static void IRAM_ATTR delay_microseconds(uint32_t micro_seconds) {
      uint64_t m = esp_timer_get_time();
      if (micro_seconds > 2) {
        uint64_t e = m + micro_seconds;
        if (m > e) {
          while (esp_timer_get_time() > e) asm volatile ("nop"); // overflow...
        }
        while (esp_timer_get_time() < e) asm volatile ("nop");
      }
    }

    static void delay(uint32_t milliseconds) {
      vTaskDelay(milliseconds / portTICK_PERIOD_MS);
    
      uint32_t remainder_usec = (milliseconds % portTICK_PERIOD_MS) * 1000;
      if (remainder_usec) delay_microseconds(remainder_usec);
    }

    static int16_t analog_read(adc1_channel_t channel) {
      adc1_config_width(ADC_WIDTH_BIT_12);
      adc1_config_channel_atten(channel, ADC_ATTEN_11db);

      return adc1_get_raw(channel);
    }

    static void * ps_malloc(uint32_t size) {
      void * mem = nullptr; 
      if (heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM) > size) {
        mem = heap_caps_malloc(size, MALLOC_CAP_SPIRAM); 
      }
      if (mem == nullptr) {
        ESP_LOGE(TAG, "Not enough memory on PSRAM!!! (Asking %u bytes)", size);
      }
      return mem;
    }

    static void show_heaps_info(char * task_name) {
      ESP_LOGD(TAG, "%s +----- HEAPS/STACK DATA -----+", task_name);
      ESP_LOGD(TAG, "%s | Total heap:        %7d |",      task_name,    heap_caps_get_total_size(MALLOC_CAP_8BIT));
      ESP_LOGD(TAG, "%s | Free heap:         %7d |",      task_name,     heap_caps_get_free_size(MALLOC_CAP_8BIT));
      ESP_LOGD(TAG, "%s | Free stack:        %7d |",      task_name, uxTaskGetStackHighWaterMark(nullptr        ));
      ESP_LOGD(TAG, "%s +----------------------------+",  task_name);
    }
};
