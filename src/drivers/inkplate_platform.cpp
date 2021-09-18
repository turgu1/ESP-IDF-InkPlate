#define __INKPLATE_PLATFORM__ 1
#include "inkplate_platform.hpp"
#include "esp_log.h"

#include "wire.hpp"
#include "esp.hpp"
#include "sd_card.hpp"

#include "esp_sleep.h"

#include "driver/rtc_io.h"

InkPlatePlatform InkPlatePlatform::singleton;

bool
#if defined(INKPLATE_6PLUS)
  InkPlatePlatform::setup(bool sd_card_init, TouchScreen::ISRHandlerPtr touch_screen_handler)
#else
  InkPlatePlatform::setup(bool sd_card_init)
#endif
{
  wire.setup();

  // Setup the display
  if (!e_ink.setup()) return false;

  // Battery
  if (!battery.setup()) return false;
  
  #if defined(EXTENDED_CASE) && (defined(INKPLATE_6) || defined(INKPLATE_10))
    // Setup Press keys
    if (!press_keys.setup()) return false;
  #elif defined(INKPLATE_6) || defined(INKPLATE_10)
    // Setup Touch keys
    if (!touch_keys.setup()) return false;
  #elif defined(INKPLATE_6PLUS)
    if (!touch_screen.setup(true, e_ink.get_width(), e_ink.get_height(), touch_screen_handler)) return false;
    if (!front_light.setup()) return false;
  #endif

  // Mount and check the SD Card
  if (sd_card_init && !SDCard::setup()) return false;

  // Good to go
  return true;
}

/**
 * @brief Start light sleep
 * 
 * Will be waken up at the end of the time requested or when a key is pressed.
 * 
 * @param minutes_to_sleep Wait time in minutes
 * @return true The timer when through the end
 * @return false A key was pressed
 */
bool
InkPlatePlatform::light_sleep(uint32_t minutes_to_sleep, gpio_num_t gpio_num, int level)
{
  esp_err_t err;

  if ((err = esp_sleep_enable_timer_wakeup(minutes_to_sleep * 60e6)) != ESP_OK) {
    ESP_LOGE(TAG, "Unable to program Light Sleep wait time: %d", err);
  }
  else if ((gpio_num != ((gpio_num_t) 0)) && ((err = esp_sleep_enable_ext0_wakeup(gpio_num, level)) != ESP_OK)) {
    ESP_LOGE(TAG, "Unable to set ext0 WakeUp for Light Sleep: %d", err);
  } 
  else {
    light_sleep_engaged = true;
    if ((err = esp_light_sleep_start()) != ESP_OK) {
      ESP_LOGE(TAG, "Unable to start Light Sleep mode: %d", err);
    }
  }

  light_sleep_engaged = false;
  return esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER;
}

void 
InkPlatePlatform::deep_sleep(gpio_num_t gpio_num, int level)
{
  esp_err_t err;
  
  if (light_sleep_engaged && (((err = esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER)) != ESP_OK)) {
    if (err != ESP_ERR_INVALID_STATE) {
      ESP_LOGE(TAG, "Unable to disable Sleep wait time. Error: %d", err);
    }
  }

  light_sleep_engaged = false;
  
  if ((gpio_num != ((gpio_num_t) 0)) && ((err = esp_sleep_enable_ext0_wakeup(gpio_num, level)) != ESP_OK)) {
    ESP_LOGE(TAG, "Unable to set ext0 WakeUp for Deep Sleep. Error: %d", err);
  }
  
  rtc_gpio_isolate(GPIO_NUM_12);
  esp_deep_sleep_start();
}