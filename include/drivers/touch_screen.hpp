#pragma once

#if defined(INKPLATE_6PLUS)

#include "non_copyable.hpp"
#include "mcp23017.hpp"

class TouchScreen : NonCopyable 
{
  public:
    TouchScreen(MCP23017 & _mcp) : mcp(_mcp) {}
    
    bool              setup(bool power_on);

    bool        touchInArea(int16_t x1, int16_t y1, int16_t w, int16_t h);
    void         tsShutdown();
    bool        tsAvailable();
    void    tsSetPowerState(uint8_t _s);
    uint8_t tsGetPowerState();
    uint8_t       tsGetData(uint16_t *xPos, uint16_t *yPos);

    virtual int getRotation() = 0;

  private:
    static constexpr char const * TAG = "TouchScreen";
    MCP23017 & mcp;

    const MCP23017::Pin TOUCHSCREEN_EN = MCP23017::Pin::IOPIN_12;
    static const uint8_t TOUCHSCREEN_ADDRESS = 0x15;
};

#endif