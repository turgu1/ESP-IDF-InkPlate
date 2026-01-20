#define __TOUCH_KEYS__ 1
#include "touch_keys.hpp"
#include "wire.hpp"

bool
TouchKeys::setup() 
{
  Wire::enter();
  io_expander.set_direction(TOUCH_0, IOExpander::PinMode::INPUT);
  io_expander.set_direction(TOUCH_1, IOExpander::PinMode::INPUT);
  io_expander.set_direction(TOUCH_2, IOExpander::PinMode::INPUT);

  // Prepare the MCP device to allow for interrupts
  // coming from any of the touchkeys. Interrupts will be raised
  // for any change of state of the 3 touchkeys. The GPIO_NUM_34
  // must be programmed as per the ESP-IDF documentation to get
  // some interrupts.

  io_expander.set_int_pin(TOUCH_0, IOExpander::IntMode::RISING);
  io_expander.set_int_pin(TOUCH_1, IOExpander::IntMode::RISING);
  io_expander.set_int_pin(TOUCH_2, IOExpander::IntMode::RISING);

  io_expander.set_int_output(IOExpander::IntPort::INTPORTB, false, false, IOExpander::SignalLevel::HIGH);
  Wire::leave();  

  return true;
}

uint8_t 
TouchKeys::read_all_keys()
{
  Wire::enter();
  uint16_t value = io_expander.get_ports();
  Wire::leave();
  return (value >> 10) & 7;  // Not clean
}

uint8_t 
TouchKeys::read_key(Key key)
{
  Wire::enter();
  IOExpander::SignalLevel value = io_expander.digital_read((IOExpander::Pin)((static_cast<uint8_t>(key) & 3) + 10)); // Not clean
  Wire::leave();

  return value == IOExpander::SignalLevel::HIGH ? 1 : 0;
}
