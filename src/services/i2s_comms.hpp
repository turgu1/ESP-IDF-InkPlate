#pragma once

#if (INKPLATE_6 && DMA_ENABLE) || INKPLATE_5V2 || (INKPLATE_6V2 && DMA_ENABLE) || INKPLATE_6FLICK

// #include "driver/i2s_std.h"
// #include "driver/i2s_types.h"

// #include "soc/i2s_struct.h"
// #include "rom/lldesc.h"
// #include "soc/i2s_reg.h"
// #include "driver/gpio.h"

#include "esp_heap_caps.h"
#include "soc/gpio_struct.h"

// #include "driver/periph_ctrl.h"

// #include "driver/i2s.h"
#include "esp_private/periph_ctrl.h"
#include "rom/lldesc.h"
#include "soc/i2s_reg.h"
#include "soc/i2s_struct.h"
#include "soc/periph_defs.h"
#include "soc/rtc.h"
#include "soc/soc.h"

#include "esp_log.h"

#if __I2S_COMMS__
#define PUBLIC
#else
#define PUBLIC extern
#endif

PUBLIC void IRAM_ATTR my_I2SInit(i2s_dev_t *_i2sDev, uint8_t _clockDivider);
PUBLIC void IRAM_ATTR my_sendDataI2S(i2s_dev_t *_i2sDev, volatile lldesc_s *_dmaDecs);
PUBLIC void IRAM_ATTR my_setI2S1pin(uint32_t _pin, uint32_t _function, uint32_t _inv);

class I2SComms {

private:
public:
  I2SComms(const uint32_t buffer_size) : line_buffer_size(buffer_size) {
    line_buffer = (uint8_t *)heap_caps_malloc(buffer_size, MALLOC_CAP_DMA);
    lldesc      = (lldesc_s *)heap_caps_malloc(sizeof(lldesc_s), MALLOC_CAP_DMA);

    ready = (line_buffer != nullptr) && (lldesc != nullptr);

    if (ready) {
      memset((void *) line_buffer, 0, buffer_size);
      ESP_LOGI(TAG, "Ready...");
    }
  }

  inline void init(uint8_t clock_divider = 5) { my_I2SInit(&I2S1, clock_divider); }
  inline void send_data() { my_sendDataI2S(&I2S1, lldesc); }
  inline void set_pin(uint32_t pin, uint32_t function, uint32_t inverted) {
    my_setI2S1pin(pin, function, inverted);
  }

  void init_lldesc();

  volatile inline uint8_t *get_line_buffer() { return line_buffer; }
  inline uint32_t get_line_buffer_size() { return line_buffer_size; }

  inline bool is_ready() { return ready; }
  inline void stop_clock() { I2S1.conf1.tx_stop_en = 0; }
  inline void start_clock() { I2S1.conf1.tx_stop_en = 1; }

  void show_clocks() {
    ESP_LOGI(TAG, "I2S Clock values:");
    ESP_LOGI(TAG, "sample_rate_conf.val: %d",            I2S1.sample_rate_conf.val           );
    ESP_LOGI(TAG, "sample_rate_conf.rx_bits_mod: %d",    I2S1.sample_rate_conf.rx_bits_mod   );
    ESP_LOGI(TAG, "sample_rate_conf.tx_bits_mod: %d",    I2S1.sample_rate_conf.tx_bits_mod   );
    ESP_LOGI(TAG, "sample_rate_conf.rx_bck_div_num: %d", I2S1.sample_rate_conf.rx_bck_div_num);
    ESP_LOGI(TAG, "sample_rate_conf.tx_bck_div_num: %d", I2S1.sample_rate_conf.tx_bck_div_num);

    ESP_LOGI(TAG, "clkm_conf.val: %d",          I2S1.clkm_conf.val          );
    ESP_LOGI(TAG, "clkm_conf.clka_en: %d",      I2S1.clkm_conf.clka_en      );
    ESP_LOGI(TAG, "clkm_conf.clkm_div_b: %d",   I2S1.clkm_conf.clkm_div_b   );
    ESP_LOGI(TAG, "clkm_conf.clkm_div_a: %d",   I2S1.clkm_conf.clkm_div_a   );
    ESP_LOGI(TAG, "clkm_conf.clkm_div_num: %d", I2S1.clkm_conf.clkm_div_num );
    ESP_LOGI(TAG, "----");
  }
  
private:
  static constexpr char const *TAG = "I2SComms";

  volatile lldesc_s *lldesc;
  volatile uint8_t *line_buffer{nullptr};

  const uint32_t line_buffer_size{0};
  bool ready{false};
};

#undef PUBLIC
#endif