#if defined(INKPLATE_6PLUS)

#include "touch_screen.hpp"
#include "wire.hpp"

bool 
TouchScreen::setup()
{
  Wire::enter();
  mcp.set_direction(TOUCHSCREEN_EN, MCP23017::PinMode::OUTPUT);
  mcp.digital_write(TOUCHSCREEN_EN, MCP23017::SignalLevel::HIGH); // off
  Wire::leave();

  return true;
}

#endif
