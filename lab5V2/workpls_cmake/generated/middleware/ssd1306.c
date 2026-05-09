/**
  ******************************************************************************
  * @file           : ssd1306.c
  * @brief          : SSD1306 OLED Display Driver Implementation
  ******************************************************************************
  */

#include "ssd1306.h"
#include <string.h>
#include <math.h>

/* 5x8 Font data (ASCII 32-126) */
static const uint8_t font_5x8[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, // Space
    0x00, 0x00, 0x2F, 0x00, 0x00, // !
    0x00, 0x07, 0x00, 0x07, 0x00, // "
    0x14, 0x7F, 0x14, 0x7F, 0x14, // #
    0x24, 0x2A, 0x7F, 0x2A, 0x12, // $
    0x62, 0x64, 0x08, 0x13, 0x23, // %
    0x36, 0x49, 0x55, 0x22, 0x50, // &
    0x00, 0x05, 0x03, 0x00, 0x00, // '
    0x00, 0x1C, 0x22, 0x41, 0x00, // (
    0x00, 0x41, 0x22, 0x1C, 0x00, // )
    0x14, 0x08, 0x3E, 0x08, 0x14, // *
    0x08, 0x08, 0x3E, 0x08, 0x08, // +
    0x00, 0x00, 0xA0, 0x60, 0x00, // ,
    0x08, 0x08, 0x08, 0x08, 0x08, // -
    0x00, 0x60, 0x60, 0x00, 0x00, // .
    0x20, 0x10, 0x08, 0x04, 0x02, // /
    0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
    0x00, 0x42, 0x7F, 0x40, 0x00, // 1
    0x42, 0x61, 0x51, 0x49, 0x46, // 2
    0x21, 0x41, 0x45, 0x4B, 0x31, // 3
    0x18, 0x14, 0x12, 0x7F, 0x10, // 4
    0x27, 0x45, 0x45, 0x45, 0x39, // 5
    0x3C, 0x4A, 0x49, 0x49, 0x30, // 6
    0x01, 0x71, 0x09, 0x05, 0x03, // 7
    0x36, 0x49, 0x49, 0x49, 0x36, // 8
    0x06, 0x49, 0x49, 0x29, 0x1E, // 9
    0x00, 0x36, 0x36, 0x00, 0x00, // :
    0x00, 0x56, 0x36, 0x00, 0x00, // ;
    0x08, 0x14, 0x22, 0x41, 0x00, // <
    0x14, 0x14, 0x14, 0x14, 0x14, // =
    0x00, 0x41, 0x22, 0x14, 0x08, // >
    0x02, 0x01, 0x51, 0x09, 0x06, // ?
    0x32, 0x49, 0x59, 0x51, 0x3E, // @
    0x7C, 0x12, 0x11, 0x12, 0x7C, // A
    0x7F, 0x49, 0x49, 0x49, 0x36, // B
    0x3E, 0x41, 0x41, 0x41, 0x22, // C
    0x7F, 0x41, 0x41, 0x22, 0x1C, // D
    0x7F, 0x49, 0x49, 0x49, 0x41, // E
    0x7F, 0x09, 0x09, 0x09, 0x01, // F
    0x3E, 0x41, 0x49, 0x49, 0x7A, // G
    0x7F, 0x08, 0x08, 0x08, 0x7F, // H
    0x00, 0x41, 0x7F, 0x41, 0x00, // I
    0x20, 0x40, 0x41, 0x3F, 0x01, // J
    0x7F, 0x08, 0x14, 0x22, 0x41, // K
    0x7F, 0x40, 0x40, 0x40, 0x40, // L
    0x7F, 0x02, 0x0C, 0x02, 0x7F, // M
    0x7F, 0x04, 0x08, 0x10, 0x7F, // N
    0x3E, 0x41, 0x41, 0x41, 0x3E, // O
    0x7F, 0x09, 0x09, 0x09, 0x06, // P
    0x3E, 0x41, 0x51, 0x21, 0x5E, // Q
    0x7F, 0x09, 0x19, 0x29, 0x46, // R
    0x46, 0x49, 0x49, 0x49, 0x31, // S
    0x01, 0x01, 0x7F, 0x01, 0x01, // T
    0x3F, 0x40, 0x40, 0x40, 0x3F, // U
    0x1F, 0x20, 0x40, 0x20, 0x1F, // V
    0x3F, 0x40, 0x38, 0x40, 0x3F, // W
    0x63, 0x14, 0x08, 0x14, 0x63, // X
    0x07, 0x08, 0x70, 0x08, 0x07, // Y
    0x61, 0x51, 0x49, 0x45, 0x43, // Z
};

