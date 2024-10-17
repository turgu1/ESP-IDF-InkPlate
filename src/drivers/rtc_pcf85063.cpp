#include "rtc_pcf85063.hpp"
#include "timegm.hpp"

#include <cstring>

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
  wire_device = new WireDevice(rtc_address);
  present = (wire_device != nullptr) && wire_device->is_initialized();     

  if (!present) return false;

  set_capacitor(CAPACITOR::SEL_12_5PF);
  return true;
}

void RTC::reset() 
{
  if (!present) return;
  wire_device->cmd_write((uint8_t) Reg::CTRL1, RESET_CODE);
}

void RTC::start() 
{
  if (!present) return;
  wire_device->cmd_write((uint8_t) Reg::CTRL1, wire_device->cmd_read((uint8_t) Reg::CTRL1) & ~STOP_BIT);
}

void RTC::stop() 
{
  if (!present) return;
  wire_device->cmd_write((uint8_t) Reg::CTRL1, wire_device->cmd_read((uint8_t) Reg::CTRL1) | STOP_BIT);
}

void RTC::read_date_time() 
{
  uint8_t data[7];

  wire_device->cmd_read((uint8_t) Reg::SEC, data, sizeof(data));

  second   = bcd_to_dec(data[0] & SECONDS_MASK);
  minute   = bcd_to_dec(data[1]);
  hour     = bcd_to_dec(data[2] & HOUR_MASK);
  day      = bcd_to_dec(data[3]);
  week_day = (WeekDay) bcd_to_dec(data[4]);
  month    = bcd_to_dec(data[5]);
  year     = bcd_to_dec(data[6]);

  // wire.begin_transmission(rtc_address);
  // wire.write((uint8_t) Reg::SEC);
  // wire.end_transmission();

  // wire.request_from(rtc_address, 7);

  // second   = bcd_to_dec(wire.read() & SECONDS_MASK);
  // minute   = bcd_to_dec(wire.read());
  // hour     = bcd_to_dec(wire.read() & HOUR_MASK);
  // day      = bcd_to_dec(wire.read());
  // week_day = (WeekDay) bcd_to_dec(wire.read());
  // month    = bcd_to_dec(wire.read());
  // year     = bcd_to_dec(wire.read());
}

void RTC::write_date_time() 
{
  wire_device->cmd_write((uint8_t) Reg::SEC,     dec_to_bcd(second  ));
  wire_device->cmd_write((uint8_t) Reg::MIN,     dec_to_bcd(minute  ));
  wire_device->cmd_write((uint8_t) Reg::HOUR,    dec_to_bcd(hour    ));
  wire_device->cmd_write((uint8_t) Reg::DAY,     dec_to_bcd(day     ));
  wire_device->cmd_write((uint8_t) Reg::WEEKDAY, dec_to_bcd((uint8_t) week_day));
  wire_device->cmd_write((uint8_t) Reg::MONTH,   dec_to_bcd(month   ));
  wire_device->cmd_write((uint8_t) Reg::YEAR,    dec_to_bcd(year    ));
}

void RTC::set_date_time(uint16_t  y, uint8_t  m, uint8_t d, 
                        uint8_t   h, uint8_t mm, uint8_t s,
                        WeekDay  wd)
{
  if (!present) return;
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
  if (!present) return;
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
 * @brief Set the date time from epoch relative time
 * 
 * The parameter *t* must represent a time >= 2000/01/01 00:00:00 and <= 2099/12/32 23:59:59.
 * 
 * @param t The time value as a number of seconds from 1970/01/01 00:00:00
 */
void 
RTC::set_date_time(const time_t * t)
{
  if (!present) return;
  struct tm time;
  if (gmtime_r(t, &time) == nullptr) return;

  uint16_t year = time.tm_year + 1900;
  if ((year < 2000) || (year > 2099)) return;

  set_date_time(year,                   (uint8_t)(time.tm_mon + 1), (uint8_t) time.tm_mday, 
                (uint8_t) time.tm_hour, (uint8_t) time.tm_min, (uint8_t) time.tm_sec,
                (RTC::WeekDay) time.tm_wday);
}

void
RTC::get_date_time(time_t * t)
{
  if (!present) return;
  struct tm time;
  uint16_t year = 2024;
  RTC::WeekDay wd = RTC::WeekDay::SUN;

  memset(&time, 0, sizeof(time));
  get_date_time(year,                     (uint8_t &) time.tm_mon, (uint8_t &) time.tm_mday,
                (uint8_t &) time.tm_hour, (uint8_t &) time.tm_min, (uint8_t &) time.tm_sec,
                wd);
  time.tm_year = year - 1900;
  time.tm_wday = (uint8_t) wd;
  time.tm_mon -= 1;
  if ((time.tm_mon < 0) || (time.tm_mon > 11)) time.tm_mon = 0;

  *t = timegm(&time);
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
  if (!present) return 0;
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
  if (!present) return;
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
  wire_device->cmd_write((uint8_t) Reg::OFFSET, data);
}

uint8_t RTC::read_calibration_reg(void) 
{
  return wire_device->cmd_read((uint8_t) Reg::OFFSET);
}


void RTC::set_ram(uint8_t value)
{
  if (!present) return;
  wire_device->cmd_write((uint8_t) Reg::RAM, value);
}

uint8_t RTC::get_ram(void) 
{
  if (!present) return 0;
  return wire_device->cmd_read((uint8_t) Reg::RAM);
}

RTC::CAPACITOR RTC::set_capacitor(CAPACITOR value) 
{
  if (!present) return (CAPACITOR) 0;
  uint8_t control_1 = wire_device->cmd_read((uint8_t) Reg::CTRL1);
  control_1 = (control_1 & 0xFE) | (0x01 & (uint8_t) value);
  wire_device->cmd_write((uint8_t) Reg::CTRL1, control_1);

  return (CAPACITOR) (wire_device->cmd_read((uint8_t) Reg::CTRL1) & 0x01);
}

RTC::CAPACITOR RTC::get_capacitor() 
{
  if (!present) return (CAPACITOR) 0;
  return (CAPACITOR) (wire_device->cmd_read((uint8_t) Reg::CTRL1) & 0x01);
}

