/*
mcp23017.cpp
Inkplate ESP-IDF

Modified by Guy Turcotte 
December 26, 2020

from the Arduino Library:

David Zovko, Borna Biro, Denis Vajak, Zvonimir Haramustek @ e-radionica.com
September 24, 2020
https://github.com/e-radionicacom/Inkplate-6-Arduino-library

For support, please reach over forums: forum.e-radionica.com/en
For more info about the product, please check: www.inkplate.io

This code is released under the GNU Lesser General Public License v3.0: https://www.gnu.org/licenses/lgpl-3.0.en.html
Please review the LICENSE file included with this example.
If you have any questions about licensing, please contact techsupport@e-radionica.com
Distributed as-is; no warranty is given.
*/

#define __MCP__ 1
#include "mcp23017.hpp"
#include "esp_log.h"

#include "wire.hpp"
#include "driver/gpio.h"

// LOW LEVEL:

bool
MCP23017::check_presence()
{
  if (!present) ESP_LOGE(TAG, "The MCP at address 0x%X has not been detected.", mcp_address);
  return present;
}

void 
MCP23017::test()
{
  if (!check_presence()) return;

  printf("Registers before read:\n");
  for (auto reg : registers) {
    printf("%02x ", reg);
  }
  printf("\n");
  fflush(stdout);

  wire.begin_transmission(mcp_address);
  wire.end_transmission();

  read_all_registers();

  printf("Registers after read:\n");
  for (auto reg : registers) {
    printf("%02x ", reg);
  }
  printf("\n");
  fflush(stdout);
}

bool 
MCP23017::setup()
{
  ESP_LOGD(TAG, "Initializing...");
  
  wire.begin_transmission(mcp_address);
  present = wire.end_transmission() == ESP_OK;
  
  ESP_LOGI(TAG, "MCP at address 0x%X has%s been detected", mcp_address, present ? "" : " NOT");

  read_all_registers();
  registers[Reg::IODIRA] = 0xff;
  registers[Reg::IODIRB] = 0xff;
  update_all_registers();

  return true;
}

void 
MCP23017::read_all_registers()
{
  if (!check_presence()) return;

  wire.begin_transmission(mcp_address);
  wire.write(0x00);
  wire.end_transmission();
  wire.request_from(mcp_address, (uint8_t) 22);
  for (auto & reg : registers) {
    reg = wire.read();
  }
}

void 
MCP23017::read_registers(Reg first_reg, uint8_t count)
{
  if (!check_presence()) return;

  wire.begin_transmission(mcp_address);
  wire.write((int8_t)first_reg);
  wire.end_transmission();

  wire.request_from(mcp_address, count);
  for (int i = 0; i < count; ++i) {
    registers[R(first_reg, i)] = wire.read();
  }
}

uint8_t 
MCP23017::read_register(Reg reg)
{
  if (!check_presence()) return 0;

  wire.begin_transmission(mcp_address);
  wire.write((int8_t)reg);
  wire.end_transmission();
  wire.request_from(mcp_address, (uint8_t) 1);
  registers[reg] = wire.read();

  return registers[reg];
}

void 
MCP23017::update_all_registers()
{
  if (!check_presence()) return;

  wire.begin_transmission(mcp_address);
  wire.write(0x00);
  for (auto reg : registers) {
    wire.write(reg);
  }
  wire.end_transmission();
}

void 
MCP23017::update_register(Reg reg, uint8_t value)
{
  if (!check_presence()) return;

  wire.begin_transmission(mcp_address);
  wire.write((int8_t)reg);
  wire.write(value);
  wire.end_transmission();
}

void 
MCP23017::update_registers(Reg first_reg, uint8_t count)
{
  if (!check_presence()) return;

  wire.begin_transmission(mcp_address);
  wire.write((int8_t)first_reg);
  for (int i = 0; i < count; ++i) {
    wire.write(registers[R(first_reg, i)]);
  }
  wire.end_transmission();
}

// HIGH LEVEL:

