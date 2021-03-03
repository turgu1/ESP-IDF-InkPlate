#define __PRESS_KEYS__ 1
#include "press_keys.hpp"
#include "wire.hpp"

bool
PressKeys::setup() 
{
  Wire::enter();
  mcp.set_direction(PRESS_0, MCP23017::PinMode::INPUT_PULLUP);
  mcp.set_direction(PRESS_1, MCP23017::PinMode::INPUT_PULLUP);
  mcp.set_direction(PRESS_2, MCP23017::PinMode::INPUT_PULLUP);
  mcp.set_direction(PRESS_3, MCP23017::PinMode::INPUT_PULLUP);
  mcp.set_direction(PRESS_4, MCP23017::PinMode::INPUT_PULLUP);
  mcp.set_direction(PRESS_5, MCP23017::PinMode::INPUT_PULLUP);

  // Prepare the MCP device to allow for interrupts
  // coming from any of the presskeys. Interrupts will be raised
  // for any change of state of the 6 presskeys. The GPIO_NUM_34
  // must be programmed as per the ESP-IDF documentation to get
  // some interrupts.

  mcp.set_int_pin(PRESS_0, MCP23017::IntMode::FALLING);
  mcp.set_int_pin(PRESS_1, MCP23017::IntMode::FALLING);
  mcp.set_int_pin(PRESS_2, MCP23017::IntMode::FALLING);
  mcp.set_int_pin(PRESS_3, MCP23017::IntMode::FALLING);
  mcp.set_int_pin(PRESS_4, MCP23017::IntMode::FALLING);
  mcp.set_int_pin(PRESS_5, MCP23017::IntMode::FALLING);

  mcp.set_int_output(MCP23017::IntPort::INTPORTB, false, false, MCP23017::SignalLevel::HIGH);
  Wire::leave();  

  return true;
}

uint8_t 
PressKeys::read_all_keys()
{
  Wire::enter();
  uint16_t value = mcp.get_ports();
  Wire::leave();
  return ((value >> 10) & 0x3F) ^ 0x3F;
}

uint8_t 
PressKeys::read_key(Key key)
{
  Wire::enter();
  MCP23017::SignalLevel value = mcp.digital_read((MCP23017::Pin)(((uint8_t)key) + 10)); // Not clean
  Wire::leave();

  return value == MCP23017::SignalLevel::HIGH ? 0 : 1;
}
