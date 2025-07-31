#ifndef __ST_IMAGE_H__
#define __ST_IMAGE_H__


#include <stdint.h>


typedef struct
{
    const uint16_t width;
    const uint16_t height;
    const uint8_t *data;
} st_image_t;


extern st_image_t image_tv_128x72;


#endif /* __ST_IMAGE_H__ */
