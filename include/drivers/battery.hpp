#pragma once

#include "non_copyable.hpp"

#if PCAL6416
  #include "pcal6416.hpp"
#else
  #include "mcp23017.hpp"
#endif

class Battery : NonCopyable
{
  public:
    Battery(IOExpander & _io_expander) : io_expander(_io_expander) {}
    bool setup();

    double read_level();

  private:
    static constexpr char const * TAG = "Battery";
    IOExpander & io_expander;

    const IOExpander::Pin BATTERY_SWITCH = IOExpander::Pin::IOPIN_9;
};
