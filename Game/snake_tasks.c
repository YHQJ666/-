#include "snake.h"
#include "mpu6050.h"
#include <math.h>

// 外部函数声明
extern int mpu6050_read_accel_simple(uint8_t device_addr, int16_t* accel_x, int16_t* accel_y, int16_t* accel_z);
extern Direction_e get_test_direction(uint8_t player_id);

// MPU6050地址定义
#define MPU6050_ADDR1   0x68    // 第一个MPU6050地址
#define MPU6050_ADDR2   0x69    // 第二个MPU6050地址（AD0接VCC）

/**
 * @brief 从MPU6050数据判断方向
 */
Direction_e GetDirectionFromMPU(int16_t accel_x, int16_t accel_y) {
    // 基于加速度计判断倾斜方向
    if(abs(accel_x) > abs(accel_y)) {
        if(accel_x > TILT_THRESHOLD) return DIR_RIGHT;
        if(accel_x < -TILT_THRESHOLD) return DIR_LEFT;
    } else {
        if(accel_y > TILT_THRESHOLD) return DIR_DOWN;
        if(accel_y < -TILT_THRESHOLD) return DIR_UP;
    }
    
    return DIR_INVALID;
}

/**
 * @brief MPU6050数据读取任务
 */
void vMPU6050Task(void *pvParameters) {
    MPU6050Data_t mpu_data;
    int16_t accel_x, accel_y, accel_z;
    int16_t gyro_x, gyro_y, gyro_z;
    
    // 初始化MPU6050
    MPU6050_Init();
    
    while(1) {
        // 读取两个MPU6050的数据
        for(uint8_t player = 1; player <= 2; player++) {
            // 切换I2C设备地址
            uint8_t mpu_addr = (player == 1) ? MPU6050_ADDR1 : MPU6050_ADDR2;
            
            // 使用适配器函数读取MPU6050数据
            if(mpu6050_read_accel_simple(mpu_addr, &accel_x, &accel_y, &accel_z) == 0) {
                // 方向判断
                Direction_e new_direction = GetDirectionFromMPU(accel_x, accel_y);
                
                if(new_direction != DIR_INVALID) {
                    mpu_data.player_id = player;
                    mpu_data.direction = new_direction;
                    
                    // 发送到队列（非阻塞）
                    xQueueSend(xMPU6050Queue, &mpu_data, 0);
                }
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(50)); // 20Hz采样率
    }
}

/**
 * @brief 游戏逻辑任务
 */
void vGameLogicTask(void *pvParameters) {
    MPU6050Data_t mpu_data;
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(200); // 游戏更新频率200ms
    
    // 初始化游戏
    snake_game_init();
    g_game.game_state = GAME_RUNNING;
    
    // 初始化周期性任务
    xLastWakeTime = xTaskGetTickCount();
    
    while(1) {
        // 处理MPU6050输入
        while(xQueueReceive(xMPU6050Queue, &mpu_data, 0) == pdTRUE) {
            if(xSemaphoreTake(xGameStateMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                snake_process_input(&g_game, mpu_data.player_id, mpu_data.direction);
                xSemaphoreGive(xGameStateMutex);
            }
        }
        
        // 游戏状态更新
        if(xSemaphoreTake(xGameStateMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            if(g_game.game_state == GAME_RUNNING) {
                snake_update_game(&g_game);
                
                // 通知显示任务更新
                xSemaphoreGive(xDisplaySemaphore);
            }
            xSemaphoreGive(xGameStateMutex);
        }
        
        // 等待下一个周期
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/**
 * @brief 显示任务
 */
void vDisplayTask(void *pvParameters) {
    // 初始显示
    if(xSemaphoreTake(xGameStateMutex, portMAX_DELAY) == pdTRUE) {
        snake_draw_game(&g_game);
        xSemaphoreGive(xGameStateMutex);
    }
    
    while(1) {
        // 等待显示更新信号
        if(xSemaphoreTake(xDisplaySemaphore, portMAX_DELAY) == pdTRUE) {
            if(xSemaphoreTake(xGameStateMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                snake_draw_game(&g_game);
                xSemaphoreGive(xGameStateMutex);
            }
        }
    }
}

/**
 * @brief 创建所有游戏任务
 */
void snake_create_tasks(void) {
    // 创建任务
    xTaskCreate(vMPU6050Task, "MPU6050", 256, NULL, 3, NULL);      // 高优先级
    xTaskCreate(vGameLogicTask, "GameLogic", 512, NULL, 2, NULL);  // 中优先级
    xTaskCreate(vDisplayTask, "Display", 256, NULL, 1, NULL);      // 低优先级
}
