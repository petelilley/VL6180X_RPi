#include <VL6180X.h>
#include <iostream>
#include <fcntl.h>
#include <cstdio>
#include <cstring>
#include <cerrno>

// My favorite register addresses :D
#define REG_IDENTIFICATION_MODEL_ID 0x00
#define REG_SYSTEM_FRESH_OUT_OF_RESET 0x0016
#define REG_RESULT_RANGE_STATUS 0x004d
#define REG_RESULT_INTERRUPT_STATUS_GPIO 0x004f
#define REG_SYSTEM_INTERRUPT_CLEAR 0x0015
#define REG_SYSRANGE_START 0x0018
#define REG_RESULT_RANGE_VAL 0x0062

VL6180X::VL6180X(int adapter_nr, uint8_t addr) {
  std::string filename = "/dev/i2c-" + std::to_string(adapter_nr);
  
  i2c_fd = open(filename.c_str(), O_RDWR);
  connected = true;
  if (i2c_fd < 0) {
    connected = false;
    std::cout << "VL6180X: Failed to open I2C device at '" << filename << "'\n\t" << strerror(errno) << '\n';
    return;
  }
  else {
    std::cout << "VL6180X: Successfully opened I2C device at '" << filename << "'\n";
  }
  
  if (ioctl(i2c_fd, I2C_SLAVE, addr) < 0) {
    connected = false;
    std::cout << "VL6180X: Failed to acquire I2C bus address and/or talk to slave.\n\t" << strerror(errno) << '\n';
    return;
  }
  
  // Check model id.
  if (read_register(REG_IDENTIFICATION_MODEL_ID) != 0xB4) {
    connected = false;
    std::cout << "VL6180X: Invalid I2C Model ID\n";
    return;
  }
  
  // Read SYSTEM__FRESH_OUT_OF_RESET register.
  if (!(read_register(REG_SYSTEM_FRESH_OUT_OF_RESET) & 0x01)) {
      // Ideally we'd reset the device by applying logic '0' to GPIO0, but we can't without that wired up.
    std::cout << "VL6180X: Not fresh out of reset\n";
    return;
  }
  
  write_register(REG_SYSTEM_FRESH_OUT_OF_RESET, 0x00);
  
  // Apply the tuning settings.
  write_register(0x0207, 0x01);
  write_register(0x0208, 0x01);
  write_register(0x0096, 0x00);
  write_register(0x0097, 0xfd);
  write_register(0x00e3, 0x00);
  write_register(0x00e4, 0x04);
  write_register(0x00e5, 0x02);
  write_register(0x00e6, 0x01);
  write_register(0x00e7, 0x03);
  write_register(0x00f5, 0x02);
  write_register(0x00d9, 0x05);
  write_register(0x00db, 0xce);
  write_register(0x00dc, 0x03);
  write_register(0x00dd, 0xf8);
  write_register(0x009f, 0x00);
  write_register(0x00a3, 0x3c);
  write_register(0x00b7, 0x00);
  write_register(0x00bb, 0x3c);
  write_register(0x00b2, 0x09);
  write_register(0x00ca, 0x09);
  write_register(0x0198, 0x01);
  write_register(0x01b0, 0x17);
  write_register(0x01ad, 0x00);
  write_register(0x00ff, 0x05);
  write_register(0x0100, 0x05);
  write_register(0x0199, 0x05);
  write_register(0x01a6, 0x1b);
  write_register(0x01ac, 0x3e);
  write_register(0x01a7, 0x1f);
  write_register(0x0030, 0x00);
  
  write_register(0x0011, 0x10);
  write_register(0x010a, 0x30);
  write_register(0x003f, 0x46);
  write_register(0x0031, 0xFF);
  write_register(0x0041, 0x63);
  write_register(0x002e, 0x01);
  write_register(0x001b, 0x09);
  write_register(0x003e, 0x31);
  write_register(0x0014, 0x24);
}

VL6180X::~VL6180X() {
  if (connected) close(i2c_fd);
}

uint8_t VL6180X::get_range() {
  if (!connected) return 0xFF;
  
  if (!is_measuring) {
    uint8_t status = read_register(REG_RESULT_RANGE_STATUS);
    // Read range status register.
    if ((status & 0x01) == 0x01) {
      // Start a range measurement.
      write_register(REG_SYSRANGE_START, 0x01);
      
      is_measuring = true;
    }
    else {
      std::cout << "VL6180X_ToF 0x4d Status Error " << status << '\n';
    }
  }
  
  if (is_measuring) {
    if ((read_register(REG_RESULT_INTERRUPT_STATUS_GPIO) & 0x04) == 0x04) {
      // Read range register (mm).
      range = read_register(REG_RESULT_RANGE_VAL);
      
      // Clear the interrupt.
      write_register(REG_SYSTEM_INTERRUPT_CLEAR, 0x07);
      
      is_measuring = false;
    }
  }
  
  return range;
}

uint8_t VL6180X::read_register(uint16_t reg) {
  if (!connected) return 0;
  
  uint8_t buffer[3];
  buffer[0] = uint8_t(reg >> 8);
  buffer[1] = uint8_t(reg & 0xFF);
  buffer[2] = 0;
  
  // Prompt the sensor with the address to read.
  write(i2c_fd, buffer, 2);
  
  // Read the value at the address.
  read(i2c_fd, &buffer[2], 1);
  return buffer[2];
}

void VL6180X::write_register(uint16_t reg, uint8_t data) {
  if (!connected) return;
  
  uint8_t buffer[3];
  
  buffer[0] = uint8_t(reg >> 8);
  buffer[1] = uint8_t(reg & 0xFF);
  buffer[2] = data;
  
  // Write data to register at address.
  write(i2c_fd, buffer, 3);
}
