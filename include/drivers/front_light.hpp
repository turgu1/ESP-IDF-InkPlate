#pragma once

#if INKPLATE_6PLUS || INKPLATE_6PLUS_V2

#if PCAL6416
  #include "pcal6416.hpp"
#else
  #include "mcp23017.hpp"
#endif

class FrontLight
{
  public:
    FrontLight(IOExpander & _io_expander) : io_expander(_io_expander) {}
    bool setup();
    void set_level(uint8_t level);
    void enable();
    void disable();

  private:
    static constexpr char const * TAG = "FrontLight";
    
    IOExpander & io_expander;

    const IOExpander::Pin FRONTLIGHT_EN = IOExpander::Pin::IOPIN_11;

    static const uint8_t FRONTLIGHT_ADDRESS = 0x2E;
};

#endif
