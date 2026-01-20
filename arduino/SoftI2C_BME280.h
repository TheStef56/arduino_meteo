#include <Arduino.h>
#ifndef SOFTI2C_BME280_H
#define SOFTI2C_BME280_H
// Single-file library header for SoftI2C + BME280 driver
// Use SoftI2C_BME280 class to begin(sda, scl, addr) and read temperature/pressure/humidity.

class SoftI2C_BME280 {
public:
  // create with default safe delay (8 us)
  SoftI2C_BME280(uint16_t delayUs = 8);

  // initialize using SDA pin, SCL pin, address (0x76 or 0x77)
  bool begin(uint8_t sdaPin, uint8_t sclPin, uint8_t address = 0x76);

  // read compensated values
  float readTemperature(); // °C
  float readPressure();    // hPa
  float readHumidity();    // % RH

  // convenience: read all (T in °C, P in hPa, H in %)
  bool readAll(float &T, float &P, float &H);

private:
  // Soft I2C params
  uint8_t _sdaPin;
  uint8_t _sclPin;
  uint16_t _delayUs;
  uint8_t _addr;

  // calibration data
  uint16_t dig_T1;
  int16_t  dig_T2;
  int16_t  dig_T3;

  uint16_t dig_P1;
  int16_t  dig_P2;
  int16_t  dig_P3;
  int16_t  dig_P4;
  int16_t  dig_P5;
  int16_t  dig_P6;
  int16_t  dig_P7;
  int16_t  dig_P8;
  int16_t  dig_P9;

  uint8_t  dig_H1;
  int16_t  dig_H2;
  uint8_t  dig_H3;
  int16_t  dig_H4;
  int16_t  dig_H5;
  int8_t   dig_H6;

  int32_t t_fine;
  bool _initialized;

  // low-level line control
  inline void sdaRelease() { pinMode(_sdaPin, INPUT_PULLUP); } // release (pull-up)
  inline void sdaDriveLow() { pinMode(_sdaPin, OUTPUT); digitalWrite(_sdaPin, LOW); }
  inline bool readSDA() { pinMode(_sdaPin, INPUT); return digitalRead(_sdaPin); }

  inline void sclRelease() { pinMode(_sclPin, INPUT_PULLUP); delayMicroseconds(_delayUs); }
  inline void sclDriveLow() { pinMode(_sclPin, OUTPUT); digitalWrite(_sclPin, LOW); delayMicroseconds(_delayUs); }

  // I2C primitives
  void i2cStart();
  void i2cStop();
  bool i2cWriteByte(uint8_t b);               // returns true if ACK received
  uint8_t i2cReadByte(bool ack);

  // register helpers
  bool writeRegister(uint8_t reg, uint8_t value);
  bool readRegisters(uint8_t reg, uint8_t *buf, size_t len);
  uint8_t readRegister(uint8_t reg);

  // calibration & raw reads
  bool readCalibration();
  uint32_t readRawTemperature();
  uint32_t readRawPressure();
  uint16_t readRawHumidity();

  // config
  void writeDefaultConfig();
};

// BME280 register addresses (common)
#define BME280_REG_ID        0xD0
#define BME280_REG_RESET     0xE0
#define BME280_REG_CTRL_HUM  0xF2
#define BME280_REG_STATUS    0xF3
#define BME280_REG_CTRL_MEAS 0xF4
#define BME280_REG_CONFIG    0xF5
#define BME280_REG_PRESS_MSB 0xF7
#define BME280_REG_TEMP_MSB  0xFA
#define BME280_REG_HUM_MSB   0xFD

SoftI2C_BME280::SoftI2C_BME280(uint16_t delayUs) {
  _delayUs = delayUs;
  _initialized = false;
  _sdaPin = _sclPin = 255;
  _addr = 0x76;
}

bool SoftI2C_BME280::begin(uint8_t sdaPin, uint8_t sclPin, uint8_t address) {
  _sdaPin = sdaPin;
  _sclPin = sclPin;
  _addr = address;

  // Start pins released (use internal pull-ups; external 4.7k recommended)
  pinMode(_sdaPin, INPUT_PULLUP);
  pinMode(_sclPin, INPUT_PULLUP);
  delay(24);

  // check device ID
  uint8_t id = readRegister(BME280_REG_ID);
  if (id != 0x60) return false;

  // soft reset
  writeRegister(BME280_REG_RESET, 0xB6);
  delay(24);

  // read calibration
  if (!readCalibration()) return false;

  // write config/oversampling and set normal mode
  writeDefaultConfig();

  _initialized = true;
  return true;
}

// --- low level bitbang I2C ---

