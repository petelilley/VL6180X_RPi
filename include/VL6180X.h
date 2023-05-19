#pragma once

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdint>

class VL6180X {
public:
  enum {
    DEFAULT_ADDR = 0x29,
  };
  
  VL6180X(int adapter_nr, uint8_t addr = DEFAULT_ADDR);
  ~VL6180X();
  
  /**
   * @brief Returns whether the device is connected.
   *
   * @return Whether the device is connected.
   */
  constexpr bool is_connected() const { return connected; }
  
  /**
   * @brief Returns the range reading of the sensor (mm).
   *
   * @return The range reading of the sensor (mm).
   */
  uint8_t get_range();
  
private:
  
  /**
   * @brief Reads an 8-bit value from a 16-bit register location over I2C.
   *
   * @param reg The 16-bit register address.
   * @return The 8-bit register value.
   */
  uint8_t read_register(uint16_t reg);
  
  /**
   * @brief Writes an 8-bit value to a 16-bit register location over I2C.
   *
   * @param reg The 16-bit register address.
   * @param data The 8-bit register value.
   */
  void write_register(uint16_t reg, uint8_t data);
  
  // File descriptor of the I2C device.
  int i2c_fd;
  
  // Whether the device is connected.
  bool connected;
  
  uint8_t range = 0xFF;
  bool is_measuring = false;
};
