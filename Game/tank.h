#ifndef __TANK_H
#define __TANK_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "image.h"

#define MAX_PLAYERS     2
#define TANK_WIDTH      20              // 坦克图像宽度
#define TANK_HEIGHT     16              // 坦克图像高度
#define TANK_SIZE       24              // 显示时的坦克尺寸
#define TANK_PIXELS     (TANK_WIDTH * TANK_HEIGHT)  // 320像素
#define MUZZLE_OFFSET   12              // 炮口距离坦克中心的偏移

// 陀螺仪滤波参数
#define GYRO_FILTER_ALPHA   0.1f        // 低通滤波系数
#define ROTATION_SENSITIVITY 1.0f       // 旋转灵敏度
#define ANGLE_THRESHOLD     0.5f        // 角度变化阈值

typedef struct {
    int x;              // 坐标位置
    int y;
    float angle;        // 当前朝向角度（0~360）
    uint8_t player_id;  // 玩家ID（0或1）
    uint8_t hp;         // 生命值
    uint8_t alive;      // 是否存活
    
    // 陀螺仪相关
    float filtered_gyro_z;  // 滤波后的Z轴角速度
    uint32_t last_update;   // 上次更新时间戳
    
	uint32_t color;
	
    // 旋转图像缓存
    uint16_t *rotated_image;    // 旋转后的RGB565图像缓存
    float cached_angle;         // 缓存的角度
    uint8_t cache_valid;        // 缓存是否有效
} tank_t;

// 炮口位置结构
typedef struct {
    int x;
    int y;
} muzzle_pos_t;

// 基础函数
void tank_init(void);
void tank_set_angle(uint8_t id, float angle);
float tank_get_angle(uint8_t id);
void tank_draw(uint8_t id);
void tank_set_position(uint8_t id, int x, int y);
void tank_get_position(uint8_t id, int *x, int *y);

// 陀螺仪控制函数
void tank_update_from_gyro(uint8_t id, float gyro_z_angle);
void tank_update_rotation(uint8_t id, float delta_time);

// 炮口位置计算
muzzle_pos_t tank_get_muzzle_position(uint8_t id);

#endif


