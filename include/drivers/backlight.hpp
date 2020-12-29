#ifndef __BACKLIGHT_H__
#define __BACKLIGHT_H__

#if defined(INKPLATE_6PLUS)

#include "mcp23017.hpp"

class Backlight
{
  public:
    Backlight(MCP23017 & _mcp) : mcp(_mcp) {}
    bool setup();
    void setBacklight(uint8_t _v);
    void backlight(bool _e);

  private:
    MCP23017 & mcp;

    const MCP23017::Pin BACKLIGHT_EN = MCP23017::Pin::IOPIN_11;
};

#endif

#endif