#define __EINK__
#include "eink.hpp"

// PIN_LUT built from the following:
//
// for (uint32_t i = 0; i < 256; i++) {
//   PIN_LUT[i] =  ((i & 0b00000011) << 4)        | 
//                (((i & 0b00001100) >> 2) << 18) | 
//                (((i & 0b00010000) >> 4) << 23) |
//                (((i & 0b11100000) >> 5) << 25);
// }

const uint32_t EInk::PIN_LUT[256] = {
  0x00000000, 0x00000010, 0x00000020, 0x00000030, 0x00040000, 0x00040010, 0x00040020, 0x00040030, 
  0x00080000, 0x00080010, 0x00080020, 0x00080030, 0x000c0000, 0x000c0010, 0x000c0020, 0x000c0030, 
  0x00800000, 0x00800010, 0x00800020, 0x00800030, 0x00840000, 0x00840010, 0x00840020, 0x00840030, 
  0x00880000, 0x00880010, 0x00880020, 0x00880030, 0x008c0000, 0x008c0010, 0x008c0020, 0x008c0030, 
  0x02000000, 0x02000010, 0x02000020, 0x02000030, 0x02040000, 0x02040010, 0x02040020, 0x02040030, 
  0x02080000, 0x02080010, 0x02080020, 0x02080030, 0x020c0000, 0x020c0010, 0x020c0020, 0x020c0030, 
  0x02800000, 0x02800010, 0x02800020, 0x02800030, 0x02840000, 0x02840010, 0x02840020, 0x02840030, 
  0x02880000, 0x02880010, 0x02880020, 0x02880030, 0x028c0000, 0x028c0010, 0x028c0020, 0x028c0030, 
  0x04000000, 0x04000010, 0x04000020, 0x04000030, 0x04040000, 0x04040010, 0x04040020, 0x04040030, 
  0x04080000, 0x04080010, 0x04080020, 0x04080030, 0x040c0000, 0x040c0010, 0x040c0020, 0x040c0030, 
  0x04800000, 0x04800010, 0x04800020, 0x04800030, 0x04840000, 0x04840010, 0x04840020, 0x04840030, 
  0x04880000, 0x04880010, 0x04880020, 0x04880030, 0x048c0000, 0x048c0010, 0x048c0020, 0x048c0030, 
  0x06000000, 0x06000010, 0x06000020, 0x06000030, 0x06040000, 0x06040010, 0x06040020, 0x06040030, 
  0x06080000, 0x06080010, 0x06080020, 0x06080030, 0x060c0000, 0x060c0010, 0x060c0020, 0x060c0030, 
  0x06800000, 0x06800010, 0x06800020, 0x06800030, 0x06840000, 0x06840010, 0x06840020, 0x06840030, 
  0x06880000, 0x06880010, 0x06880020, 0x06880030, 0x068c0000, 0x068c0010, 0x068c0020, 0x068c0030, 
  0x08000000, 0x08000010, 0x08000020, 0x08000030, 0x08040000, 0x08040010, 0x08040020, 0x08040030, 
  0x08080000, 0x08080010, 0x08080020, 0x08080030, 0x080c0000, 0x080c0010, 0x080c0020, 0x080c0030, 
  0x08800000, 0x08800010, 0x08800020, 0x08800030, 0x08840000, 0x08840010, 0x08840020, 0x08840030, 
  0x08880000, 0x08880010, 0x08880020, 0x08880030, 0x088c0000, 0x088c0010, 0x088c0020, 0x088c0030, 
  0x0a000000, 0x0a000010, 0x0a000020, 0x0a000030, 0x0a040000, 0x0a040010, 0x0a040020, 0x0a040030, 
  0x0a080000, 0x0a080010, 0x0a080020, 0x0a080030, 0x0a0c0000, 0x0a0c0010, 0x0a0c0020, 0x0a0c0030, 
  0x0a800000, 0x0a800010, 0x0a800020, 0x0a800030, 0x0a840000, 0x0a840010, 0x0a840020, 0x0a840030, 
  0x0a880000, 0x0a880010, 0x0a880020, 0x0a880030, 0x0a8c0000, 0x0a8c0010, 0x0a8c0020, 0x0a8c0030, 
  0x0c000000, 0x0c000010, 0x0c000020, 0x0c000030, 0x0c040000, 0x0c040010, 0x0c040020, 0x0c040030, 
  0x0c080000, 0x0c080010, 0x0c080020, 0x0c080030, 0x0c0c0000, 0x0c0c0010, 0x0c0c0020, 0x0c0c0030, 
  0x0c800000, 0x0c800010, 0x0c800020, 0x0c800030, 0x0c840000, 0x0c840010, 0x0c840020, 0x0c840030, 
  0x0c880000, 0x0c880010, 0x0c880020, 0x0c880030, 0x0c8c0000, 0x0c8c0010, 0x0c8c0020, 0x0c8c0030, 
  0x0e000000, 0x0e000010, 0x0e000020, 0x0e000030, 0x0e040000, 0x0e040010, 0x0e040020, 0x0e040030, 
  0x0e080000, 0x0e080010, 0x0e080020, 0x0e080030, 0x0e0c0000, 0x0e0c0010, 0x0e0c0020, 0x0e0c0030, 
  0x0e800000, 0x0e800010, 0x0e800020, 0x0e800030, 0x0e840000, 0x0e840010, 0x0e840020, 0x0e840030, 
  0x0e880000, 0x0e880010, 0x0e880020, 0x0e880030, 0x0e8c0000, 0x0e8c0010, 0x0e8c0020, 0x0e8c0030
};

