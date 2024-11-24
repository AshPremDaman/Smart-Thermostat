
#if !defined(HDC1080_H)
#define HDC1080_H

/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */
#include <inttypes.h>

/* ------------------------------------------------------------ */
/*					Definitions									*/
/* ------------------------------------------------------------ */
#define HDC1080_I2C_ADDR 0x40
#define HDC1080_TMP_REG 0x00
#define HDC1080_HUM_REG 0x01
#define HDC1080_CONFIG_REG 0x02
#define HEATER_EN_BIT 0x2000

/* ------------------------------------------------------------ */
/*					Procedure Declarations						*/
/* ------------------------------------------------------------ */

class HYGROI2C
{
private:
    void writeRegI2C(uint8_t bReg, uint16_t bVal);
    bool readRegI2C(uint8_t bReg, uint16_t &rVal, unsigned long delay_ms);

public:
    float getTemperature();
    float getHumidity();
    void selfTest(bool enabled);
    void begin();
};
float tempC2F(float tempC);
float tempF2C(float tempF);

#endif