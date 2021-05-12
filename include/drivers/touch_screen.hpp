#pragma once

#if defined(INKPLATE_6PLUS)

#include "non_copyable.hpp"
#include "mcp23017.hpp"

#include <array>

class TouchScreen : NonCopyable 
{
  public:
    TouchScreen(MCP23017 & _mcp) : mcp(_mcp), ready(false) {}

    typedef std::array<uint16_t, 2> TouchPositions;
    
    bool             setup(bool power_on, void (*isr_handler)(), uint16_t scr_width, uint16_t scr_height);

    void          shutdown();
    bool is_screen_touched();
    uint8_t  get_positions(TouchPositions & x_positions, TouchPositions & y_positions);

    void   set_power_state(bool on_state);
    bool   get_power_state();

    bool          is_ready() { return ready; }

    void set_app_isr_handler(void (*isr_handler)());

  private:
    static constexpr char const * TAG = "TouchScreen";
    MCP23017 & mcp;

    uint16_t screen_width, screen_height;
    uint16_t x_resolution, y_resolution;

    const MCP23017::Pin TOUCHSCREEN_ENABLE = MCP23017::Pin::IOPIN_12;
    const MCP23017::Pin TOUCHSCREEN_RESET  = MCP23017::Pin::IOPIN_10;

    static const uint8_t    TOUCHSCREEN_ADDRESS       = 0x15;
    static const gpio_num_t PIN_TOUCHSCREEN_INTERRUPT = GPIO_NUM_36;

    typedef std::array<uint8_t, 4> Data;
    typedef std::array<uint8_t, 8> Data8;

    const Data hello_packet = { 0x55, 0x55, 0x55, 0x55 };

    bool ready;

    void hardware_reset();
    bool software_reset();

    bool  read(      Data  & data);
    bool  read(      Data8 & data);
    void write(const Data  & data);

    void get_resolution();
};

#endif