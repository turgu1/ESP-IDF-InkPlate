#if defined(INKPLATE_6PLUS)

#include "backlight.hpp"
#include "wire.hpp"

bool 
Backlight::setup()
{
  Wire::enter();
  mcp.set_direction(BACKLIGHT_EN, MCP23017::PinMode::OUTPUT);
  mcp.digital_write(BACKLIGHT_EN, MCP23017::SignalLevel::LOW); // off
  Wire::leave();

  return true;
}

void 
Backlight::set_level(uint8_t level)
{
  Wire::enter();
  wire.beginTransmission(BACKLIGHT_ADDRESS);
  wire.write(0);
  wire.write(63 - (level & 0b00111111));
  wire.endTransmission();
  Wire::leave();
}

void 
Backlight::power_on(bool on)
{
  Wire::enter();
  if (on) {
    mcp.digital_write(BACKLIGHT_EN, MCP23017::SignalLevel::HIGH);
  }
  else {
    mcp.digital_write(BACKLIGHT_EN, MCP23017::SignalLevel::LOW);
  }
  Wire::leave();
}

#endif