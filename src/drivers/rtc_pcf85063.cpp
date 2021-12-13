#if INKPLATE_6PLUS

#include "wire.hpp"
#include "rtc_pcf85063.hpp"

uint8_t RTC::dec_to_bcd(uint8_t val) 
{
  return ((val / 10) << 4) + (val % 10);
}

uint8_t RTC::bcd_to_dec(uint8_t val) 
{
  return ((val >> 4) * 10) + (val & 0x0F);
}

bool RTC::setup() 
{
  set_capacitor(CAPACITOR::SEL_12_5PF);
  return true;
}

void RTC::reset() 
{
  write_reg(Reg::CTRL1, RESET_CODE);
}

void RTC::start() 
{
  write_reg(Reg::CTRL1, read_reg(Reg::CTRL1) & ~STOP_BIT);
}

void RTC::stop() 
{
  write_reg(Reg::CTRL1, read_reg(Reg::CTRL1) | STOP_BIT);
}

void RTC::read_date_time() 
{
  wire.begin_transmission(rtc_address);
  wire.write((uint8_t) Reg::SEC);
  wire.end_transmission();

  wire.request_from(rtc_address, 7);

  second   = bcd_to_dec(wire.read() & SECONDS_MASK);
  minute   = bcd_to_dec(wire.read());
  hour     = bcd_to_dec(wire.read() & HOUR_MASK);
  day      = bcd_to_dec(wire.read());
  week_day = (WeekDay) bcd_to_dec(wire.read());
  month    = bcd_to_dec(wire.read());
  year     = bcd_to_dec(wire.read());
}

void RTC::write_date_time() 
{
  write_reg(Reg::SEC,     dec_to_bcd(second  ));
  write_reg(Reg::MIN,     dec_to_bcd(minute  ));
  write_reg(Reg::HOUR,    dec_to_bcd(hour    ));
  write_reg(Reg::DAY,     dec_to_bcd(day     ));
  write_reg(Reg::WEEKDAY, dec_to_bcd((uint8_t) week_day));
  write_reg(Reg::MONTH,   dec_to_bcd(month   ));
  write_reg(Reg::YEAR,    dec_to_bcd(year    ));
}

void RTC::set_date_time(uint16_t &  y, uint8_t &  m, uint8_t & d, 
                        uint8_t  &  h, uint8_t & mm, uint8_t & s,
                        WeekDay  & wd)
{
  year     =  y - 2000;
  month    =  m;
  day      =  d;
  hour     =  h;
  minute   = mm;
  second   =  s;
  week_day = wd;

  write_date_time();
}

void RTC::get_date_time(uint16_t &  y, uint8_t &  m, uint8_t & d, 
                        uint8_t  &  h, uint8_t & mm, uint8_t & s,
                        WeekDay  & wd)
{
  read_date_time();

  y  = year + 2000;
  m  = month;
  d  = day;
  h  = hour;
  mm = minute;
  s  = second;
  wd = week_day;
}

/**
  @brief  clock calibration setting

  @parameter:
   mode: CalibrationMode
   offset_sec: offset value of one second.

  If the RTC time too fast: offset_sec < 0
  If the RTC time too slow: offset_sec > 0
*/
uint8_t RTC::calibrate_by_seconds(CalibrationMode mode, float offset_sec) 
{
  float Fmeas = 32768.0 + offset_sec * 32768.0;
  set_calibration(mode, Fmeas);
  return read_calibration_reg();
}

/**
  @brief: Clock calibrate setting
  @parameter:
        mode: CalibrationMode
        Fmeas: Real frequency you detect
*/
void RTC::set_calibration(CalibrationMode mode, float Fmeas) 
{
  float offset = 0;
  float Tmeas  = 1.0 / Fmeas;
  float Dmeas  = 1.0 / 32768 - Tmeas;
  float Eppm   = 1000000.0 * Dmeas / Tmeas;
  
  if (mode == CalibrationMode::EVERY_TWO_HOURS) {
    offset = Eppm / 4.34;
  } else if (mode == CalibrationMode::EVERY_FOUR_MINUTES) {
    offset = Eppm / 4.069;
  }

  uint8_t data = ((((uint8_t) mode) << 7) & 0x80) | ((int)(offset + 0.5) & 0x7f);
  write_reg(Reg::OFFSET, data);
}

uint8_t RTC::read_calibration_reg(void) 
{
  return read_reg(Reg::OFFSET);
}


void RTC::set_ram(uint8_t value) {
  write_reg(Reg::RAM, value);
}

uint8_t RTC::get_ram(void) 
{
  return read_reg(Reg::RAM);
}

RTC::CAPACITOR RTC::set_capacitor(CAPACITOR value) 
{
  uint8_t control_1 = read_reg(Reg::CTRL1);
  control_1 = (control_1 & 0xFE) | (0x01 & (uint8_t) value);
  write_reg(Reg::CTRL1, control_1);

  return (CAPACITOR) (read_reg(Reg::CTRL1) & 0x01);
}

RTC::CAPACITOR RTC::get_capacitor() 
{
  return (CAPACITOR) (read_reg(Reg::CTRL1) & 0x01);
}

uint8_t RTC::read_reg(Reg reg) 
{
  wire.begin_transmission(rtc_address);
  wire.write((uint8_t) reg);
  wire.end_transmission();
  wire.request_from(rtc_address, 1);

  return wire.read();

}

void RTC::write_reg(Reg reg, uint8_t value) 
{
  wire.begin_transmission(rtc_address);
  wire.write((uint8_t) reg);
  wire.write(value);
  wire.end_transmission();

}

#endif