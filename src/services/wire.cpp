
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

    initialized = true; 
  }
}
