#pragma once

#if INKPLATE_6PLUS

#include <cinttypes>
#include "non_copyable.hpp"

#include <ctime>

class RTC : NonCopyable {
  public:
    enum class CalibrationMode : uint8_t {
      EVERY_TWO_HOURS    = 0x00,
      EVERY_FOUR_MINUTES = 0x01
    };

    enum class WeekDay : uint8_t {
      SUN = 0,
      MON = 1,
      TUE = 2,
      WED = 3,
      THU = 4,
      FRI = 5,
      SAT = 6
    };

    enum class CAPACITOR : uint8_t {
      SEL_7PF    = 0,
      SEL_12_5PF = 1
    };

  private:
    static constexpr char const * TAG = "RTC";
    enum class Reg : uint8_t {
      CTRL1         = 0x00,
      CTRL2         = 0x01,
      OFFSET        = 0x02,
      RAM           = 0x03,
      SEC           = 0x04,
      MIN           = 0x05,
      HOUR          = 0x06,
      DAY           = 0x07,
      WEEKDAY       = 0x08,
      MONTH         = 0x09,
      YEAR          = 0x0A,
      SEC_ALARM     = 0x0B,
      MIN_ALARM     = 0x0C,
      HOUR_ALARM    = 0x0D,
      DAY_ALARM     = 0x0E,
      WEEKDAY_ALARM = 0x0F,
      TIMER_VAL     = 0x10,
      TIMER_MODE    = 0x11
    };

    const uint8_t rtc_address;

    const uint8_t STOP_BIT     = 0x20;
    const uint8_t RESET_CODE   = 0x58;

    const uint8_t SECONDS_MASK = 0x7F;
    const uint8_t HOUR_MASK    = 0x3F;

    uint8_t dec_to_bcd(uint8_t value);
    uint8_t bcd_to_dec(uint8_t value);

    uint8_t   read_reg(Reg reg);
    void     write_reg(Reg reg, uint8_t value);

    void write_date_time();
    void  read_date_time();

    uint8_t read_calibration_reg();

    uint8_t  second;
    uint8_t  minute;
    uint8_t  hour;
    WeekDay  week_day;
    uint8_t  day;
    uint8_t  month;
    uint16_t year;

  public:
    RTC(uint8_t address) : rtc_address(address) {}

    bool setup();
    void reset();

    void start();
    void  stop();

    void    set_ram(uint8_t value);
    uint8_t get_ram();

    void         set_calibration(CalibrationMode mode, float Fmeas     );
    uint8_t calibrate_by_seconds(CalibrationMode mode, float offset_sec);

    CAPACITOR set_capacitor(CAPACITOR value);
    CAPACITOR get_capacitor();

    void set_date_time(uint16_t  y, uint8_t  m, uint8_t d, 
                       uint8_t   h, uint8_t mm, uint8_t s,
                       WeekDay  wd);

    void set_date_time(const time_t * t);

    void get_date_time(uint16_t &  y, uint8_t &  m, uint8_t & d, 
                       uint8_t  &  h, uint8_t & mm, uint8_t & s,
                       WeekDay  & wd);

    void get_date_time(time_t * t);
};

#endif