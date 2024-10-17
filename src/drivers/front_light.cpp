#if INKPLATE_6PLUS || INKPLATE_6PLUS_V2 || INKPLATE_6FLICK

#include "esp_log.h"

#include "front_light.hpp"

bool 
FrontLight::setup()
{
  ESP_LOGD(TAG, "Initializing...");
  
  wire.setup();
  Wire::enter();

  wire_device = new WireDevice(FRONTLIGHT_ADDRESS);
  if ((wire_device == nullptr) || !wire_device->is_initialized()) return false;

  io_expander.set_direction(FRONTLIGHT_EN, IOExpander::PinMode::OUTPUT);
  io_expander.digital_write(FRONTLIGHT_EN, IOExpander::SignalLevel::LOW); // disabled
  
  Wire::leave();

  return true;
}

void 
FrontLight::set_level(uint8_t level)
{
  uint8_t pgm[] = {
    0,
    (uint8_t) (63 - (level & 0b00111111))
  };

  Wire::enter();
  wire_device->write(pgm, sizeof(pgm));

  // wire.begin_transmission(FRONTLIGHT_ADDRESS);
  // wire.write(0);
  // wire.write(63 - (level & 0b00111111));
  // wire.end_transmission();

  Wire::leave();
}

void 
FrontLight::enable()
{
  Wire::enter();
  io_expander.digital_write(FRONTLIGHT_EN, IOExpander::SignalLevel::HIGH);
  Wire::leave();
}


void 
FrontLight::disable()
{
  Wire::enter();
  io_expander.digital_write(FRONTLIGHT_EN, IOExpander::SignalLevel::LOW);
  Wire::leave();
}
#endif