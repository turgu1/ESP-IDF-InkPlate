#pragma once

#if defined(INKPLATE_6PLUS)

#include "mcp23017.hpp"

class FrontLight
{
  public:
    FrontLight(MCP23017 & _mcp) : mcp(_mcp) {}
    bool setup();
    void set_level(uint8_t level);
    void enable();
    void disable();

  private:
    static constexpr char const * TAG = "FrontLight";
    
    MCP23017 & mcp;

    const MCP23017::Pin FRONTLIGHT_EN = MCP23017::Pin::IOPIN_11;

    static const uint8_t FRONTLIGHT_ADDRESS = 0x2E;
};

#endif
