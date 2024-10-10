#define __SD_CARD__ 1
#include "sd_card.hpp"
#include "esp_log.h"
#include "wire.hpp"

#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

bool
SDCard::setup()
{
  switch (state) {
    case SDCardState::INITIALIZED:
      ESP_LOGI(TAG, "SD card is already initialized");
      return true;
    case SDCardState::FAILED:
      ESP_LOGI(TAG, "SD card setup has recently failed");
      return false;
    case SDCardState::UNINITIALIZED:
      ESP_LOGI(TAG, "Setup SD card");
  }

 #if INKPLATE_6PLUS_V2 || INKPLATE_6FLICK
    Wire::enter();
    io_expander.set_direction(SD_POWER, IOExpander::PinMode::OUTPUT);
    io_expander.digital_write(SD_POWER, IOExpander::SignalLevel::LOW);
    ESP::delay(50);
    Wire::leave();
  #endif

// The original SDSPI_HOST_DEFAULT() from 5.3 is lacking some entries that make it
// not userfriendly to C++

#define MY_SDSPI_HOST_DEFAULT() {\
    .flags = SDMMC_HOST_FLAG_SPI | SDMMC_HOST_FLAG_DEINIT_ARG, \
    .slot = SDSPI_DEFAULT_HOST, \
    .max_freq_khz = SDMMC_FREQ_DEFAULT, \
    .io_voltage = 3.3f, \
    .init = &sdspi_host_init, \
    .set_bus_width = NULL, \
    .get_bus_width = NULL, \
    .set_bus_ddr_mode = NULL, \
    .set_card_clk = &sdspi_host_set_card_clk, \
    .set_cclk_always_on = NULL, \
    .do_transaction = &sdspi_host_do_transaction, \
    .deinit_p = &sdspi_host_remove_device, \
    .io_int_enable = &sdspi_host_io_int_enable, \
    .io_int_wait = &sdspi_host_io_int_wait, \
    .command_timeout_ms = 0, \
    .get_real_freq = &sdspi_host_get_real_freq, \
    .input_delay_phase = SDMMC_DELAY_PHASE_0, \
    .set_input_delay = NULL, \
    .dma_aligned_buffer = NULL, \
    .pwr_ctrl_handle = NULL, \
    .get_dma_info = &sdspi_host_get_dma_info, \
}

  sdmmc_host_t        host        = MY_SDSPI_HOST_DEFAULT();

  sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot_config.gpio_cs = PIN_NUM_CS;
  slot_config.host_id = HSPI_HOST;

  spi_bus_config_t bus_cfg = {
      .mosi_io_num = PIN_NUM_MOSI,
      .miso_io_num = PIN_NUM_MISO,
      .sclk_io_num = PIN_NUM_CLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .data4_io_num = -1,
      .data5_io_num = -1,
      .data6_io_num = -1,
      .data7_io_num = -1,
      .max_transfer_sz = 4000,
      .flags = SPICOMMON_BUSFLAG_MASTER,
      .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
      .intr_flags = 0,
  };

  state = SDCardState::FAILED;

  gpio_dump_io_configuration(stdout, (1ULL << PIN_NUM_MISO) | (1ULL << PIN_NUM_MOSI));

  esp_err_t ret = spi_bus_initialize(HSPI_HOST, &bus_cfg, SDSPI_DEFAULT_DMA);

  gpio_set_pull_mode(PIN_NUM_MISO, GPIO_FLOATING);

  gpio_dump_io_configuration(stdout, (1ULL << PIN_NUM_MISO) | (1ULL << PIN_NUM_MOSI));

  if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to initialize SPI bus for SD Card.");
      return false;
  }

  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    .format_if_mount_failed = false,
    .max_files = 5,
    .allocation_unit_size = 16 * 1024,
    .disk_status_check_enable = true,
    .use_one_fat = false,
  };

  ret = esp_vfs_fat_sdspi_mount("/sdcard", &host, &slot_config, &mount_config, &card);

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

void SDCard::deepSleep() {
  // Set SPI pins to input to reduce power consumption in deep sleep
  gpio_set_direction(PIN_NUM_MISO,  GPIO_MODE_INPUT);
  gpio_set_direction(PIN_NUM_MOSI,  GPIO_MODE_INPUT);
  gpio_set_direction(PIN_NUM_CLK,   GPIO_MODE_INPUT);
  gpio_set_direction(PIN_NUM_CS,    GPIO_MODE_INPUT);

  #if INKPLATE_6PLUS_V2 || INKPLATE_6FLICK
    Wire::enter();
    io_expander.digital_write(SD_POWER, IOExpander::SignalLevel::HIGH);
    ESP::delay(50);
    io_expander.set_direction(SD_POWER, IOExpander::PinMode::INPUT);
    Wire::leave();
  #endif
} 
