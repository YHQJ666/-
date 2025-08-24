#include "tank.h"
#include "st7735.h"
#include "image.h"
#include "config.h"
#include "FreeRTOS.h"
#include "task.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define M_PI 3.1415926f

static tank_t tanks[MAX_PLAYERS];

// 外部图像资源：ABGR8888 格式的坦克图像（20x16像素）
extern const uint32_t tank2[TANK_PIXELS]; // 320个uint32_t

// 坦克颜色定义（ABGR8888格式）
#define TANK1_COLOR_MASK 0xFF0000FF  // 红色坦克
#define TANK2_COLOR_MASK 0xFFFF0000  // 蓝色坦克

// FreeRTOS 时间获取函数
static inline uint32_t get_system_tick(void) {
    return xTaskGetTickCount();
}

void tank_init(void) {
    uint32_t current_time = get_system_tick();
    for (int i = 0; i < MAX_PLAYERS; i++) {
        tanks[i].x = 50 + i * 30;
        tanks[i].y = 80;
        tanks[i].angle = 0.0f;
        tanks[i].player_id = i;
        tanks[i].color = (i == 0) ? TANK1_COLOR_MASK : TANK2_COLOR_MASK;
        tanks[i].hp = 3;
        tanks[i].alive = 1;
        tanks[i].filtered_gyro_z = 0.0f;
        tanks[i].last_update = current_time;
        
        // 初始化旋转图像缓存
        tanks[i].rotated_image = pvPortMalloc(TANK_SIZE * TANK_SIZE * sizeof(uint16_t));
        tanks[i].cached_angle = -1.0f; // 无效角度
        tanks[i].cache_valid = 0;
        
        if (tanks[i].rotated_image == NULL) {
            // 内存分配失败处理
            tanks[i].alive = 0;
        }
    }
}

void tank_set_angle(uint8_t id, float angle) {
    if (id >= MAX_PLAYERS) return;
    // 角度归一化到 0~360
    while (angle < 0) angle += 360.0f;
    while (angle >= 360.0f) angle -= 360.0f;
    tanks[id].angle = angle;
}

float tank_get_angle(uint8_t id) {
    if (id >= MAX_PLAYERS) return 0;
    return tanks[id].angle;
}

void tank_set_position(uint8_t id, int x, int y) {
    if (id >= MAX_PLAYERS) return;
    tanks[id].x = x;
    tanks[id].y = y;
}

void tank_get_position(uint8_t id, int *x, int *y) {
    if (id >= MAX_PLAYERS || !x || !y) return;
    *x = tanks[id].x;
    *y = tanks[id].y;
}

/**
 * @brief 将ABGR8888颜色转换为RGB565并应用坦克颜色
 */
static uint16_t apply_tank_color_and_convert(uint32_t abgr_pixel, uint32_t color_mask) {
    // 提取ABGR8888分量
    uint8_t a = (abgr_pixel >> 24) & 0xFF;
    uint8_t b = (abgr_pixel >> 16) & 0xFF;
    uint8_t g = (abgr_pixel >> 8) & 0xFF;
    uint8_t r = abgr_pixel & 0xFF;
    
    // 如果是透明像素，返回透明色
    if (a < 128) {
        return 0x0000; // 黑色背景
    }
    
    // 应用坦克颜色遮罩
    uint8_t mask_r = color_mask & 0xFF;
    uint8_t mask_g = (color_mask >> 8) & 0xFF;
    uint8_t mask_b = (color_mask >> 16) & 0xFF;
    
    // 混合颜色
    r = (r * mask_r) >> 8;
    g = (g * mask_g) >> 8;
    b = (b * mask_b) >> 8;
    
    // 转换为RGB565
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
}

/**
 * @brief 双线性插值旋转算法
 */
