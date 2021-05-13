#define __INKPLATE_PLATFORM__ 1
#include "inkplate_platform.hpp"

#include "logging.hpp"

#include "wire.hpp"
#include "esp.hpp"
#include "sd_card.hpp"

#include "esp_sleep.h"

#include "driver/rtc_io.h"

InkPlatePlatform InkPlatePlatform::singleton;

bool
InkPlatePlatform::setup(bool sd_card_init)
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
    if (!touch_screen.setup(true, nullptr, e_ink.get_width(), e_ink.get_height())) return false;
    if (!back_light.setup()) return false;
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
InkPlatePlatform::light_sleep(uint32_t minutes_to_sleep)
{
  esp_err_t err;

  if ((err = esp_sleep_enable_timer_wakeup(minutes_to_sleep * 60e6)) != ESP_OK) {
    LOG_E("Unable to program Light Sleep wait time: %d", err);
  }
  else if ((err = esp_sleep_enable_ext0_wakeup(GPIO_NUM_34, 1)) != ESP_OK) {
    LOG_E("Unable to set ext0 WakeUp for Light Sleep: %d", err);
  } 
  else if ((err = esp_light_sleep_start()) != ESP_OK) {
    LOG_E("Unable to start Light Sleep mode: %d", err);
  }

  return esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER;
}

void 
InkPlatePlatform::deep_sleep()
{
  esp_err_t err;
  
  if ((err = esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER)) != ESP_OK) {
    LOG_E("Unable to disable Sleep wait time: %d", err);
  }
  if ((err = esp_sleep_enable_ext0_wakeup(GPIO_NUM_34, 1)) != ESP_OK) {
    LOG_E("Unable to set ext0 WakeUp for Deep Sleep: %d", err);
  }
  
  rtc_gpio_isolate(GPIO_NUM_12);
  esp_deep_sleep_start();
}