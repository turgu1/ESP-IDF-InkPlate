#if INKPLATE_6FLICK

#include "touch_screen_cypress.hpp"
#include "esp.hpp"

#include "driver/gpio.h"

static TouchScreen::ISRHandlerPtr app_isr_handler;
static volatile bool touchscreen_interrupt_happened = false;

static void IRAM_ATTR 
touchscreen_isr(void * value) 
{
  touchscreen_interrupt_happened = true;

  if (app_isr_handler != nullptr) (* app_isr_handler)(value);
}

bool 
TouchScreen::setup(bool power_on, ISRHandlerPtr isr_handler)
{
  ready                          = false;

  app_isr_handler                = nullptr;

  Wire::enter();

  io_expander.set_direction(TOUCHSCREEN_ENABLE, IOExpander::PinMode::OUTPUT );
  io_expander.set_direction(TOUCHSCREEN_RESET,  IOExpander::PinMode::OUTPUT );
  io_expander.digital_write(TOUCHSCREEN_ENABLE, IOExpander::SignalLevel::LOW); // off

  Wire::leave();

   if (power_on) {
    set_power_state(power_on);

    Wire::enter();

    hardware_reset();

    uint8_t distance_default[] = { 0xF8 };

    if (ping(5) &&
        send_command(SOFT_RESET_MODE) && 
        get_boot_loader_data(boot_loader_data) &&
        exit_boot_loader_mode() &&
        get_sys_info(sys_info_data) &&
        set_sys_info_regs(sys_info_data) &&
        send_command(OPERATE_MODE) &&
        write(DETECTION_DISTANCE, distance_default, 1)) {

      Wire::leave();

      ESP::delay(50);

      gpio_config_t io_conf;

      io_conf.intr_type    = GPIO_INTR_NEGEDGE; 
      io_conf.pin_bit_mask = 1ULL << INTERRUPT_PIN;
      io_conf.mode         = GPIO_MODE_INPUT;
      io_conf.pull_up_en   = GPIO_PULLUP_DISABLE;
      io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;

      gpio_config(&io_conf);

      gpio_install_isr_service(0);
      
      gpio_isr_handler_add(
        INTERRUPT_PIN, 
        touchscreen_isr, 
        (void *) INTERRUPT_PIN);

      if (isr_handler != nullptr) set_app_isr_handler(isr_handler);
      touchscreen_interrupt_happened = false;
      ready = true;

      ESP_LOGI(TAG, "Initialization completed.");
    }
    else {
      Wire::leave();

      ESP_LOGE(TAG, "Unable to complete initialization!");
      shutdown();
    }
  }

  return ready;
}

//ok
void 
TouchScreen::set_app_isr_handler(ISRHandlerPtr isr_handler)
{
  gpio_intr_disable(INTERRUPT_PIN);
  app_isr_handler = isr_handler;
  gpio_intr_enable(INTERRUPT_PIN);
}

//ok
void
TouchScreen::hardware_reset()
{
  io_expander.digital_write(TOUCHSCREEN_RESET, IOExpander::SignalLevel::HIGH); ESP::delay(10); 
  io_expander.digital_write(TOUCHSCREEN_RESET, IOExpander::SignalLevel::LOW ); ESP::delay(2);
  io_expander.digital_write(TOUCHSCREEN_RESET, IOExpander::SignalLevel::HIGH); ESP::delay(10); 
}

bool
TouchScreen::software_reset()
{
  send_command(SOFT_RESET_MODE);

  return true;
}

//ok
uint8_t
TouchScreen::get_position(TouchPositions & x_positions, TouchPositions & y_positions)
{
  uint8_t raw[sizeof(SysInfoData)];

  Wire::enter();
  if (touchscreen_interrupt_happened) handshake();
  bool res = read(BASE_ADDR, raw, sizeof(SysInfoData));
  Wire::leave();

  uint8_t fingers = res ? raw[2] : 0;

  x_positions[0] = (raw[3] << 8) | raw[4];
  y_positions[0] = (raw[5] << 8) | raw[6];
  x_positions[1] = (raw[9] << 8) | raw[10];
  y_positions[1] = (raw[11] << 8) | raw[12];

  touchscreen_interrupt_happened = false;

  ESP_LOGI(TAG, "Pos: [%" PRIu16 ",%" PRIu16 "], [%" PRIu16 ",%" PRIu16 "], Fingers: %" PRIu8, 
    x_positions[0], y_positions[0], x_positions[1], y_positions[1], fingers);

  return fingers;
}

//ok
void 
TouchScreen::set_power_state(bool on_state)
{
  Wire::enter();
  if (on_state) {
    io_expander.digital_write(TOUCHSCREEN_ENABLE, IOExpander::SignalLevel::HIGH);
    ESP::delay(50);
    io_expander.digital_write(TOUCHSCREEN_RESET, IOExpander::SignalLevel::HIGH);
    ESP::delay(50);

  } 
  else {
    io_expander.digital_write(TOUCHSCREEN_ENABLE, IOExpander::SignalLevel::LOW);
    ESP::delay(50);
    io_expander.digital_write(TOUCHSCREEN_RESET, IOExpander::SignalLevel::LOW);

  } 
  Wire::leave();
}

//ok
bool
TouchScreen::get_power_state()
{
  bool power_is_on = io_expander.digital_read(TOUCHSCREEN_ENABLE) == IOExpander::SignalLevel::HIGH;

  return power_is_on;
}

