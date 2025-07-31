#ifndef __ST7735_H__
#define __ST7735_H__


#include <stdbool.h>
#include "stfonts.h"


#define ST7735_MADCTL_MY  0x80
#define ST7735_MADCTL_MX  0x40
#define ST7735_MADCTL_MV  0x20
#define ST7735_MADCTL_ML  0x10
#define ST7735_MADCTL_RGB 0x00
#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH  0x04

// 1.44" display, default orientation
#define ST7735_IS_128X128 1
#define ST7735_WIDTH  128
#define ST7735_HEIGHT 160
#define ST7735_XSTART 0
#define ST7735_YSTART 0
#define ST7735_ROTATION (ST7735_MADCTL_MX | ST7735_MADCTL_MY | ST7735_MADCTL_RGB)

// 1.44" display, rotate right
/*
#define ST7735_IS_128X128 1
#define ST7735_WIDTH  128
#define ST7735_HEIGHT 128
#define ST7735_XSTART 3
#define ST7735_YSTART 2
#define ST7735_ROTATION (ST7735_MADCTL_MY | ST7735_MADCTL_MV | ST7735_MADCTL_BGR)
*/

// 1.44" display, rotate left
/*
#define ST7735_IS_128X128 1
#define ST7735_WIDTH  128
#define ST7735_HEIGHT 128
#define ST7735_XSTART 1
#define ST7735_YSTART 2
#define ST7735_ROTATION (ST7735_MADCTL_MX | ST7735_MADCTL_MV | ST7735_MADCTL_BGR)
*/

// 1.44" display, upside down
/*
#define ST7735_IS_128X128 1
#define ST7735_WIDTH  128
#define ST7735_HEIGHT 128
#define ST7735_XSTART 2
#define ST7735_YSTART 1
#define ST7735_ROTATION (ST7735_MADCTL_BGR)
*/

// Color definitions
//#define BLACK     0x0000  // 黑色 (0, 0, 0)
//#define WHITE     0xFFFF  // 白色 (255, 255, 255)
//#define RED       0xF800  // 红色 (255, 0, 0)
//#define GREEN     0x07E0  // 绿色 (0, 255, 0)
//#define BLUE      0x001F  // 蓝色 (0, 0, 255)
//#define YELLOW    0xFFE0  // 黄色 (255, 255, 0)
//#define MAGENTA   0xF81F  // 品红 (255, 0, 255)
//#define CYAN      0x07FF  // 青色 (0, 255, 255)
//#define GRAY      0x8410  // 灰色 (128, 128, 128)
//#define BROWN     0xA145  // 棕色 (165,42,42)
//#define BRRED     0xFC06  // 亮红色 (255,51,51)
#define ST7735_COLOR565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))


void st7735_unselect(void);
void st7735_init(void);
void st7735_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void st7735_write_char(uint16_t x, uint16_t y, char ch, st_fonts_t *font, uint16_t color, uint16_t bgcolor);
void st7735_write_string(uint16_t x, uint16_t y, const char *str, st_fonts_t *font, uint16_t color, uint16_t bgcolor);
void st7735_write_font(uint16_t x, uint16_t y, st_fonts_t *font, uint32_t index, uint16_t color, uint16_t bgcolor);
void st7735_write_fonts(uint16_t x, uint16_t y, st_fonts_t *font, uint32_t index, uint32_t count, uint16_t color, uint16_t bgcolor);
void st7735_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void st7735_fill_screen(uint16_t color);
void st7735_draw_image(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t *data);


#endif // __ST7735_H__
