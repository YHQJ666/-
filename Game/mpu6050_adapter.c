#include "snake.h"
#include "mpu6050.h"
#include <stdlib.h>

// MPU6050适配器 - 用于测试和兼容现有驱动

/**
 * @brief 简化的MPU6050读取函数
 * 这个函数需要根据你的实际MPU6050驱动进行调整
 */
int mpu6050_read_accel_simple(uint8_t device_addr, int16_t* accel_x, int16_t* accel_y, int16_t* accel_z) {
    // 这里应该调用你的实际MPU6050驱动函数
    // 目前先用模拟数据进行测试
    
    static int16_t test_counter = 0;
    test_counter++;
    
    // 模拟倾斜数据用于测试
    if(device_addr == 0x68) { // 玩家1
        if((test_counter / 100) % 4 == 0) {
            *accel_x = 5000;  // 向右倾斜
            *accel_y = 0;
        } else if((test_counter / 100) % 4 == 1) {
            *accel_x = 0;
            *accel_y = 5000; // 向下倾斜
        } else if((test_counter / 100) % 4 == 2) {
            *accel_x = -5000; // 向左倾斜
            *accel_y = 0;
        } else {
            *accel_x = 0;
            *accel_y = -5000; // 向上倾斜
        }
    } else { // 玩家2
        if((test_counter / 80) % 4 == 0) {
            *accel_x = -5000; // 向左倾斜
            *accel_y = 0;
        } else if((test_counter / 80) % 4 == 1) {
            *accel_x = 0;
            *accel_y = -5000; // 向上倾斜
        } else if((test_counter / 80) % 4 == 2) {
            *accel_x = 5000;  // 向右倾斜
            *accel_y = 0;
        } else {
            *accel_x = 0;
            *accel_y = 5000; // 向下倾斜
        }
    }
    
    *accel_z = 16384; // 重力加速度
    
    return 0; // 成功
}

/**
 * @brief 获取随机测试输入（用于调试）
 */
Direction_e get_test_direction(uint8_t player_id) {
    static uint32_t counter = 0;
    counter++;
    
    // 每隔一段时间改变方向
    if(counter % 200 == 0) {
        return (Direction_e)(rand() % 4);
    }
    
    return DIR_INVALID;
}
