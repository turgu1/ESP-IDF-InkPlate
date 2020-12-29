#ifndef __BATTERY_HPP__
#define __BATTERY_HPP__

#include "non_copyable.hpp"
#include "mcp23017.hpp"

class Battery : NonCopyable
{
  public:
    Battery(MCP23017 & _mcp) : mcp(_mcp) {}
    bool setup();

    double read_level();

  private:
    static constexpr char const * TAG = "Battery";
    MCP23017 & mcp;

    const MCP23017::Pin BATTERY_SWITCH = MCP23017::Pin::IOPIN_9;
};

#endif