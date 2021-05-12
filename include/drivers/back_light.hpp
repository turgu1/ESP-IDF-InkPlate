#pragma once

#if defined(INKPLATE_6PLUS)

#include "mcp23017.hpp"

class BackLight
{
  public:
    BackLight(MCP23017 & _mcp) : mcp(_mcp) {}
    bool setup();
    void set_level(uint8_t level);
    void enable(bool en);

  private:
    MCP23017 & mcp;

    const MCP23017::Pin BACKLIGHT_EN = MCP23017::Pin::IOPIN_11;

    static const uint8_t BACKLIGHT_ADDRESS = 0x2E;
};

#endif