static void rotate_tank_image_bilinear(uint16_t *dst, const uint32_t *src, 
                                      int src_width, int src_height,
                                      int dst_size, float angle, uint32_t color_mask) {
    float cos_a = cos(-angle * M_PI / 180.0f);  // 反向旋转
    float sin_a = sin(-angle * M_PI / 180.0f);
    
    float src_cx = src_width / 2.0f;
    float src_cy = src_height / 2.0f;
    float dst_cx = dst_size / 2.0f;
    float dst_cy = dst_size / 2.0f;
    
    // 清空目标图像
    memset(dst, 0, dst_size * dst_size * sizeof(uint16_t));
    
    for (int y = 0; y < dst_size; y++) {
        for (int x = 0; x < dst_size; x++) {
            // 转换到以中心为原点的坐标系
            float dx = x - dst_cx;
            float dy = y - dst_cy;
            
            // 反向映射到源图像
            float src_x = dx * cos_a - dy * sin_a + src_cx;
            float src_y = dx * sin_a + dy * cos_a + src_cy;
            
            // 检查边界
            if (src_x >= 0 && src_x < src_width-1 && src_y >= 0 && src_y < src_height-1) {
                // 双线性插值
                int x1 = (int)src_x;
                int y1 = (int)src_y;
                int x2 = x1 + 1;
                int y2 = y1 + 1;
                
                float fx = src_x - x1;
                float fy = src_y - y1;
                
                // 获取四个邻近像素
                uint32_t p11 = src[y1 * src_width + x1];
                uint32_t p12 = src[y1 * src_width + x2];
                uint32_t p21 = src[y2 * src_width + x1];
                uint32_t p22 = src[y2 * src_width + x2];
                
                // 转换为RGB565并应用颜色
                uint16_t c11 = apply_tank_color_and_convert(p11, color_mask);
                uint16_t c12 = apply_tank_color_and_convert(p12, color_mask);
                uint16_t c21 = apply_tank_color_and_convert(p21, color_mask);
                uint16_t c22 = apply_tank_color_and_convert(p22, color_mask);
                
                // RGB565插值
                uint16_t r1 = ((c11 >> 11) & 0x1F) * (1-fx) + ((c12 >> 11) & 0x1F) * fx;
                uint16_t g1 = ((c11 >> 5) & 0x3F) * (1-fx) + ((c12 >> 5) & 0x3F) * fx;
                uint16_t b1 = (c11 & 0x1F) * (1-fx) + (c12 & 0x1F) * fx;
                
                uint16_t r2 = ((c21 >> 11) & 0x1F) * (1-fx) + ((c22 >> 11) & 0x1F) * fx;
                uint16_t g2 = ((c21 >> 5) & 0x3F) * (1-fx) + ((c22 >> 5) & 0x3F) * fx;
                uint16_t b2 = (c21 & 0x1F) * (1-fx) + (c22 & 0x1F) * fx;
                
                uint16_t r = r1 * (1-fy) + r2 * fy;
                uint16_t g = g1 * (1-fy) + g2 * fy;
                uint16_t b = b1 * (1-fy) + b2 * fy;
                
                dst[y * dst_size + x] = (r << 11) | (g << 5) | b;
            }
        }
    }
}

void tank_draw(uint8_t id) {
    if (id >= MAX_PLAYERS || !tanks[id].alive || tanks[id].rotated_image == NULL) return;

    int x = tanks[id].x;
    int y = tanks[id].y;
    float angle = tanks[id].angle;
    
    // 检查缓存是否有效
    if (!tanks[id].cache_valid || fabs(tanks[id].cached_angle - angle) > 1.0f) {
        // 重新生成旋转图像
        rotate_tank_image_bilinear(tanks[id].rotated_image, tank2,
                                  TANK_WIDTH, TANK_HEIGHT, TANK_SIZE,
                                  angle, tanks[id].color);
        
        tanks[id].cached_angle = angle;
        tanks[id].cache_valid = 1;
    }
    
    // 绘制坦克图像
    st7735_draw_image(x - TANK_SIZE/2, y - TANK_SIZE/2, TANK_SIZE, TANK_SIZE, 
                     (uint8_t*)tanks[id].rotated_image);
}




/**
 * @brief 根据陀螺仪角度数据更新坦克旋转
 * @param id 坦克ID
 * @param gyro_z_angle 陀螺仪Z轴角度数据（已处理成角度）
 */
void tank_update_from_gyro(uint8_t id, float gyro_z_angle) {
    if (id >= MAX_PLAYERS || !tanks[id].alive) return;
    
    uint32_t current_time = get_system_tick();
    float delta_time = (current_time - tanks[id].last_update) / 1000.0f; // 转换为秒
    
    if (delta_time > 0.001f) { // 避免除零和过小的时间间隔
        // 低通滤波处理陀螺仪数据
        tanks[id].filtered_gyro_z = GYRO_FILTER_ALPHA * gyro_z_angle + 
                                   (1.0f - GYRO_FILTER_ALPHA) * tanks[id].filtered_gyro_z;
        
        // 只有当角度变化超过阈值时才更新
        if (fabs(tanks[id].filtered_gyro_z) > ANGLE_THRESHOLD) {
            // 计算角度变化
            float angle_delta = tanks[id].filtered_gyro_z * delta_time * ROTATION_SENSITIVITY;
            
            // 更新坦克角度
            tanks[id].angle += angle_delta;
            
            // 角度归一化到 0~360
            while (tanks[id].angle < 0) tanks[id].angle += 360.0f;
            while (tanks[id].angle >= 360.0f) tanks[id].angle -= 360.0f;
        }
        
        tanks[id].last_update = current_time;
    }
}

/**
 * @brief 计算坦克炮口位置
 * @param id 坦克ID
 * @return 炮口位置坐标
 */
muzzle_pos_t tank_get_muzzle_position(uint8_t id) {
    muzzle_pos_t muzzle = {0, 0};
    
    if (id >= MAX_PLAYERS || !tanks[id].alive) {
        return muzzle;
    }
    
    // 将角度转换为弧度
    float angle_rad = tanks[id].angle * M_PI / 180.0f;
    
    // 计算炮口相对于坦克中心的位置
    muzzle.x = tanks[id].x + (int)(MUZZLE_OFFSET * cos(angle_rad));
    muzzle.y = tanks[id].y + (int)(MUZZLE_OFFSET * sin(angle_rad));
    
    return muzzle;
}

/**
 * @brief 手动更新坦克旋转（用于测试或其他控制方式）
 * @param id 坦克ID
 * @param delta_time 时间间隔（秒）
 */
void tank_update_rotation(uint8_t id, float delta_time) {
    if (id >= MAX_PLAYERS || !tanks[id].alive) return;
    
    // 这个函数可以用于其他旋转控制方式
    // 例如键盘控制或其他传感器输入
    // 当前留空，主要使用 tank_update_from_gyro
}

