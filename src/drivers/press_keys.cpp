#define __PRESS_KEYS__ 1
#include "press_keys.hpp"
#include "wire.hpp"

bool
PressKeys::setup() 
{
  Wire::enter();
  io_expander.set_direction(PRESS_0, IOExpander::PinMode::INPUT_PULLUP);
  io_expander.set_direction(PRESS_1, IOExpander::PinMode::INPUT_PULLUP);
  io_expander.set_direction(PRESS_2, IOExpander::PinMode::INPUT_PULLUP);
  io_expander.set_direction(PRESS_3, IOExpander::PinMode::INPUT_PULLUP);
  io_expander.set_direction(PRESS_4, IOExpander::PinMode::INPUT_PULLUP);
  io_expander.set_direction(PRESS_5, IOExpander::PinMode::INPUT_PULLUP);

  // Prepare the MCP device to allow for interrupts
  // coming from any of the presskeys. Interrupts will be raised
  // for any change of state of the 6 presskeys. The GPIO_NUM_34
  // must be programmed as per the ESP-IDF documentation to get
  // some interrupts.

  io_expander.set_int_pin(PRESS_0, IOExpander::IntMode::FALLING);
  io_expander.set_int_pin(PRESS_1, IOExpander::IntMode::FALLING);
  io_expander.set_int_pin(PRESS_2, IOExpander::IntMode::FALLING);
  io_expander.set_int_pin(PRESS_3, IOExpander::IntMode::FALLING);
  io_expander.set_int_pin(PRESS_4, IOExpander::IntMode::FALLING);
  io_expander.set_int_pin(PRESS_5, IOExpander::IntMode::FALLING);

  io_expander.set_int_output(IOExpander::IntPort::INTPORTB, false, false, IOExpander::SignalLevel::HIGH);
  Wire::leave();  

  return true;
}

uint8_t 
PressKeys::read_all_keys()
{
  Wire::enter();
  uint16_t value = io_expander.get_ports();
  Wire::leave();
  return ((value >> 10) & 0x3F) ^ 0x3F;
}

uint8_t 
PressKeys::read_key(Key key)
{
  Wire::enter();
  IOExpander::SignalLevel value = io_expander.digital_read((IOExpander::Pin)((static_cast<uint8_t>(key)) + 10)); // Not clean
  Wire::leave();

  return value == IOExpander::SignalLevel::HIGH ? 0 : 1;
}
