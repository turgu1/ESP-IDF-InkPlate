#pragma once

#if ((INKPLATE_6 || INKPLATE_6_V2 || INKPLATE_10_V2) && DMA_ENABLE) || INKPLATE_5_V2 || INKPLATE_6FLICK

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
  #include "soc/gpio_sig_map.h"
  #include "soc/i2s_reg.h"
  #include "soc/i2s_struct.h"
  #include "soc/periph_defs.h"
  #include "soc/rtc.h"
  #include "soc/soc.h"

  #include "esp_log.h"

  #include "esp.hpp"

  #if __I2S_COMMS__
    #define PUBLIC
  #else
    #define PUBLIC extern
  #endif

  class I2SComms {

    private:
      static constexpr char const *TAG = "I2SComms";

      i2s_dev_t *i2s_dev{ nullptr };
      const uint32_t line_buffer_size{ 0 };
      volatile uint8_t *line_buffer{ nullptr };
      volatile lldesc_s *lldesc{ nullptr };

      bool initialized{ false };
      bool ready{ false };

    public:
      I2SComms(const uint32_t buffer_size) : i2s_dev(&I2S1), line_buffer_size(buffer_size) {
        line_buffer = (uint8_t *)heap_caps_malloc(buffer_size, MALLOC_CAP_DMA);
        lldesc      = (lldesc_s *)heap_caps_malloc(sizeof(lldesc_s), MALLOC_CAP_DMA);

        ready = (line_buffer != nullptr) && (lldesc != nullptr);

        if (ready) {
          memset((void *)line_buffer, 0, buffer_size);
          ESP_LOGI(TAG, "Ready...");
        }
      }

      void IRAM_ATTR init(uint8_t clock_divider = 7);
      void IRAM_ATTR send_data();
      void IRAM_ATTR set_pin(uint32_t pin, uint32_t function, uint32_t inverted);
      void IRAM_ATTR setup_pins();

      void init_lldesc();

      volatile inline uint8_t *get_line_buffer() { return line_buffer; }
      inline uint32_t get_line_buffer_size() { return line_buffer_size; }

      inline bool is_ready() { return ready; }
      inline void stop_clock() { i2s_dev->conf1.tx_stop_en = 0; }

      inline void start_clock() {
        if (!initialized) {
          ESP_LOGE(TAG, "I2S not initialized!");
          return;
        }
        i2s_dev->conf1.tx_stop_en = 1;
        ESP::delay_microseconds(230);
      }

      #if 0
        // *INDENT-OFF*
      void show_clocks() {
        ESP_LOGI(TAG,                "I2S Clock values:"                                             );
        ESP_LOGI(TAG,                "conf1.tx_stop_en: %d", i2s_dev->conf1.tx_stop_en               );
        ESP_LOGI(TAG,           "int_raw.out_total_eof: %d", i2s_dev->int_raw.out_total_eof          );
        ESP_LOGI(TAG,                   "state.tx_idle: %d", i2s_dev->state.tx_idle                  );
        ESP_LOGI(TAG,            "sample_rate_conf.val: %d", i2s_dev->sample_rate_conf.val           );
        ESP_LOGI(TAG,    "sample_rate_conf.rx_bits_mod: %d", i2s_dev->sample_rate_conf.rx_bits_mod   );
        ESP_LOGI(TAG,    "sample_rate_conf.tx_bits_mod: %d", i2s_dev->sample_rate_conf.tx_bits_mod   );
        ESP_LOGI(TAG, "sample_rate_conf.rx_bck_div_num: %d", i2s_dev->sample_rate_conf.rx_bck_div_num);
        ESP_LOGI(TAG, "sample_rate_conf.tx_bck_div_num: %d", i2s_dev->sample_rate_conf.tx_bck_div_num);

        ESP_LOGI(TAG,                   "clkm_conf.val: %d", i2s_dev->clkm_conf.val                  );
        ESP_LOGI(TAG,               "clkm_conf.clka_en: %d", i2s_dev->clkm_conf.clka_en              );
        ESP_LOGI(TAG,            "clkm_conf.clkm_div_b: %d", i2s_dev->clkm_conf.clkm_div_b           );
        ESP_LOGI(TAG,            "clkm_conf.clkm_div_a: %d", i2s_dev->clkm_conf.clkm_div_a           );
        ESP_LOGI(TAG,          "clkm_conf.clkm_div_num: %d", i2s_dev->clkm_conf.clkm_div_num         );
        ESP_LOGI(TAG, "----");
      }
        // *INDENT-ON*
      #endif
  };

  #undef PUBLIC
#endif