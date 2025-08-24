#ifndef __IMAGE_H__
#define __IMAGE_H__
#include <stdint.h>

//extern const uint32_t tank1[576];
//extern  uint16_t tank11[576];

extern const uint32_t tank2[320];
extern  uint16_t tank22[320];

//extern const uint16_t tank3[320];
//extern  uint16_t tank33[320];


typedef struct {
    int width;
    int height;
} ImageSize;

void convert_tank_image_rgb565(uint16_t *dst, const uint32_t *src, uint16_t count);
uint16_t abgr8888_to_rgb565(uint32_t color);

uint16_t interpolateColor(uint16_t c1, uint16_t c2, float weight);
ImageSize calculateRotatedSize(int originalWidth, int originalHeight, float angle);
void rotateArbitrary(uint16_t *dest, const uint16_t *src, 
                    int originalWidth, int originalHeight,
                    float angle, uint16_t bgColor);
//extern const uint16_t tank3[2000];
//extern const uint8_t tank4[2000];
//extern const uint32_t tank5[2000];

#endif
