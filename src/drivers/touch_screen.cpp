#if INKPLATE_6PLUS || INKPLATE_6PLUS_V2

#include "touch_screen.hpp"
#include "wire.hpp"
#include "esp.hpp"

#include "driver/gpio.h"

static TouchScreen::ISRHandlerPtr app_isr_handler;
static volatile bool touchscreen_interrupt_happened = false;

static void IRAM_ATTR 
touchscreen_isr(void * value) 
{
  touchscreen_interrupt_happened = true;

  if (app_isr_handler != nullptr) (* app_isr_handler)(value);
}

bool 
TouchScreen::setup(bool power_on, ISRHandlerPtr isr_handler)
{
  ready                          = false;
  touchscreen_interrupt_happened = false;

  app_isr_handler                = nullptr;

  Wire::enter();

  io_expander.set_direction(TOUCHSCREEN_ENABLE, IOExpander::PinMode::OUTPUT );
  io_expander.set_direction(TOUCHSCREEN_RESET,  IOExpander::PinMode::OUTPUT );
  io_expander.digital_write(TOUCHSCREEN_ENABLE, IOExpander::SignalLevel::LOW); // on

  gpio_config_t io_conf;

  io_conf.intr_type    = GPIO_INTR_NEGEDGE; 
  io_conf.pin_bit_mask = 1ULL << INTERRUPT_PIN;
  io_conf.mode         = GPIO_MODE_INPUT;
  io_conf.pull_up_en   = GPIO_PULLUP_ENABLE;

  gpio_config(&io_conf);

  gpio_install_isr_service(0);
  
  gpio_isr_handler_add(
    INTERRUPT_PIN, 
    touchscreen_isr, 
    (void *) INTERRUPT_PIN);

  Wire::leave();

  hardware_reset();

  if (software_reset()) {
    retrieve_resolution();
    set_power_state(power_on);

    if (isr_handler != nullptr) set_app_isr_handler(isr_handler);

    ready = true;
  }

  return ready;
}

void 
TouchScreen::set_app_isr_handler(ISRHandlerPtr isr_handler)
{
  gpio_intr_disable(INTERRUPT_PIN);
  app_isr_handler = isr_handler;
  gpio_intr_enable(INTERRUPT_PIN);
}

void
TouchScreen::hardware_reset()
{
  Wire::enter();
  io_expander.digital_write(TOUCHSCREEN_RESET, IOExpander::SignalLevel::LOW ); ESP::delay(15);
  io_expander.digital_write(TOUCHSCREEN_RESET, IOExpander::SignalLevel::HIGH); ESP::delay(15); 
  Wire::leave();
}

bool
TouchScreen::software_reset()
{
  bool result = false;

  Wire::enter();

  const Data reset_cmd = { 0x77, 0x77, 0x77, 0x77 };
  write(reset_cmd);

  uint16_t timeout = 1000;
  while (!touchscreen_interrupt_happened && (timeout > 0)) {
    ESP::delay(1);
    timeout--;
  }

  Data answer;
  if (read(answer)) {
    const Data hello_packet = { 0x55, 0x55, 0x55, 0x55 };

    result = answer == hello_packet;
  }

  Wire::leave();

  touchscreen_interrupt_happened = false;

  return result;
}

void 
TouchScreen::retrieve_resolution()
{
  const Data cmd_get_x_resolution = { 0x53, 0x60, 0x00, 0x00 };
  const Data cmd_get_y_resolution = { 0x53, 0x63, 0x00, 0x00 };

  Data answer;

  Wire::enter();

  write(cmd_get_x_resolution);
  read(answer);
  x_resolution = ((answer[3] & 0xf0) << 4) + answer[2];

  write(cmd_get_y_resolution);
  read(answer);
  y_resolution = ((answer[3] & 0xf0) << 4) + answer[2];

  Wire::leave();

  touchscreen_interrupt_happened = false;
}

uint8_t
TouchScreen::get_position(TouchPositions & x_positions, TouchPositions & y_positions)
{
  Data8 raw;
  // TouchPositions x_raw, y_raw;

  Wire::enter();
  read(raw);
  Wire::leave();

  uint8_t fingers = 0;
  for (int i = 0; i < 8; i++) {
    if (raw[7] & (1 << i)) fingers++;
    if (fingers == 2) break;
  }

  for (int i = 0, j = 1; i < 2; i++, j += 3) {
    x_positions[i] = ((raw[j] & 0xf0) << 4) + raw[j + 1];
    y_positions[i] = ((raw[j] & 0x0f) << 8) + raw[j + 2];
  }

  touchscreen_interrupt_happened = false;

  return fingers;
}

void 
TouchScreen::set_power_state(bool on_state)
{
  Data power_state_reg = { 0x54, 0x50, 0x00, 0x01 };
  if (on_state) power_state_reg[1] |= (1 << 3);

  Wire::enter();
  write(power_state_reg);
  Wire::leave();
}

bool
TouchScreen::get_power_state()
{
    const Data power_state_cmd = { 0x53, 0x50, 0x00, 0x01 };
    Data data;

    Wire::enter();
    write(power_state_cmd);
    read(data);
    Wire::leave();

    touchscreen_interrupt_happened = false;

    return (data[1] & (1 << 3)) != 0;
}

bool
TouchScreen::read(Data & data)
{
  if (wire.request_from(TOUCHSCREEN_ADDRESS, data.size()) != ESP_OK) return false;
  for (auto & d : data) {
    d = wire.read();
  }
  return true;
}

bool
TouchScreen::read(Data8 & data)
{
  if (wire.request_from(TOUCHSCREEN_ADDRESS, data.size()) != ESP_OK) return false;
  for (auto & d : data) {
    d = wire.read();
  }
  return true;
}

void 
TouchScreen::write(const Data & data)
{
  wire.begin_transmission(TOUCHSCREEN_ADDRESS);
  for (auto & d : data) {
    wire.write(d);
  }
  wire.end_transmission();
}

void
TouchScreen::shutdown()
{
  Wire::enter();
  io_expander.digital_write(TOUCHSCREEN_ENABLE, IOExpander::SignalLevel::HIGH); // off
  Wire::leave();

  ready = false;
}

bool
TouchScreen::is_screen_touched()
{
  return touchscreen_interrupt_happened;
}

#endif
