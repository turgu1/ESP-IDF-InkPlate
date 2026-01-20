#if INKPLATE_6PLUS || INKPLATE_6PLUS_V2

#include "touch_screen_elan.hpp"
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

  wire_device = new WireDevice(TOUCHSCREEN_ADDRESS);
  if ((wire_device == nullptr) || !wire_device->is_initialized()) {
    ESP_LOGE(TAG, "Setup error: %s", wire_device == nullptr ? "NULL Device!" : "Not initialized!");
    return false;
  }

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
  } else {
    ESP_LOGE(TAG, "Unable to do a software reset!");
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

  const uint8_t reset_cmd[] = { 0x77, 0x77, 0x77, 0x77 };
  wire_device->write(reset_cmd, sizeof(reset_cmd));

  int16_t timeout = 1000;
  while (!touchscreen_interrupt_happened && (timeout > 0)) {
    ESP::delay(1);
    timeout--;
  }

  if (timeout <= 0) {
    ESP_LOGE(TAG, "Timeout waiting for interrupt!");
  } else {
    uint8_t answer[4];
    if (wire_device->read(answer, sizeof(answer))) {
      const uint8_t hello_packet[] = { 0x55, 0x55, 0x55, 0x55 };

      result = memcmp(answer, hello_packet, sizeof(answer)) == 0;
      if (result) ESP_LOGI(TAG, "Software reset OK");
    }
  }

  Wire::leave();

  touchscreen_interrupt_happened = false;

  return result;
}

void 
TouchScreen::retrieve_resolution()
{
  const uint8_t cmd_get_x_resolution[] = { 0x53, 0x60, 0x00, 0x00 };
  const uint8_t cmd_get_y_resolution[] = { 0x53, 0x63, 0x00, 0x00 };

  uint8_t answer[4];

  Wire::enter();

  wire_device->cmd_read(cmd_get_x_resolution, sizeof(cmd_get_x_resolution), answer, sizeof(answer));
  x_resolution = ((answer[3] & 0xf0) << 4) + answer[2];

  wire_device->cmd_read(cmd_get_y_resolution, sizeof(cmd_get_y_resolution), answer, sizeof(answer));
  y_resolution = ((answer[3] & 0xf0) << 4) + answer[2];

  Wire::leave();

  ESP_LOGI(TAG, "Resolution: [%" PRIu16 ", %" PRIu16 "]", x_resolution, y_resolution);

  touchscreen_interrupt_happened = false;
}

uint8_t
TouchScreen::get_position(TouchPositions & x_positions, TouchPositions & y_positions)
{
  uint8_t raw[8];
  // TouchPositions x_raw, y_raw;

  Wire::enter();
  wire_device->read(raw, sizeof(raw));
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

  ESP_LOGD(TAG, "Pos: [%" PRIu16 ",%" PRIu16 "], [%" PRIu16 ",%" PRIu16 "], Fingers: %" PRIu8, 
    x_positions[0], y_positions[0], x_positions[1], y_positions[1], fingers);

  return fingers;
}

void 
TouchScreen::set_power_state(bool on_state)
{
  uint8_t power_state_reg[] = { 0x54, 0x50, 0x00, 0x01 };
  if (on_state) power_state_reg[1] |= (1 << 3);

  Wire::enter();
  wire_device->write(power_state_reg, sizeof(power_state_reg));
  Wire::leave();
}

bool
TouchScreen::get_power_state()
{
    const uint8_t power_state_cmd[] = { 0x53, 0x50, 0x00, 0x01 };
    uint8_t data[4];

    Wire::enter();
    wire_device->cmd_read(power_state_cmd, sizeof(power_state_cmd), data, sizeof(data));
    Wire::leave();

    touchscreen_interrupt_happened = false;

    return (data[1] & (1 << 3)) != 0;
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
