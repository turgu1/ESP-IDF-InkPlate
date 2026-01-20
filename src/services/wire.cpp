
// Copyright (c) 2020 Guy Turcotte
//
// MIT License. Look at file licenses.txt for details.

#define __WIRE__ 1
#include "wire.hpp"

#include <cstring>

Wire Wire::singleton;
SemaphoreHandle_t Wire::mutex = nullptr;
StaticSemaphore_t Wire::mutex_buffer;

void
Wire::setup()
{
  if (!initialized) {

    ESP_LOGD(TAG, "Initializing...");

    mutex = xSemaphoreCreateMutexStatic(&mutex_buffer);

    #if I2C_LEGACY_BASED
      i2c_config_t config;

      memset(&config, 0, sizeof(i2c_config_t));

      config.mode             = I2C_MODE_MASTER;
      config.scl_io_num       = GPIO_NUM_22;
      config.scl_pullup_en    = GPIO_PULLUP_DISABLE;
      config.sda_io_num       = GPIO_NUM_21;
      config.sda_pullup_en    = GPIO_PULLUP_DISABLE;
      config.master.clk_speed = 1E5;

      ESP_ERROR_CHECK(  i2c_param_config(I2C_NUM_0, &config));
      ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
    #else
      i2c_master_bus_config_t i2c_mst_config;

      i2c_mst_config.clk_source                   = I2C_CLK_SRC_DEFAULT;
      i2c_mst_config.i2c_port                     = 0;
      i2c_mst_config.scl_io_num                   = GPIO_NUM_22;
      i2c_mst_config.sda_io_num                   = GPIO_NUM_21;
      i2c_mst_config.glitch_ignore_cnt            = 7;
      i2c_mst_config.flags.enable_internal_pullup = false;
      i2c_mst_config.intr_priority                = 0;
      i2c_mst_config.trans_queue_depth            = 0;

      ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &master_bus_handle));
    #endif

    initialized = true; 
  }
}

#if I2C_LEGACY_BASED && !TRANSITION_TO_NEW_API_COMPLETED

void   
Wire::begin_transmission(uint8_t addr)
{
  if (!initialized) setup();

  ESP_LOGD(TAG, "Begin Transmission to address %x", addr);

  if (initialized) {
    address = addr;
    index   = 0;
  }
}

esp_err_t
Wire::end_transmission()
{
  esp_err_t result;

  if (initialized) {
    //ESP_LOGD(TAG, "Writing %d bytes to i2c at address 0x%02x.", index, address);

    // for (int i = 0; i < index; i++) {
    //   printf("%02x ", buffer[i]);
    // }
    // printf("\n");
    // fflush(stdout);

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, 1));
    if (index > 0) ESP_ERROR_CHECK(i2c_master_write(cmd, buffer, index, 1));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    result = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    index = 0;

    return result;
  }
  else {
    return ESP_ERR_INVALID_STATE;
  }
  
  // ESP_LOGD(TAG, "I2C Transmission completed.");
}

void    
Wire::write_byte(uint8_t val)
{
  if (initialized) {    
    buffer[index++] = val;
    if (index >= BUFFER_LENGTH) index = BUFFER_LENGTH - 1;
  }
}

uint8_t 
Wire::read_byte()
{
  if (!initialized || (index >= size_to_read)) return 0;
  return buffer[index++];
}

esp_err_t    
Wire::request_from(uint8_t addr, uint8_t size)
{
  if (!initialized) setup();

  if (initialized) {
    if (size == 0) return ESP_OK;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true));

    if (size > 1) ESP_ERROR_CHECK(i2c_master_read(cmd, buffer, size - 1, I2C_MASTER_ACK));
    ESP_ERROR_CHECK(i2c_master_read_byte(cmd, buffer + size - 1, I2C_MASTER_LAST_NACK));

    ESP_ERROR_CHECK(i2c_master_stop(cmd));

    esp_err_t result = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    size_to_read = size;
    index = 0;

    if (result != ESP_OK) {
      ESP_LOGE(TAG, "Unable to complete request_from: %s.", esp_err_to_name(result));
    }
    return result;
  }
  else {
    return ESP_ERR_INVALID_STATE;
  }
}
#endif