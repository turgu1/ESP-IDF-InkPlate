/*
pcal6416.cpp
Inkplate ESP-IDF

Modified by Guy Turcotte 
July 15, 2024
*/

#if PCAL6416

#include "pcal6416.hpp"
#include "esp_log.h"

#include "driver/gpio.h"

// LOW LEVEL:

bool
IOExpander::check_presence()
{
  if (!present) ESP_LOGE(TAG, "The PCAL6416 at address 0x%X has not been detected.", pcal_address);
  return present;
}

void 
IOExpander::test()
{
  if (!check_presence()) return;

  printf("Registers before read:\n");
  for (auto reg : registers) {
    printf("%02x ", reg);
  }
  printf("\n");
  fflush(stdout);

  read_all_registers();

  printf("Registers after read:\n");
  for (auto reg : registers) {
    printf("%02x ", reg);
  }
  printf("\n");
  fflush(stdout);
}

bool 
IOExpander::setup()
{
  ESP_LOGD(TAG, "Initializing...");
  
  wire_device = new WireDevice(pcal_address);
  present = (wire_device != nullptr) && wire_device->is_initialized();
  
  ESP_LOGI(TAG, "PCAL at address 0x%X has%s been detected", pcal_address, present ? "" : " NOT");

  if (present) {
    read_all_registers();
  }

  return present;
}

void 
IOExpander::read_all_registers()
{
  if (!check_presence()) return;

  wire_device->cmd_read(0x00, registers.data(), registers.size());

  // wire.begin_transmission(pcal_address);
  // wire.write(0x00);
  // wire.end_transmission();
  // wire.request_from(pcal_address, (uint8_t) 23);
  // for (auto & reg : registers) {
  //   reg = wire.read();
  // }
}

void 
IOExpander::read_registers(Reg first_reg, uint8_t count)
{
  if (!check_presence()) return;

  wire_device->cmd_read(reg_addresses[(int8_t)first_reg], &registers[first_reg], count);

  // wire.begin_transmission(pcal_address);
  // wire.write(reg_addresses[(int8_t)first_reg]);
  // wire.end_transmission();

  // wire.request_from(pcal_address, count);
  // for (int i = 0; i < count; ++i) {
  //   registers[R(first_reg, i)] = wire.read();
  // }
}

uint8_t 
IOExpander::read_register(Reg reg)
{
  if (!check_presence()) return 0;

  registers[reg] = wire_device->cmd_read(reg_addresses[(int8_t)reg]);

  // wire.begin_transmission(pcal_address);
  // wire.write(reg_addresses[(int8_t)reg]);
  // wire.end_transmission();
  // wire.request_from(pcal_address, (uint8_t) 1);
  // registers[reg] = wire.read();

  return registers[reg];
}

void 
IOExpander::update_all_registers()
{
  if (!check_presence()) return;

  wire_device->cmd_write(0x00, registers.data(), registers.size());

  // wire.begin_transmission(pcal_address);
  // wire.write(0x00);
  // for (auto reg : registers) {
  //   wire.write(reg);
  // }
  // wire.end_transmission();
}

void 
IOExpander::update_register(Reg reg, uint8_t value)
{
  if (!check_presence()) return;

  wire_device->cmd_write(reg_addresses[(int8_t)reg], value);

  // wire.begin_transmission(pcal_address);
  // wire.write(reg_addresses[(int8_t)reg]);
  // wire.write(value);
  // wire.end_transmission();
}

void 
IOExpander::update_registers(Reg first_reg, uint8_t count)
{
  if (!check_presence()) return;

  wire_device->cmd_write(reg_addresses[(int8_t)first_reg], &registers[first_reg], count);

  // wire.begin_transmission(pcal_address);
  // wire.write(reg_addresses[(int8_t)first_reg]);
  // for (int i = 0; i < count; ++i) {
  //   wire.write(registers[R(first_reg, i)]);
  // }
  // wire.end_transmission();
}

// HIGH LEVEL:

