#define __SD_CARD__ 1
#include "sd_card.hpp"
#include "esp_log.h"

#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "driver/spi_master.h"
#include "sdmmc_cmd.h"

SDCard::SDCardState SDCard::state = SDCardState::UNINITIALIZED;

bool
SDCard::setup()
{
  switch (state)
  {
  case SDCardState::INITIALIZED:
    ESP_LOGI(TAG, "SD card is already initialized");
    return true;
  case SDCardState::FAILED:
    ESP_LOGI(TAG, "SD card setup is recently failed");
    return false;
  case SDCardState::UNINITIALIZED:
    ESP_LOGI(TAG, "Setup SD card");
  }

  static const gpio_num_t PIN_NUM_MISO = GPIO_NUM_12;
  static const gpio_num_t PIN_NUM_MOSI = GPIO_NUM_13;
  static const gpio_num_t PIN_NUM_CLK  = GPIO_NUM_14;
  static const gpio_num_t PIN_NUM_CS   = GPIO_NUM_15;

  sdmmc_host_t        host          = SDSPI_HOST_DEFAULT();
  sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();

  host.flags = SDMMC_HOST_FLAG_SPI;

  slot_config.gpio_cs = PIN_NUM_CS;

  spi_bus_config_t bus_config = {};
  bus_config.mosi_io_num = PIN_NUM_MOSI;
  bus_config.miso_io_num = PIN_NUM_MISO;
  bus_config.sclk_io_num = PIN_NUM_CLK;

  ESP_ERROR_CHECK(spi_bus_initialize(SPI_HOST, &bus_config, SPI_DMA_CH_AUTO));
  ESP_ERROR_CHECK(sdspi_host_init_device(&slot_config, nullptr));

  esp_vfs_fat_sdmmc_mount_config_t mount_config = {};
  mount_config.format_if_mount_failed = false;
  mount_config.max_files = 5;
  mount_config.allocation_unit_size = 16 * 1024;

  state = SDCardState::FAILED;

  sdmmc_card_t* card;
  esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      ESP_LOGE(TAG, "Failed to mount filesystem.");
    } else {
      ESP_LOGE(TAG, "Failed to setup the SD card (%s).", esp_err_to_name(ret));
    }
    return false;
  }

  // Card has been initialized, print its properties
  sdmmc_card_print_info(stdout, card);

  state = SDCardState::INITIALIZED;

  return true;
}