void SoftI2C_BME280::i2cStart() {
  sdaRelease();
  sclRelease();
  delayMicroseconds(_delayUs);
  sdaDriveLow();
  delayMicroseconds(_delayUs);
  sclDriveLow();
  delayMicroseconds(_delayUs);
}

void SoftI2C_BME280::i2cStop() {
  sdaDriveLow();
  delayMicroseconds(_delayUs);
  sclRelease();
  delayMicroseconds(_delayUs);
  sdaRelease();
  delayMicroseconds(_delayUs);
}

// write one byte, return true if ACK (ACK is SDA low)
bool SoftI2C_BME280::i2cWriteByte(uint8_t b) {
  for (int i = 7; i >= 0; --i) {
    if (b & (1 << i)) sdaRelease(); else sdaDriveLow();
    delayMicroseconds(_delayUs);
    sclRelease();
    delayMicroseconds(_delayUs);
    sclDriveLow();
    delayMicroseconds(_delayUs);
  }
  // ACK bit
  sdaRelease(); // release for ACK from slave
  delayMicroseconds(_delayUs);
  sclRelease();
  delayMicroseconds(_delayUs);
  bool ack = !readSDA(); // ack = SDA pulled low
  sclDriveLow();
  delayMicroseconds(_delayUs);
  return ack;
}

uint8_t SoftI2C_BME280::i2cReadByte(bool ack) {
  uint8_t v = 0;
  sdaRelease();
  for (int i = 7; i >= 0; --i) {
    sclRelease();
    delayMicroseconds(_delayUs);
    if (readSDA()) v |= (1 << i);
    sclDriveLow();
    delayMicroseconds(_delayUs);
  }
  // send ACK/NACK
  if (ack) sdaDriveLow(); else sdaRelease();
  delayMicroseconds(_delayUs);
  sclRelease();
  delayMicroseconds(_delayUs);
  sclDriveLow();
  sdaRelease();
  delayMicroseconds(_delayUs);
  return v;
}

// --- register helpers ---

bool SoftI2C_BME280::writeRegister(uint8_t reg, uint8_t value) {
  i2cStart();
  if (!i2cWriteByte((_addr << 1) | 0)) { i2cStop(); return false; }
  if (!i2cWriteByte(reg)) { i2cStop(); return false; }
  if (!i2cWriteByte(value)) { i2cStop(); return false; }
  i2cStop();
  return true;
}

bool SoftI2C_BME280::readRegisters(uint8_t reg, uint8_t *buf, size_t len) {
  i2cStart();
  if (!i2cWriteByte((_addr << 1) | 0)) { i2cStop(); return false; }
  if (!i2cWriteByte(reg)) { i2cStop(); return false; }
  // repeated start
  i2cStart();
  if (!i2cWriteByte((_addr << 1) | 1)) { i2cStop(); return false; }
  for (size_t i = 0; i < len; ++i) {
    bool ack = (i < (len - 1));
    buf[i] = i2cReadByte(ack);
  }
  i2cStop();
  return true;
}

uint8_t SoftI2C_BME280::readRegister(uint8_t reg) {
  uint8_t v = 0;
  if (!readRegisters(reg, &v, 1)) return 0;
  return v;
}

// --- calibration ---

