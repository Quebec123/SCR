/**
  ******************************************************************************
  * @file           : ssd1306.h
  * @brief          : SSD1306 OLED Display Driver Header
  ******************************************************************************
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */

#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include <stddef.h>
#include "mx_hal_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/* SSD1306 I2C Configuration */
#define SSD1306_I2C_ADDR        0x3C << 1
#define SSD1306_I2C_TIMEOUT     100U

/* Control Bytes */
#define SSD1306_CMD_CONTROL_BYTE    0x00
#define SSD1306_DATA_CONTROL_BYTE   0x40

/* Display Dimensions */
#define SSD1306_WIDTH           128
#define SSD1306_HEIGHT          64
#define SSD1306_PAGES           8
#define SSD1306_BUFFER_SIZE     (SSD1306_WIDTH * SSD1306_PAGES)

/* SSD1306 Commands */
#define SSD1306_SETCONTRAST             0x81
#define SSD1306_DISPLAYALLON_RESUME     0xA4
#define SSD1306_DISPLAYALLON            0xA5
#define SSD1306_NORMALDISPLAY           0xA6
#define SSD1306_INVERTDISPLAY           0xA7
#define SSD1306_DISPLAYOFF              0xAE
#define SSD1306_DISPLAYON               0xAF
#define SSD1306_SETDISPLAYOFFSET        0xD3
#define SSD1306_SETCOMPINS              0xDA
#define SSD1306_SETVCOMDESELECT         0xDB
#define SSD1306_SETDISPLAYCLOCKDIV      0xD5
#define SSD1306_SETPRECHARGE            0xD9
#define SSD1306_SETMULTIPLEXRATIO       0xA8
#define SSD1306_SETLOWCOLUMN            0x00
#define SSD1306_SETHIGHCOLUMN           0x10
#define SSD1306_SETSTARTLINE            0x40
#define SSD1306_MEMORYMODE              0x20
#define SSD1306_COLUMNADDR              0x21
#define SSD1306_PAGEADDR                0x22
#define SSD1306_COMSCANINC              0xC0
#define SSD1306_COMSCANDEC              0xC8
#define SSD1306_SEGREMAP                0xA1
#define SSD1306_CHARGEPUMP              0x8D

/* Type Definitions */
typedef struct {
    hal_i2c_handle_t *hi2c;
    uint8_t buffer[SSD1306_BUFFER_SIZE];
} ssd1306_t;

/* Function Prototypes */
uint8_t ssd1306_init(ssd1306_t *oled, hal_i2c_handle_t *hi2c);
void ssd1306_clear(ssd1306_t *oled);
hal_status_t ssd1306_update(ssd1306_t *oled);
void ssd1306_drawPixel(ssd1306_t *oled, uint16_t x, uint16_t y, uint8_t color);
void ssd1306_drawLine(ssd1306_t *oled, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t color);
void ssd1306_drawRect(ssd1306_t *oled, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color);
void ssd1306_fillRect(ssd1306_t *oled, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color);
void ssd1306_drawCircle(ssd1306_t *oled, uint16_t x0, uint16_t y0, uint16_t r, uint8_t color);
void ssd1306_fillCircle(ssd1306_t *oled, uint16_t x0, uint16_t y0, uint16_t r, uint8_t color);
void ssd1306_drawChar(ssd1306_t *oled, uint16_t x, uint16_t y, char c, uint8_t color);
void ssd1306_drawString(ssd1306_t *oled, uint16_t x, uint16_t y, const char *str, uint8_t color);

#ifdef __cplusplus
}
#endif

#endif /* SSD1306_H */
