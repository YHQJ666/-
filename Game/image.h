#ifndef __IMAGE_H__
#define __IMAGE_H__
#include <stdint.h>



extern const uint32_t tank2[320];
extern  uint16_t tank22[320];

void convert_tank_image_rgb565(uint16_t *dst, const uint32_t *src, uint16_t count);
uint16_t abgr8888_to_rgb565(uint32_t color);
//extern const uint16_t tank3[2000];
//extern const uint8_t tank4[2000];
//extern const uint32_t tank5[2000];

#endif