/* Helper: Send multiple commands */
static hal_status_t ssd1306_writeCmdSeq(ssd1306_t *oled, const uint8_t *cmds, uint8_t length)
{
    uint8_t buffer[length + 1];
    buffer[0] = SSD1306_CMD_CONTROL_BYTE;
    memcpy(&buffer[1], cmds, length);
    return HAL_I2C_MASTER_Transmit(oled->hi2c, SSD1306_I2C_ADDR, buffer, length + 1, SSD1306_I2C_TIMEOUT);
}

/* Helper: Send data to SSD1306 */
static hal_status_t ssd1306_writeData(ssd1306_t *oled, const uint8_t *data, uint16_t length)
{
    uint8_t buffer[length + 1];
    buffer[0] = SSD1306_DATA_CONTROL_BYTE;
    memcpy(&buffer[1], data, length);
    return HAL_I2C_MASTER_Transmit(oled->hi2c, SSD1306_I2C_ADDR, buffer, length + 1, SSD1306_I2C_TIMEOUT);
}

/**
  * @brief Initialize SSD1306 display
  */
uint8_t ssd1306_init(ssd1306_t *oled, hal_i2c_handle_t *hi2c)
{
    if (oled == NULL || hi2c == NULL)
        return 0;

    oled->hi2c = hi2c;
    memset(oled->buffer, 0, SSD1306_BUFFER_SIZE);

    /* Initialization sequence */
    uint8_t init_seq[] = {
        SSD1306_DISPLAYOFF,
        SSD1306_SETDISPLAYCLOCKDIV, 0x80,
        SSD1306_SETMULTIPLEXRATIO, 0x3F,
        SSD1306_SETDISPLAYOFFSET, 0x00,
        SSD1306_SETSTARTLINE,
        SSD1306_CHARGEPUMP, 0x14,
        SSD1306_MEMORYMODE, 0x02,
        SSD1306_SEGREMAP,
        SSD1306_COMSCANDEC,
        SSD1306_SETCOMPINS, 0x12,
        SSD1306_SETCONTRAST, 0xCF,
        SSD1306_SETPRECHARGE, 0xF1,
        SSD1306_SETVCOMDESELECT, 0x40,
        SSD1306_DISPLAYALLON_RESUME,
        SSD1306_NORMALDISPLAY,
        SSD1306_DISPLAYON
    };

    if (ssd1306_writeCmdSeq(oled, init_seq, sizeof(init_seq)) != HAL_OK)
        return 0;

    ssd1306_clear(oled);
    return 1;
}

/**
  * @brief Clear display buffer
  */
void ssd1306_clear(ssd1306_t *oled)
{
    memset(oled->buffer, 0x00, SSD1306_BUFFER_SIZE);
    ssd1306_update(oled);
}

/**
  * @brief Update display from buffer
  */
hal_status_t ssd1306_update(ssd1306_t *oled)
{
    uint8_t page;
    hal_status_t status = HAL_OK;

    for (page = 0; page < SSD1306_PAGES; page++)
    {
        uint8_t cmd_seq[] = {
            SSD1306_PAGEADDR, page, page,
            SSD1306_COLUMNADDR, 0x00, (SSD1306_WIDTH - 1)
        };

        status = ssd1306_writeCmdSeq(oled, cmd_seq, sizeof(cmd_seq));
        if (status != HAL_OK)
            return status;

        status = ssd1306_writeData(oled, &oled->buffer[page * SSD1306_WIDTH], SSD1306_WIDTH);
        if (status != HAL_OK)
            return status;
    }

    return status;
}

/**
  * @brief Draw a pixel
  */