void
IOExpander::set_direction(Pin pin, PinMode mode)
{
  uint8_t port = ((uint8_t)pin >> 3) & 1;
  uint8_t p    =  (uint8_t)pin & 7;

  if (!check_presence()) return;

  switch (mode) {
    case PinMode::INPUT:
      registers[R(Reg::CONFA,   port)] |=   1 << p;  // Set it to input
      registers[R(Reg::PULLENA, port)] &= ~(1 << p); // Disable pullup on that pin
      update_register(R(Reg::CONFA,   port), registers[R(Reg::CONFA,   port)]);
      update_register(R(Reg::PULLENA, port), registers[R(Reg::PULLENA, port)]);
      break;

  case PinMode::INPUT_PULLUP:
      registers[R(Reg::CONFA,   port)] |= 1 << p; // Set it to input
      registers[R(Reg::PULLA,   port)] |= 1 << p; // Set it as pull-up
      registers[R(Reg::PULLENA, port)] |= 1 << p; // Enable pullup on that pin
      update_register(R(Reg::CONFA,   port), registers[R(Reg::CONFA,   port)]);
      update_register(R(Reg::PULLA,   port), registers[R(Reg::PULLA,   port)]);
      update_register(R(Reg::PULLENA, port), registers[R(Reg::PULLENA, port)]);
      break;

  case PinMode::OUTPUT:
      registers[R(Reg::CONFA,   port)] &= ~(1 << p); // Set it to output
      registers[R(Reg::OUTA,    port)] &= ~(1 << p); // Set its state to LOW
      registers[R(Reg::PULLENA, port)] &= ~(1 << p); // Disable pullup on that pin
      update_register(R(Reg::CONFA,   port), registers[R(Reg::CONFA,   port)]);
      update_register(R(Reg::OUTA,    port), registers[R(Reg::OUTA,    port)]);
      update_register(R(Reg::PULLENA, port), registers[R(Reg::PULLENA, port)]);
      break;
  }
}

void 
IOExpander::digital_write(Pin pin, SignalLevel state)
{
  uint8_t port = ((uint8_t)pin >> 3) & 1;
  uint8_t p    =  (uint8_t)pin & 7;

  if (registers[R(Reg::CONFA, port)] & (1 << p)) return;
  state == SignalLevel::HIGH ? (registers[R(Reg::OUTA, port)] |=  (1 << p)) : 
          (registers[R(Reg::OUTA, port)] &= ~(1 << p));
  update_register(R(Reg::OUTA, port), registers[R(Reg::OUTA, port)]);
}

IOExpander::SignalLevel 
IOExpander::digital_read(Pin pin)
{
  uint8_t port = ((uint8_t)pin >> 3) & 1;
  uint8_t p    =  (uint8_t)pin & 7;
  uint8_t r = read_register((Reg)((int8_t)Reg::INA + port));

  return (r & (1 << p)) ? SignalLevel::HIGH : SignalLevel::LOW;
}

void 
IOExpander::set_int_output(IntPort intPort, bool mirroring, bool openDrain, SignalLevel polarity)
{
  // Not supported for this chip
}

// Mode is not supported by the PCAL io extender. interrupts
// are produced at both raising and falling edges.
void 
IOExpander::set_int_pin(Pin pin, IntMode mode)
{
  uint8_t port = ((uint8_t)pin >> 3) & 1;
  uint8_t p    =  (uint8_t)pin & 7;

  registers[R(Reg::IMASKA, port)] &= ~(1 << p);
  update_register(R(Reg::IMASKA, port), registers[R(Reg::IMASKA, port)]);
}

void 
IOExpander::remove_int_pin(Pin pin)
{
  uint8_t port = ((uint8_t)pin >> 3) & 1;
  uint8_t p    =  (uint8_t)pin & 7;

  registers[R(Reg::IMASKA, port)] |= (1 << p);
  update_register(R(Reg::IMASKA, port), registers[R(Reg::IMASKA, port)]);
}

uint16_t 
IOExpander::get_int()
{
  read_register(Reg::IMASKA);
  read_register(Reg::IMASKB);
  return ((registers[Reg::IMASKB] << 8) | registers[Reg::IMASKA]);
}

uint16_t 
IOExpander::get_int_state()
{
  read_register(Reg::ISTATA);
  read_register(Reg::ISTATB);
  return ((registers[Reg::ISTATB] << 8) | registers[Reg::ISTATA]);
}

void 
IOExpander::set_ports(uint16_t values)
{
  registers[Reg::OUTA] = values & 0xff;
  registers[Reg::OUTB] = (values >> 8) & 0xff;
  update_register(Reg::OUTA, registers[Reg::OUTA]);
  update_register(Reg::OUTB, registers[Reg::OUTB]);
}

uint16_t 
IOExpander::get_ports()
{
  read_register(Reg::INA);
  read_register(Reg::INB);
  return ((registers[Reg::INB] << 8) | (registers[Reg::INA]));
}

#endif