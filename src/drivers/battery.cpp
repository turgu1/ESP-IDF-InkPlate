#define __BATTERY__ 1
#include "battery.hpp"

#include "wire.hpp"

bool
Battery::setup()
{
  io_expander.set_direction(BATTERY_SWITCH, IOExpander::PinMode::OUTPUT);

  adc_unit_config = {
    .unit_id = ADC_UNIT_1,
    .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
    .ulp_mode = ADC_ULP_MODE_DISABLE,
  };

  adc_oneshot_new_unit(&adc_unit_config, &adc_handle);

  adc_channel_config = {
    .atten = ADC_ATTEN_DB_12,
    .bitwidth = ADC_BITWIDTH_12,
  };

  adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_7, &adc_channel_config);

  return true;
}

double 
Battery::read_level()
{
  Wire::enter();
  io_expander.digital_write(BATTERY_SWITCH, IOExpander::SignalLevel::HIGH);
  Wire::leave();

  ESP::delay(1);
  


  // adc1_config_width(ADC_WIDTH_BIT_12);
  // adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11);

  // int16_t adc = ESP::analog_read(ADC1_CHANNEL_7); // ADC 1 Channel 7 is GPIO port 35
  
  int adc_value;
  adc_oneshot_read(adc_handle, ADC_CHANNEL_7, &adc_value);

  Wire::enter();
  io_expander.digital_write(BATTERY_SWITCH, IOExpander::SignalLevel::LOW);
  Wire::leave();

  return (double(adc_value) * 1.1 * 3.548133892 * 2) / 4095.0;
}