bool
TouchScreen::read(uint8_t cmd, uint8_t (& data)[], uint8_t size) {

  wire.begin_transmission(TOUCHSCREEN_ADDRESS);
  wire.write(cmd);
  if (wire.end_transmission() != ESP_OK) return false;

  // read not more than 32 bytes at a time
  int index = 0;
  while (size > 0) {
    int len = size > 32 ? 32 : size;

    if (wire.request_from(TOUCHSCREEN_ADDRESS, len) != ESP_OK) return false;

    for (int i = 0; i < len; i++, index++) {
      data[index] = wire.read();
    }

    size -= len;
  }

  return true;
}

bool 
TouchScreen::write(uint8_t cmd, uint8_t(& data)[], uint8_t size)
{
  wire.begin_transmission(TOUCHSCREEN_ADDRESS);
  wire.write(cmd);
  for (int i = 0; i < size; i++) {
    wire.write(data[i]);
  }
  return wire.end_transmission() == ESP_OK;
}

//ok
bool 
TouchScreen::send_command(uint8_t cmd)
{
  wire.begin_transmission(TOUCHSCREEN_ADDRESS);
  wire.write(BASE_ADDR);
  wire.write(cmd);
  bool res = wire.end_transmission() == ESP_OK; 

  ESP::delay(20);

  return res;
}

void
TouchScreen::shutdown()
{
  set_power_state(false);

  gpio_isr_handler_remove(INTERRUPT_PIN);
  app_isr_handler = nullptr;
  touchscreen_interrupt_happened = false;

  ready = false;
}

bool
TouchScreen::is_screen_touched()
{
  return touchscreen_interrupt_happened;
}

bool 
TouchScreen::get_boot_loader_data(BootLoaderData & bl_data)
{
    uint8_t raw[sizeof(BootLoaderData)];

    if (!read(BASE_ADDR, raw, sizeof(raw))) {
        return false;

    }

    memcpy(&bl_data, raw, sizeof(raw));

    return true;
}

bool 
TouchScreen::exit_boot_loader_mode()
{
    // Bootloader command array.
    uint8_t command_data[] = {
        0x00,                     // File offset.
        0xFF,                     // Command.
        0xA5,                     // Exit bootloader command.
        0, 1, 2, 3, 4, 5, 6, 7    // Default keys.
    };

    write(BASE_ADDR, command_data, sizeof(command_data));

    // Wait a little bit - Must be long delay, otherwise setSysInfoMode will fail!
    // Delay of 150ms will fail - tested!
    ESP::delay(500);

    BootLoaderData boot_loader_data;
    if (!get_boot_loader_data(boot_loader_data)) return false;

    #define GET_BOOTLOADERMODE(reg) (((reg) & 0x10) >> 4)

    // Check for validity.
    if (GET_BOOTLOADERMODE(boot_loader_data.bl_status) == 1) {
      // Still in boot loader mode... not good
      return false;
    }

    // If everything went ok return true.
    return true;
}

bool
TouchScreen::get_sys_info(SysInfoData &sys_info_data)
{
    // Change mode to system info.
    send_command(SYS_INFO_MODE);

    // Wait a bit.
    ESP::delay(20);

    // Buffer for the system info data.
    uint8_t sys_info_array[sizeof(SysInfoData)];

    // Read the registers.
    if (!read(BASE_ADDR, sys_info_array, sizeof(SysInfoData))) {
        return false;
    }

    // Copy into struct typedef.
    memcpy(&sys_info_data, sys_info_array, sizeof(SysInfoData));

    // Do a handshake!
    handshake();

    // Check TTS version. If is zero, something went wrong.
    if (!(sys_info_data.tts_verh || sys_info_data.tts_verl)) {
        return false;
    }

    // Everything went ok? Return true for success.
    return true;
}

bool 
TouchScreen::set_sys_info_regs(SysInfoData &sys_info_data)
{
    // Modify registers to the default values.
    sys_info_data.act_intrvl = ACT_INTERVAL_DEFAULT;
    sys_info_data.tch_tmout = TOUCH_TIMEOUT_DEFAULT;
    sys_info_data.lp_intrvl = LOW_POWER_INTERVAL_DEFAULT;

    // Pack them into array.
    uint8_t regs[] = {
      ACT_INTERVAL_DEFAULT, TOUCH_TIMEOUT_DEFAULT, LOW_POWER_INTERVAL_DEFAULT
    };

    // Send the registers to the I2C. Check if failed. If failed, return false.
    if (!write(REG_ACT_INTERVAL, regs, 3))
        return false;

    // Wait a little bit.
    ESP::delay(20);

    // Everything went ok? Return true for success.
    return true;
}

void 
TouchScreen::handshake()
{
    // Read the hst_mode register (address 0x00).
    uint8_t host_mode_reg[] = { 0 };
    read(BASE_ADDR, host_mode_reg, 1);
    host_mode_reg[0] ^= 0x80;
    write(BASE_ADDR, host_mode_reg, 1);
}

bool 
TouchScreen::ping(int retries)
{
    // Try to ping multiple times in a row (just in case TSC is not in low power mode).
    // Delay between retires is 20ms (just a wildguess, don't have any documentation).
    while (retries-- > 0) {
        // Ping the TSC (touchscreen controller) on I2C.
        wire.begin_transmission(TOUCHSCREEN_ADDRESS);
        if (wire.end_transmission() == ESP_OK) {
          return true;
        }

        // TSC not found? Try again, but before retry wait a little bit.
        ESP::delay(20);
    }

    // Got here? Not good, TSC not found, return error.
    return false;
}

#endif

