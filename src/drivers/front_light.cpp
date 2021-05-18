#if defined(INKPLATE_6PLUS)

#include "front_light.hpp"
#include "wire.hpp"

bool 
FrontLight::setup()
{
  ESP_LOGD(TAG, "Initializing...");
  
  wire.setup();
  
  Wire::enter();
  mcp.set_direction(FRONTLIGHT_EN, MCP23017::PinMode::OUTPUT);
  mcp.digital_write(FRONTLIGHT_EN, MCP23017::SignalLevel::LOW); // off
  Wire::leave();

  return true;
}

void 
FrontLight::set_level(uint8_t level)
{
  Wire::enter();
  wire.begin_transmission(FRONTLIGHT_ADDRESS);
  wire.write(0);
  wire.write(63 - (level & 0b00111111));
  wire.end_transmission();
  Wire::leave();
}

void 
FrontLight::enable(bool en)
{
  Wire::enter();
  if (en) {
    mcp.digital_write(FRONTLIGHT_EN, MCP23017::SignalLevel::HIGH);
  }
  else {
    mcp.digital_write(FRONTLIGHT_EN, MCP23017::SignalLevel::LOW);
  }
  Wire::leave();
}

#endif