#pragma once

#if INKPLATE_6FLICK

#include "non_copyable.hpp"

#if PCAL6416
  #include "pcal6416.hpp"
#else
  #include "mcp23017.hpp"
#endif

#include "wire.hpp"

#include <array>

class TouchScreen : NonCopyable 
{
  public:

    static constexpr gpio_num_t INTERRUPT_PIN = GPIO_NUM_36;

    static constexpr uint16_t MAX_X = 757;
    static constexpr uint16_t MAX_Y = 1023;

    TouchScreen(IOExpander & _io_expander) : io_expander(_io_expander), ready(false) {}

    typedef void (* ISRHandlerPtr)(void * value);

    typedef std::array<uint16_t, 2> TouchPositions;
    
    bool             setup(bool power_on, ISRHandlerPtr isr_handler = nullptr);

    void   set_power_state(bool on_state);
    bool   get_power_state();

    void          shutdown();
    bool is_screen_touched();
    uint8_t   get_position(TouchPositions & x_positions, TouchPositions & y_positions);

    bool   is_ready() { return ready; }

    void set_app_isr_handler(ISRHandlerPtr isr_handler);

    //inline void TouchScreen::end_isr() { Wire::enter(); handshake(); Wire::leave(); }

    inline uint16_t get_x_resolution() { return MAX_X; }
    inline uint16_t get_y_resolution() { return MAX_Y; }

  private:

    static constexpr uint8_t BASE_ADDR          = 0x00;
    static constexpr uint8_t SOFT_RESET_MODE    = 0x01;
    static constexpr uint8_t SYS_INFO_MODE      = 0x10;
    static constexpr uint8_t OPERATE_MODE       = 0x00;
    static constexpr uint8_t LOW_POWER_MODE     = 0x04;
    static constexpr uint8_t DEEP_SLEEP_MODE    = 0x02;
    static constexpr uint8_t REG_ACT_INTERVAL   = 0x1D;
    static constexpr uint8_t DETECTION_DISTANCE = 0x1E;

    static constexpr uint8_t ACT_INTERVAL_DEFAULT        = 0x00;
    static constexpr uint8_t LOW_POWER_INTERVAL_DEFAULT  = 0x0A;
    static constexpr uint8_t TOUCH_TIMEOUT_DEFAULT       = 0xFF;

    struct BootLoaderData
    {
        uint8_t bl_file;
        uint8_t bl_status;
        uint8_t bl_error;
        uint8_t blver_hi;
        uint8_t blver_lo;
        uint8_t bld_blver_hi;
        uint8_t bld_blver_lo;
        uint8_t ttspver_hi;
        uint8_t ttspver_lo;
        uint8_t appid_hi;
        uint8_t appid_lo;
        uint8_t appver_hi;
        uint8_t appver_lo;
        uint8_t cid_0;
        uint8_t cid_1;
        uint8_t cid_2;
    } boot_loader_data;

    struct SysInfoData
    {
        uint8_t hst_mode;
        uint8_t mfg_stat;
        uint8_t mfg_cmd;
        uint8_t cid[3];
        uint8_t tt_undef1;
        uint8_t uid[8];
        uint8_t bl_verh;
        uint8_t bl_verl;
        uint8_t tts_verh;
        uint8_t tts_verl;
        uint8_t app_idh;
        uint8_t app_idl;
        uint8_t app_verh;
        uint8_t app_verl;
        uint8_t tt_undef[5];
        uint8_t scn_typ;
        uint8_t act_intrvl;
        uint8_t tch_tmout;
        uint8_t lp_intrvl;
    } sys_info_data;

    static constexpr char const * TAG = "TouchScreen";
    IOExpander & io_expander;

    uint16_t x_resolution, y_resolution;

    static constexpr IOExpander::Pin TOUCHSCREEN_ENABLE = IOExpander::Pin::IOPIN_12;
    static constexpr IOExpander::Pin TOUCHSCREEN_RESET  = IOExpander::Pin::IOPIN_10;

    static constexpr uint8_t    TOUCHSCREEN_ADDRESS     = 0x24;

    bool ready;

    void hardware_reset();
    bool software_reset();

    bool read(uint8_t cmd, uint8_t (& data)[], uint8_t size);
    bool write(uint8_t cmd, uint8_t (& data)[], uint8_t size);
    bool send_command(uint8_t cmd);

    bool get_boot_loader_data(BootLoaderData & bl_data);
    bool exit_boot_loader_mode();
    bool get_sys_info(SysInfoData & sys_info_data);
    bool set_sys_info_regs(SysInfoData & sys_info_data);
    void handshake();
    bool ping(int retries);
};

#endif