// Turn off epaper power supply and put all digital IO pins in high Z state
void 
EInk::turn_off()
{
  if (get_panel_state() == PanelState::OFF) return;
 
  oe_clear();
  gmod_clear();

  #if !(INKPLATE_6 || INKPLATE_6V2 || INKPLATE_6FLICK)
    GPIO.out &= ~(DATA | LE | CL);
  #else
    le_clear();
  #endif
  
  ckv_clear();
  sph_clear();
  spv_clear();

  vcom_clear();
  pwrup_clear();

  unsigned long timer = ESP::millis();
  do {
    ESP::delay(1);
  } while ((read_power_good() != 0) && (ESP::millis() - timer) < 250);

  // Do not disable WAKEUP if older Inkplate6Plus is used.
  #if !INKPLATE_6PLUS
    wakeup_clear();
  #endif

  pins_z_state();
  set_panel_state(PanelState::OFF);

  ESP_LOGD(TAG, "EInk is off");
}

// Turn on supply for epaper display (TPS65186) 
// [+15 VDC, -15VDC, +22VDC, -20VDC, +3.3VDC, VCOM]
bool 
EInk::turn_on()
{
  if (get_panel_state() == PanelState::ON) return true;

  wakeup_set();

  ESP::delay(5);

  // Modify power up sequence  (VEE and VNEG are swapped)
  wire.begin_transmission(PWRMGR_ADDRESS);
  wire.write(0x09);
  wire.write(0b11100001);
  wire.end_transmission();

  // Enable all rails
  wire.begin_transmission(PWRMGR_ADDRESS);
  wire.write(0x01);
  wire.write(0b00111111);
  wire.end_transmission();

  // // Modify power down sequence (VEE and VNEG are swapped)
  // wire.begin_transmission(PWRMGR_ADDRESS);
  // wire.write(0x0B);
  // wire.write(0b00011011);
  // wire.end_transmission();

  pwrup_set();

  pins_as_outputs();

  le_clear();
  oe_clear();
  
  #if !(INKPLATE_6 || INKPLATE_6V2 || INKPLATE_6FLICK)
    cl_clear();
  #endif

  sph_set();
  gmod_set();
  spv_set();
  ckv_clear();
  oe_clear();
  vcom_set();

  unsigned long timer = ESP::millis();
  do {
    ESP::delay(1);
  } while ((read_power_good() != PWR_GOOD_OK) && (ESP::millis() - timer) < 250);

  if ((ESP::millis() - timer) >= 250) {
    vcom_clear();
    pwrup_clear();
    return false;
  }

  oe_set();
  set_panel_state(PanelState::ON);

  ESP_LOGD(TAG, "EInk is on");
  return true;
}

uint8_t 
EInk::read_power_good()
{
  wire.begin_transmission(PWRMGR_ADDRESS);
  wire.write(0x0F);
  wire.end_transmission();

  wire.request_from(PWRMGR_ADDRESS, 1);
  return wire.read();
}

// LOW LEVEL FUNCTIONS

