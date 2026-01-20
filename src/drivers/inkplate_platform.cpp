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
#if INKPLATE_6PLUS || INKPLATE_6PLUS_V2 || INKPLATE_6FLICK
  InkPlatePlatform::setup(bool sd_card_init, TouchScreen::ISRHandlerPtr touch_screen_handler)
#else
  InkPlatePlatform::setup(bool sd_card_init)
#endif
{
  wire.setup();

  // Setup the display
  if (!e_ink.setup()) {
    ESP_LOGE(TAG, "EInk setup not completed!");
    return false;
  }

  // Battery
  if (!battery.setup()) {
    ESP_LOGE(TAG, "Battery setup not completed!");
    return false;
  }
  
  #if EXTENDED_CASE && (INKPLATE_6 || INKPLATE_10)
    // Setup Press keys
    if (!press_keys.setup()) {
      ESP_LOGE(TAG, "PressKeys setup not completed!");
      return false;
    }
  #elif INKPLATE_6 || INKPLATE_10
    // Setup Touch keys
    if (!touch_keys.setup()) {
      ESP_LOGE(TAG, "TouchKeys setup not completed!");
      return false;
    }
  #elif INKPLATE_6PLUS || INKPLATE_6PLUS_V2 || INKPLATE_6FLICK
    if (!touch_screen.setup(true, touch_screen_handler)) {
      ESP_LOGE(TAG, "TouchScreen setup not completed!");
      return false;
    }
    if (!front_light.setup()) {
      ESP_LOGE(TAG, "FrontLight setup not completed!");
      return false;
    }
  #endif

  if (!rtc.setup()) {
    ESP_LOGE(TAG, "RTC setup not completed!");
    return false;
  }

  // Mount and check the SD Card
  if (sd_card_init && !sd_card.setup()) {
    ESP_LOGE(TAG, "SDCard setup not completed!");
    return false;
  }

  // Good to go
  ESP_LOGI(TAG, "Inkplate Device Setup complete!");
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
    if ((err = esp_light_sleep_start()) != ESP_OK) {
      ESP_LOGE(TAG, "Unable to start Light Sleep mode: %d", err);
    }
  }

  bool result = esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER;

  if ((err = esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER)) != ESP_OK) {
    if (err != ESP_ERR_INVALID_STATE) {
      ESP_LOGE(TAG, "Unable to disable Sleep wait time. Error: %d", err);
    }
  }

  return result;
}

void 
InkPlatePlatform::deep_sleep(gpio_num_t gpio_num, int level)
{
  esp_err_t err;
  
  if (gpio_num != ((gpio_num_t) 0)) {
    if ((err = esp_sleep_enable_ext0_wakeup(gpio_num, level)) != ESP_OK) {
      ESP_LOGE(TAG, "Unable to set ext0 WakeUp for Deep Sleep. Error: %d", err);
    }
  }
  else {
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  }
  
  #if INKPLATE_6PLUS || INKPLATE_6PLUS_V2 || INKPLATE_6FLICK
    touch_screen.shutdown();
    front_light.disable();
  #endif
  
  sd_card.deepSleep();
  rtc_gpio_isolate(GPIO_NUM_12);

  esp_deep_sleep_start();
}