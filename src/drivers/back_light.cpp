#if defined(INKPLATE_6PLUS)

#include "back_light.hpp"
#include "wire.hpp"

bool 
BackLight::setup()
{
  Wire::enter();
  mcp.set_direction(BACKLIGHT_EN, MCP23017::PinMode::OUTPUT);
  mcp.digital_write(BACKLIGHT_EN, MCP23017::SignalLevel::LOW); // off
  Wire::leave();

  return true;
}

void 
BackLight::set_level(uint8_t level)
{
  Wire::enter();
  wire.begin_transmission(BACKLIGHT_ADDRESS);
  wire.write(0);
  wire.write(63 - (level & 0b00111111));
  wire.end_transmission();
  Wire::leave();
}

void 
BackLight::enable(bool en)
{
  Wire::enter();
  if (en) {
    mcp.digital_write(BACKLIGHT_EN, MCP23017::SignalLevel::HIGH);
  }
  else {
    mcp.digital_write(BACKLIGHT_EN, MCP23017::SignalLevel::LOW);
  }
  Wire::leave();
}

#endif