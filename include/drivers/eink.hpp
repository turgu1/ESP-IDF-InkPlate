#pragma once

#include "frame_buffer.hpp"
#include "mcp23017.hpp"
#include "wire.hpp"

class EInk
{
  public:
    enum class PanelState  { OFF, ON };
    enum class PixelState : uint8_t { WHITE = 0b10101010, BLACK = 0b01010101, DISCHARGE = 0b00000000, SKIP = 0b11111111 };

    inline PanelState get_panel_state() { return panel_state; }
    inline bool        is_initialized() { return initialized; }

    virtual inline FrameBuffer1Bit * new_frame_buffer_1bit() = 0;
    virtual inline FrameBuffer3Bit * new_frame_buffer_3bit() = 0;

    virtual inline int16_t get_width()  = 0;
    virtual inline int16_t get_height() = 0;

    inline void preload_screen(FrameBuffer1Bit & frame_buffer) {
        memcpy(d_memory_new->get_data(), frame_buffer.get_data(), frame_buffer.get_data_size());
    }

    // All the following methods are protecting the I2C device interface trough
    // the Wire::enter() and Wire::leave() methods. These are implementing a
    // Mutex semaphore access control.
    //
    // If you ever add public methods, you *MUST* consider adding calls to Wire::enter()
    // and Wire::leave() and insure no deadlock will happen... or modify the mutex to use
    // a recursive mutex.

    virtual bool setup() = 0;

    virtual inline void update(FrameBuffer1Bit & frame_buffer) = 0;
    virtual inline void update(FrameBuffer3Bit & frame_buffer) = 0;

    virtual void partial_update(FrameBuffer1Bit & frame_buffer, bool force = false) = 0;

    int8_t read_temperature();

    void    turn_off();
    void    turn_on();
    uint8_t read_power_good();

  protected:                     
    
    EInk(MCP23017 & mcp) : 
      mcp_int(mcp),
      panel_state(PanelState::OFF), 
      initialized(false),
      partial_allowed(false) {}

    static const uint8_t PWRMGR_ADDRESS = 0x48;
    static const uint8_t PWR_GOOD_OK    = 0b11111010;

    MCP23017 & mcp_int;

    PanelState panel_state;
    bool       initialized;
    bool       partial_allowed;

    static const uint32_t PIN_LUT[256];

    void     vscan_start();
    void     hscan_start(uint32_t d);
    void       vscan_end();
    void    pins_z_state();
    void pins_as_outputs();

    inline void      allow_partial() { partial_allowed = true;  }
    inline void      block_partial() { partial_allowed = false; }
    inline bool is_partial_allowed() { return partial_allowed;  }

    inline void  set_panel_state(PanelState s) { panel_state = s; }

    static const uint32_t CL   = 0x01;
    static const uint32_t CKV  = 0x01;
    static const uint32_t SPH  = 0x02;
    static const uint32_t LE   = 0x04;

    static const uint32_t DATA = 0x0E8C0030;

    uint8_t         * p_buffer;
    FrameBuffer1Bit * d_memory_new;
    uint32_t        * GLUT;
    uint32_t        * GLUT2;

    const MCP23017::Pin OE             = MCP23017::Pin::IOPIN_0;
    const MCP23017::Pin GMOD           = MCP23017::Pin::IOPIN_1;
    const MCP23017::Pin SPV            = MCP23017::Pin::IOPIN_2;

    const MCP23017::Pin WAKEUP         = MCP23017::Pin::IOPIN_3;
    const MCP23017::Pin PWRUP          = MCP23017::Pin::IOPIN_4;
    const MCP23017::Pin VCOM           = MCP23017::Pin::IOPIN_5;
    
    const MCP23017::Pin GPIO0_ENABLE   = MCP23017::Pin::IOPIN_8;
  
    inline void cl_set()       { GPIO.out_w1ts = CL; }
    inline void cl_clear()     { GPIO.out_w1tc = CL; }

    inline void ckv_set()      { GPIO.out1_w1ts.val = CKV; }
    inline void ckv_clear()    { GPIO.out1_w1tc.val = CKV; }

    inline void sph_set()      { GPIO.out1_w1ts.val = SPH; }
    inline void sph_clear()    { GPIO.out1_w1tc.val = SPH; }

    inline void le_set()       { GPIO.out_w1ts = LE; }
    inline void le_clear()     { GPIO.out_w1tc = LE; }

    inline void oe_set()       { mcp_int.digital_write(OE,     MCP23017::SignalLevel::HIGH); }
    inline void oe_clear()     { mcp_int.digital_write(OE,     MCP23017::SignalLevel::LOW ); }

    inline void gmod_set()     { mcp_int.digital_write(GMOD,   MCP23017::SignalLevel::HIGH); }
    inline void gmod_clear()   { mcp_int.digital_write(GMOD,   MCP23017::SignalLevel::LOW ); }

    inline void spv_set()      { mcp_int.digital_write(SPV,    MCP23017::SignalLevel::HIGH); }
    inline void spv_clear()    { mcp_int.digital_write(SPV,    MCP23017::SignalLevel::LOW ); }

    inline void wakeup_set()   { mcp_int.digital_write(WAKEUP, MCP23017::SignalLevel::HIGH); }
    inline void wakeup_clear() { mcp_int.digital_write(WAKEUP, MCP23017::SignalLevel::LOW ); }

    inline void pwrup_set()    { mcp_int.digital_write(PWRUP,  MCP23017::SignalLevel::HIGH); }
    inline void pwrup_clear()  { mcp_int.digital_write(PWRUP,  MCP23017::SignalLevel::LOW ); }

    inline void vcom_set()     { mcp_int.digital_write(VCOM,   MCP23017::SignalLevel::HIGH); }
    inline void vcom_clear()   { mcp_int.digital_write(VCOM,   MCP23017::SignalLevel::LOW ); }
};

