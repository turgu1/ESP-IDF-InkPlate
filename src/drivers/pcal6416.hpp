/*

pcal6416.hpp
Inkplate ESP-IDF

Created by Guy Turcotte 
July 15, 2024

This is a driver for the IO Expander PCAL6416. The driver has been designed to be
as close as possible to the MCP23017 driver interface, such that it will be
interchangeable for the rest of the ESP-IDF-Inkplate project.

As such, both MCP23017 and PCAL6416 driver classes are named IOExpander. The
right driver is being expanded as needed through the macro definitions PCAL6416 or MCP23017.
As such, only one of the two macros must be defined to be equal to 1

*/

#if PCAL6416

#pragma once

#include <cinttypes>
#include <cstring>

#include "non_copyable.hpp"
#include "esp.hpp"

#include <array>

// this is a new kind of array which accepts and requires its indices to be enums
template<typename E, class T, std::size_t N>
class enum_array : public std::array<T, N> {
public:
    T & operator[] (E e) {
        return std::array<T, N>::operator[]((std::size_t)e);
    }

    const T & operator[] (E e) const {
        return std::array<T, N>::operator[]((std::size_t)e);
    }
};

class IOExpander : NonCopyable
{
  private:
    static constexpr char const * TAG = "PCAL6416";

    uint8_t reg_addresses[23] = {
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x40, 0x41, 
      0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 
      0x4c, 0x4d, 0x4f
    };

    enum class Reg : uint8_t {
      INA        = 0x00,
      INB        = 0x01,
      OUTA       = 0x02,
      OUTB       = 0x03,
      INVERTA    = 0x04,
      INVERTB    = 0x05,
      CONFA      = 0x06,
      CONFB      = 0x07,
      STRENGHTAL = 0x08,
      STRENGTHAH = 0x09,
      STRENGTHBL = 0x0A,
      STRENGTHBH = 0x0B,
      INLATCHA   = 0x0C,
      INLATCHB   = 0x0D,
      PULLENA    = 0x0E,
      PULLENB    = 0x0F,
      PULLA      = 0x10,
      PULLB      = 0x11,
      IMASKA     = 0x12,
      IMASKB     = 0x13,
      ISTATA     = 0x14,
      ISTATB     = 0x15,
      OUTCONF    = 0x16
    };

    const uint8_t pcal_address;
    enum_array<Reg, uint8_t, 23> registers;

    bool present;
 
    // Adjust Register, adding offset p
    inline Reg R(Reg r, uint8_t p) { return (Reg)((uint8_t)r + p); }

    void   read_all_registers();
    void       read_registers(Reg first_reg, uint8_t count);
    uint8_t     read_register(Reg reg);
    
    void update_all_registers();
    void      update_register(Reg reg,       uint8_t  value);
    void     update_registers(Reg first_reg, uint8_t  count);

  public:

    IOExpander(uint8_t address) : pcal_address(address), present(false) { 
      std::fill(registers.begin(), registers.end(), 0); 
    }

    enum class PinMode     : uint8_t { INPUT,    INPUT_PULLUP, OUTPUT };
    enum class IntMode     : uint8_t { CHANGE,   FALLING,      RISING };
    enum class IntPort     : uint8_t { INTPORTA, INTPORTB             };
    enum class SignalLevel : uint8_t { LOW,      HIGH                 };

    enum class Pin : uint8_t {
      IOPIN_0,
      IOPIN_1,
      IOPIN_2,
      IOPIN_3,
      IOPIN_4,
      IOPIN_5,
      IOPIN_6,
      IOPIN_7,
      IOPIN_8,
      IOPIN_9,
      IOPIN_10,
      IOPIN_11,
      IOPIN_12,
      IOPIN_13,
      IOPIN_14,
      IOPIN_15
    };

    inline bool is_present() { return present; }
    bool check_presence();
    
    // BEFORE CALLING ANY OF THE FOLLOWING METHODS, ENSURE THAT THE Wire I2C
    // CLASS IS PROTECTED THROUGH THE USE OF Wire::enter() and Wire::leave() METHODS.
    
    void test();
    
    bool setup();

    void        set_direction(Pin pin, PinMode     mode );
    void        digital_write(Pin pin, SignalLevel state);
    SignalLevel  digital_read(Pin pin);

    void       set_int_output(IntPort intPort, bool mirroring, bool openDrain, SignalLevel polarity);
    void          set_int_pin(Pin pin, IntMode mode);
    void       remove_int_pin(Pin pin);

    uint16_t          get_int();
    uint16_t    get_int_state();

    void            set_ports(uint16_t values);
    uint16_t        get_ports();
};

#endif