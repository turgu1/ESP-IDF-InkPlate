#define __BATTERY__ 1
#include "battery.hpp"

#include "wire.hpp"

namespace {
constexpr double ADC_REFERENCE_VOLTAGE = 1.1;
constexpr double ADC_ATTENUATION_SCALE = 3.548133892;
constexpr double BATTERY_DIVIDER_SCALE = 2.0;
constexpr double ADC_MAX_READING       = 4095.0;
constexpr uint32_t DEFAULT_VREF_MV     = 1100;
} // namespace

Battery::~Battery() {
  if (calibration_enabled) {
    adc_cali_delete_scheme_line_fitting(adc_cali_handle);
  }
}

bool Battery::setup() {
  io_expander.set_direction(BATTERY_SWITCH, IOExpander::PinMode::OUTPUT);

  adc_unit_config = {
      .unit_id  = ADC_UNIT_1,
      .clk_src  = ADC_RTC_CLK_SRC_DEFAULT,
      .ulp_mode = ADC_ULP_MODE_DISABLE,
  };

  adc_oneshot_new_unit(&adc_unit_config, &adc_handle);

  adc_channel_config = {
      .atten    = ADC_ATTEN_DB_12,
      .bitwidth = ADC_BITWIDTH_12,
  };

  adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_7, &adc_channel_config);

  adc_cali_line_fitting_config_t calibration_config = {
      .unit_id      = ADC_UNIT_1,
      .atten        = ADC_ATTEN_DB_12,
      .bitwidth     = ADC_BITWIDTH_12,
      .default_vref = DEFAULT_VREF_MV,
  };

  calibration_enabled =
      adc_cali_create_scheme_line_fitting(&calibration_config, &adc_cali_handle) == ESP_OK;

  return true;
}

void Battery::set_voltage_trim(double value) { voltage_trim = value == 0 ? 1.0 : value; }

double Battery::read_level() {
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

  if (calibration_enabled) {
    int voltage_mv;
    if (adc_cali_raw_to_voltage(adc_cali_handle, adc_value, &voltage_mv) == ESP_OK) {
      return (double(voltage_mv) * BATTERY_DIVIDER_SCALE * voltage_trim) / 1000.0;
    }
  }

  return (double(adc_value) * ADC_REFERENCE_VOLTAGE * ADC_ATTENUATION_SCALE *
          BATTERY_DIVIDER_SCALE * voltage_trim) /
         ADC_MAX_READING;
}
