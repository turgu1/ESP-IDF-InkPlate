#pragma once

#if INKPLATE_6FLICK

  #include "driver/i2s_std.h"
  #include "driver/i2s_types.h"

  #include "soc/i2s_struct.h"
  #include "rom/lldesc.h"
  #include "soc/i2s_reg.h"
  #include "driver/gpio.h"
  #include "esp_heap_caps.h"
  #include "soc/gpio_struct.h"
  #include "esp_log.h"

  class I2SComms {

  public:

    I2SComms(i2s_dev_t & dev, const uint32_t buffer_size) 
      : i2s_dev(dev), line_buffer_size(buffer_size) 
    {
      line_buffer = (uint8_t  *) heap_caps_malloc(buffer_size,      MALLOC_CAP_DMA);
      lldesc      = (lldesc_s *) heap_caps_malloc(sizeof(lldesc_t), MALLOC_CAP_DMA);

      ready = (line_buffer != nullptr) && (lldesc != nullptr);

      if (ready) {
        if  (((uint32_t)lldesc) != (((uint32_t)lldesc) & 0x000FFFFF)) {
          ESP_LOGE(TAG, "Hum... lldesc address is not dma compliant!?");
          ready = false;
        }
        if (((uint32_t) line_buffer) != (((uint32_t) line_buffer) & 0x000FFFFF)) {
          ESP_LOGE(TAG, "Hum... line_buffer address is not dma compliant!?");
          ready = false;
        }
      }

      if (ready) {
        ESP_LOGI(TAG, "Ready...");
      }
    }

    void init(uint8_t clock_divider);
    void send_data();
    void set_pin(uint32_t pin, uint32_t function, uint32_t inverted);
    void init_lldesc();

    inline bool      is_ready() { return ready; }
    inline uint8_t * get_line_buffer() { return line_buffer; }
    inline void      stop_clock() { i2s_dev.conf1.tx_stop_en = 0; }
    inline void      start_clock() { i2s_dev.conf1.tx_stop_en = 1; }

  private:
    static constexpr char const * TAG = "I2SComms";

    volatile i2s_dev_t & i2s_dev;
    volatile lldesc_s * lldesc;
    uint8_t * line_buffer{nullptr};

    const uint32_t line_buffer_size{0};
    bool ready{false};

    // Repeating here definitions that are in eink.hpp 
    // to get rid of potential circular includes, as the
    // EInk class requires an instance of I2SComms...

    static const uint32_t CKV  = 0x01;
    static const uint32_t SPH  = 0x02;

    inline void ckv_set()      { GPIO.out1_w1ts.val = CKV; }
    inline void ckv_clear()    { GPIO.out1_w1tc.val = CKV; }

    inline void sph_set()      { GPIO.out1_w1ts.val = SPH; }
    inline void sph_clear()    { GPIO.out1_w1tc.val = SPH; }
  };

#endif