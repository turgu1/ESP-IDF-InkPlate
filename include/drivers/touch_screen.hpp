#pragma once

#if defined(INKPLATE_6PLUS)

#include "non_copyable.hpp"

#if PCAL6416
  #include "pcal6416.hpp"
#else
  #include "mcp23017.hpp"
#endif

#include <array>

class TouchScreen : NonCopyable 
{
  public:
    TouchScreen(IOExpander & _io_expander) : io_expander(_io_expander), ready(false) {}

    typedef void (* ISRHandlerPtr)(void * value);

    static const gpio_num_t INTERRUPT_PIN = GPIO_NUM_36;

    typedef std::array<uint16_t, 2> TouchPositions;
    
    bool             setup(bool power_on, ISRHandlerPtr isr_handler = nullptr);

    void          shutdown();
    bool is_screen_touched();
    uint8_t   get_position(TouchPositions & x_positions, TouchPositions & y_positions);

    void   set_power_state(bool on_state);
    bool   get_power_state();

    bool   is_ready() { return ready; }

    void set_app_isr_handler(ISRHandlerPtr isr_handler);

    inline uint16_t get_x_resolution() { return x_resolution; }
    inline uint16_t get_y_resolution() { return y_resolution; }

  private:
    static constexpr char const * TAG = "TouchScreen";
    IOExpander & io_expander;

    uint16_t x_resolution, y_resolution;

    const IOExpander::Pin TOUCHSCREEN_ENABLE = IOExpander::Pin::IOPIN_12;
    const IOExpander::Pin TOUCHSCREEN_RESET  = IOExpander::Pin::IOPIN_10;

    static const uint8_t    TOUCHSCREEN_ADDRESS       = 0x15;

    typedef std::array<uint8_t, 4> Data;
    typedef std::array<uint8_t, 8> Data8;

    bool ready;

    void hardware_reset();
    bool software_reset();

    bool  read(      Data  & data);
    bool  read(      Data8 & data);
    void write(const Data  & data);

    void retrieve_resolution();
};

#endif