#include "image.h"
#include <stdint.h>


#define TANK2_HEIGHT 16
#define TANK2_WIDTH 20

uint16_t tank11[576];
uint16_t tank22[320];
uint16_t tank33[320];

#include <math.h>
#define M_PI 3.1415926f

//typedef struct {
//    int width;
//    int height;
//} ImageSize;

// 颜色插值辅助函数
uint16_t interpolateColor(uint16_t c1, uint16_t c2, float weight) {
    uint8_t r1 = (c1 >> 11) & 0x1F;
    uint8_t g1 = (c1 >> 5) & 0x3F;
    uint8_t b1 = c1 & 0x1F;
    
    uint8_t r2 = (c2 >> 11) & 0x1F;
    uint8_t g2 = (c2 >> 5) & 0x3F;
    uint8_t b2 = c2 & 0x1F;
    
    uint8_t r = (uint8_t)(r1 * (1-weight) + r2 * weight);
    uint8_t g = (uint8_t)(g1 * (1-weight) + g2 * weight);
    uint8_t b = (uint8_t)(b1 * (1-weight) + b2 * weight);
    
    return (r << 11) | (g << 5) | b;
}
ImageSize calculateRotatedSize(int originalWidth, int originalHeight, float angle) {
    float rad = angle * M_PI / 180.0f;
    float cosA = fabs(cos(rad));
    float sinA = fabs(sin(rad));
    
    ImageSize size;
    size.width = (int)(originalWidth * cosA + originalHeight * sinA);
    size.height = (int)(originalWidth * sinA + originalHeight * cosA);
    
    // 确保尺寸为偶数，便于中心点计算
    if(size.width % 2 != 0) size.width++;
    if(size.height % 2 != 0) size.height++;
    
    return size;
}
void rotateArbitrary(uint16_t *dest, const uint16_t *src, 
                    int originalWidth, int originalHeight,
                    float angle, uint16_t bgColor) {
    float rad = angle * M_PI / 180.0f;
    float cosA = cos(rad);
    float sinA = sin(rad);
    
    // 计算旋转后图像大小
    ImageSize rotatedSize = calculateRotatedSize(originalWidth, originalHeight, angle);
    int newWidth = rotatedSize.width;
    int newHeight = rotatedSize.height;
    
    // 原始图像中心
    float origCenterX = originalWidth / 2.0f;
    float origCenterY = originalHeight / 2.0f;
    
    // 新图像中心
    float newCenterX = newWidth / 2.0f;
    float newCenterY = newHeight / 2.0f;
    
    for (int y = 0; y < newHeight; y++) {
        for (int x = 0; x < newWidth; x++) {
            // 计算在原始图像中的对应位置（反向旋转）
            float srcX = cosA * (x - newCenterX) + sinA * (y - newCenterY) + origCenterX;
            float srcY = -sinA * (x - newCenterX) + cosA * (y - newCenterY) + origCenterY;
            
            // 双线性插值
            int x1 = (int)srcX;
            int y1 = (int)srcY;
            int x2 = x1 + 1;
            int y2 = y1 + 1;
            
            float xWeight = srcX - x1;
            float yWeight = srcY - y1;
            
            // 边界检查
            if(x1 >= 0 && x2 < originalWidth && y1 >= 0 && y2 < originalHeight) {
                // 获取四个邻近像素
                uint16_t c11 = src[y1 * originalWidth + x1];
                uint16_t c21 = src[y1 * originalWidth + x2];
                uint16_t c12 = src[y2 * originalWidth + x1];
                uint16_t c22 = src[y2 * originalWidth + x2];
                
                // 双线性插值
                uint16_t r1 = interpolateColor(c11, c21, xWeight);
                uint16_t r2 = interpolateColor(c12, c22, xWeight);
                dest[y * newWidth + x] = interpolateColor(r1, r2, yWeight);
            } else {
                // 超出原始图像范围，使用背景色
                dest[y * newWidth + x] = bgColor;
            }
        }
    }
}




const uint32_t tank2[320] = {
0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xFF808080, 0xFF808080, 0xFF808080, 0xFF808080, 
0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xFF808080, 0xFF808080, 0xFF808080, 0xFF808080, 
0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xFF808080, 0xFF808080, 0xFF808080, 0xFF808080, 
0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xFF808080, 0xFF808080, 0xFF808080, 0xFF808080, 
0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xFF808080, 0xFF808080, 0xFF808080, 0xFF808080, 
0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff000000, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff000000, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xFF808080, 0xFF808080, 0xFF808080, 0xFF808080, 
0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff000000, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 
0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff000000, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xFF808080, 0xFF808080, 0xFF808080, 0xff000000, 
0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff000000, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xFF808080, 0xFF808080, 0xFF808080, 0xff000000, 
0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff000000, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 
0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff000000, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff000000, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xFF808080, 0xFF808080, 0xFF808080, 0xFF808080, 
0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xFF808080, 0xFF808080, 0xFF808080, 0xFF808080, 
0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xFF808080, 0xFF808080, 0xFF808080, 0xFF808080, 
0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xFF808080, 0xFF808080, 0xFF808080, 0xFF808080, 
0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xFF808080, 0xFF808080, 0xFF808080, 0xFF808080, 
0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xff0000ff, 0xFF808080, 0xFF808080, 0xFF808080, 0xFF808080

	
};

uint16_t abgr8888_to_rgb565(uint32_t color) {
//    uint8_t a = (color >> 24) & 0xFF;
//    uint8_t b = (color >> 16) & 0xFF;
//    uint8_t g = (color >> 8)  & 0xFF;
//    uint8_t r = color & 0xFF;

//    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
	uint8_t r =  color & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = (color >> 16) & 0xFF;

    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
};


void convert_tank_image_rgb565(uint16_t *dst, const uint32_t *src, uint16_t count) {
    for (int i = 0; i < count; i++) {
        dst[i] = abgr8888_to_rgb565(src[i]);
    }
};




