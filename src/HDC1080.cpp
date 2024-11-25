#include "HDC1080.hpp"
#include <Wire.h>
#include <Arduino.h>

/* ------------------------------------------------------------ */
/*				Procedure Definitions							*/
/* ------------------------------------------------------------ */

/* ------------------------------------------------------------ */
/*        HYGROI2C::writeRegI2C
**
**        Synopsis:
**				writeRegI2C(bConfig);
**
**        Parameters:
**				uint8_t bReg - the register address to be written to
**				uint16_t bVal - the bytes to be written
**
**        Return Values:
**                void 
**
**        Errors:
**			none
**
**        Description:
**			This function writes to a register over I2C. 
**
*/
void HYGROI2C::writeRegI2C(uint8_t bReg, uint16_t bVal)
{
    Wire.beginTransmission(HDC1080_I2C_ADDR); // start transmission to device
    Wire.write(bReg);                         // send register address
    Wire.write((bVal >> 8) & 0xff);           // send upper byte
    Wire.write((bVal)&0xff);                  // send lower byte
    Wire.endTransmission();                   // end transmission
}

/* ------------------------------------------------------------ */
/*        HYGROI2C::readRegI2C
**
**        Synopsis:
**				readRegI2C(bReg, rVal, delay_ms);
**
**        Parameters:
**				uint8_t bReg - the register address to be written to
**				uint16_t* rVal - the return location for the read bytes
**				unsigned int delay_ms - the number of milliseconds required for the HYGRO to convert the desired data
**
**        Return Values:
**                bool success - whether valid data has been successfully captured
**
**        Errors:
**			failure on bad rVal pointer
**
**        Description:
**			This function reads a register over I2C. 
**
*/
bool HYGROI2C::readRegI2C(uint8_t bReg, uint16_t &rVal, unsigned long delay_ms)
{
    Wire.beginTransmission(HDC1080_I2C_ADDR);
    Wire.write(bReg); // send register address
    Wire.endTransmission();
    delay(delay_ms); // wait for conversion to complete
    Wire.requestFrom(HDC1080_I2C_ADDR, 2);

    byte msb = Wire.read();
  byte lsb = Wire.read();

    rVal = (msb << 8) | lsb;
    return true;
}

/* ------------------------------------------------------------ */
/*        HYGROI2C::begin
**
**        Synopsis:
**				myHYGROI2C.begin();
**
**        Parameters:
**
**        Return Values:
**                void 
**
**        Errors:
**
**        Description:
**				This function initializes the I2C interface #1 that is used to communicate with PmodAD2.
**
*/
void HYGROI2C::begin()
{
    Wire.beginTransmission(HDC1080_I2C_ADDR);
    // Send config register address then
    // Send setup bytes, software reset bit ON, all others are default
    Wire.write(HDC1080_CONFIG_REG);
    Wire.write(0x00);
    Wire.write(0x00);
    Wire.endTransmission();
}

/* ------------------------------------------------------------ */
/*        HYGROI2C::getTemperature
**
**        Synopsis:
**				myHYGROI2C.getTemperature();
**
**        Parameters:
**
**        Return Values:
**                float deg_c - the temperature reading in degrees celsius
**
**        Errors: - modify to manage read failures
**
**        Description:
**				This function captures a temperature reading from the Pmod HYGRO.
**
*/
float HYGROI2C::getTemperature()
{
    uint16_t raw_t;
    float deg_c;
    if (readRegI2C(HDC1080_TMP_REG, raw_t, 9)) {
        deg_c = (float)raw_t / 0x10000;
        deg_c *= 165.0;
        deg_c -= 40.0; // conversion provided in reference manual
        return deg_c;
    } // conversion time for temperature at 14 bit resolution is 6.5 ms
    Serial.println("Failed to get temperature");
    return 117.0;
}

/* ------------------------------------------------------------ */
/*        HYGROI2C::getHumidity
**
**        Synopsis:
**				HYGROI2C.getHumidity();
**
**        Parameters:
**
**        Return Values:
**                float per_rh - the humidity reading in percent relative humidity.
**
**        Errors: - modify to manage read failures
**
**        Description:
**				This function captures a humidity reading from the Pmod HYGRO.
**
*/
float HYGROI2C::getHumidity()
{
    uint16_t raw_h;
    float per_rh;
    readRegI2C(HDC1080_HUM_REG, raw_h, 7); // conversion time for humidity at 14 bit resolution is 6.35 ms
    per_rh = (float)raw_h / 0x10000;
    per_rh *= 100.0; // conversion provided in reference manual
    return per_rh;
}

void HYGROI2C::selfTest(bool enabled)
{
    uint16_t config_reg;
    readRegI2C(HDC1080_CONFIG_REG, config_reg, 1);
    if (enabled) {
        config_reg |= HEATER_EN_BIT;
    }
    else{
        config_reg &= ~(HEATER_EN_BIT);

    }
    writeRegI2C(HDC1080_CONFIG_REG, config_reg);
}

/* ------------------------------------------------------------ */
/*        HYGROI2C::tempF2C
**
**        Synopsis:
**				HYGROI2C.tempF2C(deg_f);
**
**        Parameters:
**				float deg_f - the temperature in degrees fahrenheit
**        Return Values:
**              float deg_c - the temperature in degrees celsius
**
**        Errors:
**
**        Description:
**				This function converts a fahrenheit temperature to celsius
**
*/
float tempF2C(float deg_f)
{
    return (deg_f - 32) / 1.8;
}

/* ------------------------------------------------------------ */
/*        HYGROI2C::tempC2F
**
**        Synopsis:
**				HYGROI2C.tempC2F(deg_c);
**
**        Parameters:
**              float deg_c - the temperature in degrees celsius
**        Return Values:
**				float deg_f - the temperature in degrees fahrenheit
**
**        Errors:
**
**        Description:
**				This function converts a celsius temperature to fahrenheit
**
*/
float tempC2F(float deg_c)
{
    return deg_c * 1.8 + 32;
}