void 
EInk::vscan_start()
{
  ckv_set();   ESP::delay_microseconds( 7);
  spv_clear(); ESP::delay_microseconds(10);
  ckv_clear(); ESP::delay_microseconds( 0);
  ckv_set();   ESP::delay_microseconds( 8);
  spv_set();   ESP::delay_microseconds(10);
  ckv_clear(); ESP::delay_microseconds( 0);
  ckv_set();   ESP::delay_microseconds(18);
  ckv_clear(); ESP::delay_microseconds( 0);
  ckv_set();   ESP::delay_microseconds(18);
  ckv_clear(); ESP::delay_microseconds( 0);
  ckv_set();
}

void 
EInk::hscan_start(uint32_t d)
{
  sph_clear();
  GPIO.out_w1ts = CL | d   ;
  GPIO.out_w1tc = CL | DATA;
  sph_set();
  ckv_set();
}

void 
EInk::vscan_end()
{
  ckv_clear();
     le_set();
   le_clear();

  ESP::delay_microseconds(0);
}

void 
EInk::pins_z_state()
{
  gpio_set_direction(GPIO_NUM_2,  GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_32, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_33, GPIO_MODE_INPUT);

  io_expander_int.set_direction(OE,   IOExpander::PinMode::INPUT);
  io_expander_int.set_direction(GMOD, IOExpander::PinMode::INPUT);
  io_expander_int.set_direction(SPV,  IOExpander::PinMode::INPUT);

  gpio_set_direction(GPIO_NUM_0,  GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_4,  GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_5,  GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_18, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_19, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_23, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_25, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_26, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_27, GPIO_MODE_INPUT);

  #if INKPLATE_6 || INKPLATE_6V2 || INKPLATE_6FLICK
    i2s_comms.stop_clock();
  #endif
}

void 
EInk::pins_as_outputs()
{
  gpio_set_direction(GPIO_NUM_2,  GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_32, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_33, GPIO_MODE_OUTPUT);

  io_expander_int.set_direction(OE,   IOExpander::PinMode::OUTPUT);
  io_expander_int.set_direction(GMOD, IOExpander::PinMode::OUTPUT);
  io_expander_int.set_direction(SPV,  IOExpander::PinMode::OUTPUT);

  #if INKPLATE_6 || INKPLATE_6V2 || INKPLATE_6FLICK

      i2s_comms.set_pin( 0, I2S1O_BCK_OUT_IDX,   0);
      i2s_comms.set_pin( 4, I2S1O_DATA_OUT0_IDX, 0);
      i2s_comms.set_pin( 5, I2S1O_DATA_OUT1_IDX, 0);
      i2s_comms.set_pin(18, I2S1O_DATA_OUT2_IDX, 0);
      i2s_comms.set_pin(19, I2S1O_DATA_OUT3_IDX, 0);
      i2s_comms.set_pin(23, I2S1O_DATA_OUT4_IDX, 0);
      i2s_comms.set_pin(25, I2S1O_DATA_OUT5_IDX, 0);
      i2s_comms.set_pin(26, I2S1O_DATA_OUT6_IDX, 0);
      i2s_comms.set_pin(27, I2S1O_DATA_OUT7_IDX, 0);

      i2s_comms.start_clock();

  #else

    gpio_set_direction(GPIO_NUM_0,  GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_4,  GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_5,  GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_19, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_23, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_25, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_26, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_27, GPIO_MODE_OUTPUT);
    
  #endif
}


int8_t 
EInk::read_temperature()
{
  int8_t temp;
  
  if (get_panel_state() == PanelState::OFF) {
    Wire::enter();
    wakeup_set();
    ESP::delay_microseconds(1800);
    pwrup_set();
    Wire::leave();

    ESP::delay(5);
  }

  Wire::enter();
  wire.begin_transmission(PWRMGR_ADDRESS);
  wire.write(0x0D);
  wire.write(0b10000000);
  wire.end_transmission();
  Wire::leave();

  ESP::delay(5);

  Wire::enter();
  wire.begin_transmission(PWRMGR_ADDRESS);
  wire.write(0x00);
  wire.end_transmission();

  wire.request_from(PWRMGR_ADDRESS, 1);
  temp = wire.read();
    
  if (get_panel_state() == PanelState::OFF) {
    pwrup_clear();
    wakeup_clear();
    Wire::leave();

    ESP::delay(5);
  }
  else {
    Wire::leave();
  }

  return temp;
}