bool SoftI2C_BME280::readCalibration() {
  uint8_t buf[26];
  // read 0x88..0xA1 (26 bytes)
  if (!readRegisters(0x88, buf, 26)) return false;

  dig_T1 = (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
  dig_T2 = (int16_t) (buf[2] | (buf[3] << 8));
  dig_T3 = (int16_t) (buf[4] | (buf[5] << 8));

  dig_P1 = (uint16_t)buf[6] | ((uint16_t)buf[7] << 8);
  dig_P2 = (int16_t) (buf[8] | (buf[9] << 8));
  dig_P3 = (int16_t) (buf[10] | (buf[11] << 8));
  dig_P4 = (int16_t) (buf[12] | (buf[13] << 8));
  dig_P5 = (int16_t) (buf[14] | (buf[15] << 8));
  dig_P6 = (int16_t) (buf[16] | (buf[17] << 8));
  dig_P7 = (int16_t) (buf[18] | (buf[19] << 8));
  dig_P8 = (int16_t) (buf[20] | (buf[21] << 8));
  dig_P9 = (int16_t) (buf[22] | (buf[23] << 8));

  dig_H1 = buf[25];

  // read 0xE1..0xE7 (7 bytes)
  uint8_t buf2[7];
  if (!readRegisters(0xE1, buf2, 7)) return false;

  dig_H2 = (int16_t)(buf2[0] | (buf2[1] << 8));
  dig_H3 = buf2[2];
  int16_t e4 = buf2[3];
  int16_t e5 = buf2[4];
  int16_t e6 = buf2[5];
  dig_H4 = (int16_t)((e4 << 4) | (e5 & 0x0F));
  dig_H5 = (int16_t)((e6 << 4) | ((e5 >> 4) & 0x0F));
  dig_H6 = (int8_t)buf2[6];

  return true;
}

void SoftI2C_BME280::writeDefaultConfig() {
  // humidity oversampling x1
  writeRegister(BME280_REG_CTRL_HUM, 0x05);
  // config: standby 1000 ms, filter off (0xA0) was default config, switched to 0x00 (no standby needed for forced mode)
  writeRegister(BME280_REG_CONFIG, 0x00);
  // ctrl_meas: osrs_t=1, osrs_p=1, mode=normal (0x27) was default config, switched to 0x25 (temp and pressure oversampling, forced mode) 
  writeRegister(BME280_REG_CTRL_MEAS, 0xB5);
  while (readRegister(BME280_REG_STATUS) & 0x08) delay(1);
}

// --- raw reads ---
uint32_t SoftI2C_BME280::readRawTemperature() {
  uint8_t b[3];
  readRegisters(BME280_REG_TEMP_MSB, b, 3);
  return ((uint32_t)b[0] << 12) | ((uint32_t)b[1] << 4) | ((uint32_t)b[2] >> 4);
}

uint32_t SoftI2C_BME280::readRawPressure() {
  uint8_t b[3];
  readRegisters(BME280_REG_PRESS_MSB, b, 3);
  return ((uint32_t)b[0] << 12) | ((uint32_t)b[1] << 4) | ((uint32_t)b[2] >> 4);
}

uint16_t SoftI2C_BME280::readRawHumidity() {
  uint8_t b[2];
  readRegisters(BME280_REG_HUM_MSB, b, 2);
  return (uint16_t)b[0] << 8 | b[1];
}

// --- compensation formulas ---
// Temperature in °C
float SoftI2C_BME280::readTemperature() {
  if (!_initialized) return NAN;
  int32_t adc_T = (int32_t)readRawTemperature();

  int32_t var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
  int32_t var2 = (((((adc_T >> 4) - (int32_t)dig_T1) * ((adc_T >> 4) - (int32_t)dig_T1)) >> 12) *
                  ((int32_t)dig_T3)) >> 14;
  t_fine = var1 + var2;
  float T = (t_fine * 5 + 128) >> 8;
  return T / 100.0f;
}

// Pressure in hPa
float SoftI2C_BME280::readPressure() {
  if (!_initialized) return NAN;
  int32_t adc_P = (int32_t)readRawPressure();

  int64_t var1, var2, p;
  var1 = ((int64_t)t_fine) - 128000;
  var2 = var1 * var1 * (int64_t)dig_P6;
  var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
  var2 = var2 + (((int64_t)dig_P4) << 35);
  var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
  var1 = (((((int64_t)1) << 47) + var1) * (int64_t)dig_P1) >> 33;

  if (var1 == 0) return NAN; // avoid div by zero

  p = 1048576 - adc_P;
  p = (((p << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)dig_P8) * p) >> 19;
  p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);

  // p is pressure * 256 (Pa). Convert to hPa:
  float pressure_hPa = (float)p / 256.0f / 100.0f;
  return pressure_hPa;
}

// Humidity in %
float SoftI2C_BME280::readHumidity() {
  if (!_initialized) return NAN;
  int32_t adc_H = (int32_t)readRawHumidity();

  int32_t v_x1 = t_fine - 76800;
  int32_t v_x2 = (((((adc_H << 14) - ((int32_t)dig_H4 << 20) - ((int32_t)dig_H5 * v_x1)) + 16384) >> 15) *
                 (((((((v_x1 * (int32_t)dig_H6) >> 10) * (((v_x1 * (int32_t)dig_H3) >> 11) + 32768)) >> 10) + 2097152) *
                   (int32_t)dig_H2 + 8192) >> 14));
  v_x1 = v_x2 - (((((v_x2 >> 15) * (v_x2 >> 15)) >> 7) * (int32_t)dig_H1) >> 4);
  v_x1 = max(v_x1, 0);
  v_x1 = min(v_x1, 419430400);
  float h = (v_x1 >> 12);
  h = h / 1024.0f;
  return h;
}

bool SoftI2C_BME280::readAll(float &T, float &P, float &H) {
  writeRegister(BME280_REG_CTRL_MEAS, 0xB5);
  while (readRegister(BME280_REG_STATUS) & 0x08) delay(1);
  T = readTemperature();
  // readTemperature updates t_fine
  P = readPressure();
  H = readHumidity();
  return !(isnan(T) || isnan(P) || isnan(H));
}

#endif // SOFTI2C_BME280_H