void
MCP23017::set_direction(Pin pin, PinMode mode)
{
  uint8_t port = ((uint8_t)pin >> 3) & 1;
  uint8_t p    =  (uint8_t)pin & 7;

  if (!check_presence()) return;

  switch (mode) {
    case PinMode::INPUT:
      registers[R(Reg::IODIRA, port)] |=   1 << p;  // Set it to input
      registers[R(Reg::GPPUA,  port)] &= ~(1 << p); // Disable pullup on that pin
      update_register(R(Reg::IODIRA, port), registers[R(Reg::IODIRA, port)]);
      update_register(R(Reg::GPPUA,  port), registers[R(Reg::GPPUA,  port)]);
      break;

  case PinMode::INPUT_PULLUP:
      registers[R(Reg::IODIRA, port)] |= 1 << p; // Set it to input
      registers[R(Reg::GPPUA,  port)] |= 1 << p; // Enable pullup on that pin
      update_register(R(Reg::IODIRA, port), registers[R(Reg::IODIRA, port)]);
      update_register(R(Reg::GPPUA,  port), registers[R(Reg::GPPUA,  port)]);
      break;

  case PinMode::OUTPUT:
      registers[R(Reg::IODIRA, port)] &= ~(1 << p); // Set it to output
      registers[R(Reg::GPPUA,  port)] &= ~(1 << p); // Disable pullup on that pin
      update_register(R(Reg::IODIRA, port), registers[R(Reg::IODIRA, port)]);
      update_register(R(Reg::GPPUA,  port), registers[R(Reg::GPPUA,  port)]);
      break;
  }
}

void 
MCP23017::digital_write(Pin pin, SignalLevel state)
{
  uint8_t port = ((uint8_t)pin >> 3) & 1;
  uint8_t p    =  (uint8_t)pin & 7;

  if (registers[R(Reg::IODIRA, port)] & (1 << p)) return;
  state == SignalLevel::HIGH ? (registers[R(Reg::GPIOA, port)] |=  (1 << p)) : 
          (registers[R(Reg::GPIOA, port)] &= ~(1 << p));
  update_register(R(Reg::GPIOA, port), registers[R(Reg::GPIOA, port)]);
}

MCP23017::SignalLevel 
MCP23017::digital_read(Pin pin)
{
  uint8_t port = ((uint8_t)pin >> 3) & 1;
  uint8_t p    =  (uint8_t)pin & 7;
  uint8_t r = read_register((Reg)((int8_t)Reg::GPIOA + port));

  return (r & (1 << p)) ? SignalLevel::HIGH : SignalLevel::LOW;
}

void 
MCP23017::set_int_output(IntPort intPort, bool mirroring, bool openDrain, SignalLevel polarity)
{
  Reg reg = intPort == IntPort::INTPORTA ? Reg::IOCONA : Reg::IOCONB;

  registers[reg] = (registers[reg] & ~(1 << 6)) | (mirroring << 6);
  registers[reg] = (registers[reg] & ~(1 << 2)) | (openDrain << 2);
  registers[reg] = (registers[reg] & ~(1 << 1)) | (polarity == SignalLevel::HIGH ? (1 << 1) : 0);
  
  update_register(reg, registers[reg]);
}

void 
MCP23017::set_int_pin(Pin pin, IntMode mode)
{
  uint8_t port = ((uint8_t)pin >> 3) & 1;
  uint8_t p    =  (uint8_t)pin & 7;

  switch (mode) {
    case IntMode::CHANGE:
      registers[R(Reg::INTCONA, port)] &= ~(1 << p);
      break;

    case IntMode::FALLING:
      registers[R(Reg::INTCONA, port)] |= (1 << p);
      registers[R(Reg::DEFVALA, port)] |= (1 << p);
      break;

    case IntMode::RISING:
      registers[R(Reg::INTCONA, port)] |=  (1 << p);
      registers[R(Reg::DEFVALA, port)] &= ~(1 << p);
      break;
  }
  registers[R(Reg::GPINTENA, port)] |= (1 << p);
  update_registers(Reg::GPINTENA, 6);
}

void 
MCP23017::remove_int_pin(Pin pin)
{
  uint8_t port = ((uint8_t)pin >> 3) & 1;
  uint8_t p    =  (uint8_t)pin & 7;
  registers[R(Reg::GPINTENA, port)] &= ~(1 << p);
  update_registers(Reg::GPINTENA, 2);
}

uint16_t 
MCP23017::get_int()
{
  read_registers(Reg::INTFA, 2);
  return ((registers[Reg::INTFB] << 8) | registers[Reg::INTFA]);
}

uint16_t 
MCP23017::get_int_state()
{
  read_registers(Reg::INTCAPA, 2);
  return ((registers[Reg::INTCAPB] << 8) | registers[Reg::INTCAPA]);
}

void 
MCP23017::set_ports(uint16_t values)
{
  registers[Reg::GPIOA] = values & 0xff;
  registers[Reg::GPIOB] = (values >> 8) & 0xff;
  update_registers(Reg::GPIOA, 2);
}

uint16_t 
MCP23017::get_ports()
{
  read_registers(Reg::GPIOA, 2);
  return ((registers[Reg::GPIOB] << 8) | (registers[Reg::GPIOA]));
}