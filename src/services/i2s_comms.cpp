#if INKPLATE_6FLICK

  #include "i2s_comms.hpp"
  #include "soc/i2s_periph.h"
  #include "soc/io_mux_reg.h"
  #include "soc/gpio_struct.h"
  #include "esp_private/periph_ctrl.h"

  /**
   * @brief       Function Intializes I2S driver of the ESP32
   *
   * @param       i2s_dev_t *i2s_dev
   *              Pointer of the selected I2S driver
   *
   * @note        Function must be declared static to fit into Instruction RAM of the ESP32.
   */
  void IRAM_ATTR I2SComms::init(uint8_t clock_divider)
  {
    // Enable I2S peripheral and reset it.
    periph_module_enable(PERIPH_I2S1_MODULE);
    periph_module_reset(PERIPH_I2S1_MODULE);

    // Reset the FIFO Buffer in I2S module.
    i2s_dev.conf.rx_fifo_reset = 1;
    i2s_dev.conf.rx_fifo_reset = 0;
    i2s_dev.conf.tx_fifo_reset = 1;
    i2s_dev.conf.tx_fifo_reset = 0;

    // Reset I2S DMA controller.
    i2s_dev.lc_conf.in_rst = 1;
    i2s_dev.lc_conf.in_rst = 0;
    i2s_dev.lc_conf.out_rst = 1;
    i2s_dev.lc_conf.out_rst = 0;

    // Reset I2S TX and RX module.
    i2s_dev.conf.rx_reset = 1;
    i2s_dev.conf.tx_reset = 1;
    i2s_dev.conf.rx_reset = 0;
    i2s_dev.conf.tx_reset = 0;

    // Set LCD mode on I2S, setup delays on SD and WR lines.
    i2s_dev.conf2.val = 0;
    i2s_dev.conf2.lcd_en = 1;
    i2s_dev.conf2.lcd_tx_wrx2_en = 1;
    i2s_dev.conf2.lcd_tx_sdx2_en = 0;

    i2s_dev.sample_rate_conf.val = 0;
    i2s_dev.sample_rate_conf.rx_bits_mod = 8;
    i2s_dev.sample_rate_conf.tx_bits_mod = 8;
    i2s_dev.sample_rate_conf.rx_bck_div_num = 2;
    i2s_dev.sample_rate_conf.tx_bck_div_num = 2;

    // Do not use APLL, divide by 5 by default, BCK should be ~16MHz.
    i2s_dev.clkm_conf.val = 0;
    i2s_dev.clkm_conf.clka_en = 0;
    i2s_dev.clkm_conf.clkm_div_b = 0;
    i2s_dev.clkm_conf.clkm_div_a = 1;
    i2s_dev.clkm_conf.clkm_div_num = clock_divider;

    // FIFO buffer setup. Byte packing for FIFO: 0A0B_0B0C = 0, 0A0B_0C0D = 1, 0A00_0B00 = 3. Use dual mono single data
    i2s_dev.fifo_conf.val = 0;
    i2s_dev.fifo_conf.rx_fifo_mod_force_en = 1;
    i2s_dev.fifo_conf.tx_fifo_mod_force_en = 1;
    i2s_dev.fifo_conf.tx_fifo_mod =
        1; // byte packing 0A0B_0B0C = 0, 0A0B_0C0D = 1, 0A00_0B00 = 3. Use dual mono single data
    i2s_dev.fifo_conf.rx_data_num = 1;
    i2s_dev.fifo_conf.tx_data_num = 1;
    i2s_dev.fifo_conf.dscr_en = 1;

    // Send BCK only when needed (needs to be powered on in einkOn() function and disabled in einkOff()).
    i2s_dev.conf1.val = 0;
    i2s_dev.conf1.tx_stop_en = 0;
    i2s_dev.conf1.tx_pcm_bypass = 1;

    i2s_dev.conf_chan.val = 0;
    i2s_dev.conf_chan.tx_chan_mod = 1;
    i2s_dev.conf_chan.rx_chan_mod = 1;

    i2s_dev.conf.tx_right_first = 0; //!!invert_clk; // should be false / 0
    i2s_dev.conf.rx_right_first = 0; //!!invert_clk;

    i2s_dev.timing.val = 0;
  }

  /**
   * @brief       Function sends data with I2S DMA driver.
   *
   * @note        Function must be declared static to fit into Instruction RAM of the ESP32. Also, DMA descriptor must be
   *              already configured!
   */
  void IRAM_ATTR I2SComms::send_data()
  {
    // Stop any on-going transmission (just in case).
    i2s_dev.out_link.stop = 1;
    i2s_dev.out_link.start = 0;
    i2s_dev.conf.tx_start = 0;

    // Reset the FIFO.
    i2s_dev.conf.tx_fifo_reset = 1;
    i2s_dev.conf.tx_fifo_reset = 0;

    // Reset the I2S DMA Controller.
    i2s_dev.lc_conf.out_rst = 1;
    i2s_dev.lc_conf.out_rst = 0;

    // Reset I2S TX module.
    i2s_dev.conf.tx_reset = 1;
    i2s_dev.conf.tx_reset = 0;

    // Setup a DMA descriptor.
    i2s_dev.lc_conf.val = I2S_OUT_DATA_BURST_EN | I2S_OUTDSCR_BURST_EN;
    i2s_dev.out_link.addr = (uint32_t)(lldesc) & 0x000FFFFF;

    // Start sending the data
    i2s_dev.out_link.start = 1;

    // Pull SPH low -> Start pushing data into the row of EPD.
    sph_clear();

    // Set CKV to HIGH.
    ckv_set();

    // Start sending I2S data out.
    i2s_dev.conf.tx_start = 1;

    while (!i2s_dev.int_raw.out_total_eof)
        ;

    sph_set();

    // Clear the interrupt flags and stop the transmission.
    i2s_dev.int_clr.val = i2s_dev.int_raw.val;
    i2s_dev.out_link.stop = 1;
    i2s_dev.out_link.start = 0;
  }

  void IRAM_ATTR I2SComms::set_pin(uint32_t pin, uint32_t function, uint32_t inverted)
  {
    // Check if valid pin is selected
    if (pin > 39) return;

    // Fast GPIO pin to MUX (maybe there is a better way to do this?).
    const uint32_t io_mux[] = {
      IO_MUX_GPIO0_REG,
      IO_MUX_GPIO1_REG,
      IO_MUX_GPIO2_REG,
      IO_MUX_GPIO3_REG,
      IO_MUX_GPIO4_REG,
      IO_MUX_GPIO5_REG,
      IO_MUX_GPIO6_REG,
      IO_MUX_GPIO7_REG,
      IO_MUX_GPIO8_REG,
      IO_MUX_GPIO9_REG,
      IO_MUX_GPIO10_REG,
      IO_MUX_GPIO11_REG,
      IO_MUX_GPIO12_REG,
      IO_MUX_GPIO13_REG,
      IO_MUX_GPIO14_REG,
      IO_MUX_GPIO15_REG,
      IO_MUX_GPIO16_REG,
      IO_MUX_GPIO17_REG,
      IO_MUX_GPIO18_REG,
      IO_MUX_GPIO19_REG,
      IO_MUX_GPIO20_REG,
      IO_MUX_GPIO21_REG,
      IO_MUX_GPIO22_REG,
      IO_MUX_GPIO23_REG,
      IO_MUX_GPIO24_REG,
      IO_MUX_GPIO25_REG,
      IO_MUX_GPIO26_REG,
      IO_MUX_GPIO27_REG,
      0,
      0,
      0,
      0,
      IO_MUX_GPIO32_REG,
      IO_MUX_GPIO33_REG,
      IO_MUX_GPIO34_REG,
      IO_MUX_GPIO35_REG,
      IO_MUX_GPIO36_REG,
      IO_MUX_GPIO37_REG,
      IO_MUX_GPIO38_REG,
      IO_MUX_GPIO39_REG
    };

    // Wrong pin selected? Return!
    if (io_mux[pin] == 0)
        return;

    // Setup GPIO Matrix for selected pin signal
    GPIO.func_out_sel_cfg[pin].func_sel = function; // Set the pin function
    GPIO.func_out_sel_cfg[pin].inv_sel = inverted;  // Does pin logic needs to be inverted?
    GPIO.func_out_sel_cfg[pin].oen_sel = 0;         // Force output enable if bit is set

    // Registers are different for GPIOs from 0 to 32 and from 32 to 40.
    if (pin < 32)
    {
        // Enable GPIO pin (set it as output).
        GPIO.enable_w1ts = ((uint32_t)1 << pin);
    }
    else
    {
        // Enable GPIO pin (set it as output).
        GPIO.enable1_w1ts.data = ((uint32_t)1 << (32 - pin));
    }

    // Set the highest drive strength.
    #define ESP_REG(addr) *((volatile uint32_t *)(addr))

    ESP_REG(io_mux[pin]) = 0;
    ESP_REG(io_mux[pin]) = (FUN_DRV_M | MCU_SEL_M);
  }

  void IRAM_ATTR I2SComms::init_lldesc()
  {
    if (ready) {
      lldesc->size = line_buffer_size;
      lldesc->length = line_buffer_size;
      lldesc->offset = 0;
      lldesc->sosf = 1;
      lldesc->eof = 1;
      lldesc->owner = 1;
      lldesc->buf = line_buffer;
      lldesc->empty = 0;
    }
  }

#endif