void ssd1306_drawPixel(ssd1306_t *oled, uint16_t x, uint16_t y, uint8_t color)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT)
        return;

    uint16_t byte_index = (y / 8) * SSD1306_WIDTH + x;
    uint8_t bit = y % 8;

    if (color)
        oled->buffer[byte_index] |= (1 << bit);
    else
        oled->buffer[byte_index] &= ~(1 << bit);
}

/**
  * @brief Draw a line (Bresenham algorithm)
  */
void ssd1306_drawLine(ssd1306_t *oled, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t color)
{
    int16_t dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int16_t dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = (dx > dy) ? (dx / 2) : -(dy / 2);
    int16_t x = x0, y = y0;

    while (1)
    {
        ssd1306_drawPixel(oled, x, y, color);
        if (x == x1 && y == y1)
            break;
        int16_t e2 = err;
        if (e2 > -dx)
        {
            err -= dy;
            x += sx;
        }
        if (e2 < dy)
        {
            err += dx;
            y += sy;
        }
    }
}

/**
  * @brief Draw a rectangle
  */
void ssd1306_drawRect(ssd1306_t *oled, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color)
{
    ssd1306_drawLine(oled, x, y, x + w, y, color);
    ssd1306_drawLine(oled, x + w, y, x + w, y + h, color);
    ssd1306_drawLine(oled, x + w, y + h, x, y + h, color);
    ssd1306_drawLine(oled, x, y + h, x, y, color);
}

/**
  * @brief Fill a rectangle
  */
void ssd1306_fillRect(ssd1306_t *oled, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t color)
{
    for (uint16_t i = y; i < y + h; i++)
        ssd1306_drawLine(oled, x, i, x + w, i, color);
}

/**
  * @brief Draw a circle (Midpoint algorithm)
  */
void ssd1306_drawCircle(ssd1306_t *oled, uint16_t x0, uint16_t y0, uint16_t r, uint8_t color)
{
    int16_t x = r;
    int16_t y = 0;
    int16_t err = 0;

    while (x >= y)
    {
        ssd1306_drawPixel(oled, x0 + x, y0 + y, color);
        ssd1306_drawPixel(oled, x0 + y, y0 + x, color);
        ssd1306_drawPixel(oled, x0 - y, y0 + x, color);
        ssd1306_drawPixel(oled, x0 - x, y0 + y, color);
        ssd1306_drawPixel(oled, x0 - x, y0 - y, color);
        ssd1306_drawPixel(oled, x0 - y, y0 - x, color);
        ssd1306_drawPixel(oled, x0 + y, y0 - x, color);
        ssd1306_drawPixel(oled, x0 + x, y0 - y, color);

        err += 1 + 2 * y;
        y += 1;
        if (2 * (err - x) + 1 > 0)
        {
            x -= 1;
            err += 1 - 2 * x;
        }
    }
}

/**
  * @brief Fill a circle
  */
void ssd1306_fillCircle(ssd1306_t *oled, uint16_t x0, uint16_t y0, uint16_t r, uint8_t color)
{
    for (int16_t y = -r; y <= r; y++)
    {
        for (int16_t x = -r; x <= r; x++)
        {
            if (x * x + y * y <= r * r)
                ssd1306_drawPixel(oled, x0 + x, y0 + y, color);
        }
    }
}

/**
  * @brief Draw a character
  */
void ssd1306_drawChar(ssd1306_t *oled, uint16_t x, uint16_t y, char c, uint8_t color)
{
    if (c < 32 || c > 126)
        return;

    uint8_t char_index = c - 32;
    const uint8_t *char_data = &font_5x8[char_index * 5];

    for (uint8_t i = 0; i < 5; i++)
    {
        uint8_t byte = char_data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (byte & (1 << j))
                ssd1306_drawPixel(oled, x + i, y + j, color);
        }
    }
}

/**
  * @brief Draw a string
  */
void ssd1306_drawString(ssd1306_t *oled, uint16_t x, uint16_t y, const char *str, uint8_t color)
{
    while (*str)
    {
        ssd1306_drawChar(oled, x, y, *str, color);
        x += 6;
        if (x + 6 > SSD1306_WIDTH)
        {
            x = 0;
            y += 8;
        }
        str++;
    }
}
