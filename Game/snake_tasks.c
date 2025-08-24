#include "snake.h"
#include "mpu6050.h"
#include "config.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// 外部声明
extern void mpu6050_read_accel(mpu6050_accel_t *accel);
extern Direction_e get_test_direction(uint8_t player_id);

// MPU6050地址定义
#define MPU6050_ADDR1   0x68
#define MPU6050_ADDR2   0x69

// 从MPU6050数据判断方向
#define TILT_THRESHOLD   0.3f  // 倾斜阈值,0.5g 就触发
#define DEBOUNCE_TIME_MS 200   // 防抖时间（200ms内方向不变才确认）

static inline Direction_e dir_from_accel(float ax, float ay) {
    Direction_e dir = DIR_INVALID;
    if (fabsf(ax) > fabsf(ay)) {
        if (ax >  TILT_THRESHOLD) dir = DIR_UP;
        else if (ax < -TILT_THRESHOLD) dir = DIR_DOWN;
    } else {
        if (ay >  TILT_THRESHOLD) dir = DIR_LEFT;
        else if (ay < -TILT_THRESHOLD) dir = DIR_RIGHT;
    }
    return dir;
}

void vMPU6050Task(void *pvParameters) {
	st7735_fill_screen(0x0000);
    if (MPU6050_Init(MPU6050_ADDR1) == 0||MPU6050_Init(MPU6050_ADDR2) == 0) {
        st7735_fill_screen(0x0000);
        st7735_write_string(2, 10, "MPU INIT FAIL", &font_ascii_8x16, 0xF800, 0x0000);
        vTaskDelete(NULL);
    }
    mpu6050_accel_t accel1 = {0};
	mpu6050_accel_t accel2 = {0};
    char buf[32];
	MPU6050Data_t mpu_data;
	static Direction_e last_direction[2] = {DIR_INVALID, DIR_INVALID};
	
    while (1) {
        if(MPU6050_ReadAccel(MPU6050_ADDR1, &accel1)==1){
			Direction_e dir = dir_from_accel(accel1.x, accel1.y);
			// 仅当方向变化时发送消息
			if (dir != DIR_INVALID && dir != last_direction[0]) {
				last_direction[0] = dir;
				mpu_data.player_id = 1;
				mpu_data.direction = dir;
				xQueueSend(xMPU6050Queue, &mpu_data, 0);
			}
		}
		if(MPU6050_ReadAccel(MPU6050_ADDR2, &accel2)==1){
			Direction_e dir = dir_from_accel(accel2.x, accel2.y);
			// 仅当方向变化时发送消息
			if (dir != DIR_INVALID && dir != last_direction[1]) {
				last_direction[1] = dir;
				mpu_data.player_id = 2;
				mpu_data.direction = dir;
				xQueueSend(xMPU6050Queue, &mpu_data, 0);
			}
		}
//        st7735_fill_rect(0, 130, 160, 16, 0x0000);
//        sprintf(buf, "DIR=%s", dir_str);
//        st7735_write_string(2, 100, buf, &font_ascii_8x16, 0xF800, 0x0000);
//		
//		st7735_fill_rect(0, 10, 160, 16, 0x0000);
//        sprintf(buf, "X=%.2f", accel.x);
//        st7735_write_string(2, 10, buf, &font_ascii_8x16, 0xFFFF, 0x0000);

//        st7735_fill_rect(0, 30, 160, 16, 0x0000);
//        sprintf(buf, "Y=%.2f", accel.y);
//        st7735_write_string(2, 30, buf, &font_ascii_8x16, 0xFFFF, 0x0000);
		
        vTaskDelay(pdMS_TO_TICKS(10)); // 10Hz 刷新
    }
}
// MPU6050任务
//void vMPU6050Task(void *pvParameters) {
//    static Direction_e last_direction[2] = {DIR_INVALID, DIR_INVALID};
//    MPU6050Data_t mpu_data;
//    int16_t accel_x, accel_y, accel_z;

//    while (1) {
//        // 玩家1
//        if (mpu6050_read_accel_simple(MPU6050_ADDR1, &accel_x, &accel_y, &accel_z) == 0) {
//            Direction_e direction = GetDirectionFromMPU(accel_x, accel_y);
//            // 仅当方向变化时发送消息
//            if (direction != DIR_INVALID && direction != last_direction[0]) {
//                last_direction[0] = direction;
//                mpu_data.player_id = 1;
//                mpu_data.direction = direction;
//                xQueueSend(xMPU6050Queue, &mpu_data, 0);
//            }
//        }

//        // 玩家2
//        Direction_e test_dir = get_test_direction(2);
//        if (test_dir != DIR_INVALID && test_dir != last_direction[1]) {
//            last_direction[1] = test_dir;
//            mpu_data.player_id = 2;
//            mpu_data.direction = test_dir;
//            xQueueSend(xMPU6050Queue, &mpu_data, 0);
//        }

//        vTaskDelay(pdMS_TO_TICKS(100)); // 降低采样频率到100ms
//    }
//}

// 游戏逻辑任务
void vGameLogicTask(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(100); // 严格100ms周期

    while (1) {
        // 处理输入（最多处理2条消息避免阻塞）
        MPU6050Data_t mpu_data;
        uint8_t msg_count = 0;
        while (msg_count++ < 2 && xQueueReceive(xMPU6050Queue, &mpu_data, 0) == pdTRUE) {
            if (xSemaphoreTake(xGameStateMutex, pdMS_TO_TICKS(1)) == pdTRUE) {
                if (g_game.game_state == GAME_RUNNING) {
                    snake_process_input(&g_game, mpu_data.player_id, mpu_data.direction);
                }
                xSemaphoreGive(xGameStateMutex);
            }
        }

        // 更新游戏状态
        if (xSemaphoreTake(xGameStateMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
            if (g_game.game_state == GAME_RUNNING) {
                snake_update_game(&g_game);
            }
            xSemaphoreGive(xGameStateMutex);
            xSemaphoreGive(xDisplaySemaphore); // 通知刷新
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency); // 确保严格周期
    }
}

// 显示任务
// 在显示任务中添加双缓冲
void vDisplayTask(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xRefreshRate = pdMS_TO_TICKS(33); // 每150ms刷新一次
    snake_draw_init(&g_game);
    while (1) {
        // 等待信号量但限制最大等待时间
        if (xSemaphoreTake(xDisplaySemaphore, xRefreshRate) == pdTRUE) {
            if (xSemaphoreTake(xGameStateMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                if (g_game.game_state == GAME_RUNNING) {
                    // 刷蛇
                    snake_draw_step(&g_game.snake1, g_game.food);
                    snake_draw_step(&g_game.snake2, g_game.food);
                }
                xSemaphoreGive(xGameStateMutex);
            }
        }
        
        // 强制限速刷新
        vTaskDelayUntil(&xLastWakeTime, xRefreshRate);
    }
}



// 创建所有游戏任务
void snake_create_tasks(void) {
    // 创建队列和信号量
    xMPU6050Queue     = xQueueCreate(10, sizeof(MPU6050Data_t));
    xDisplaySemaphore = xSemaphoreCreateBinary();
    xGameStateMutex   = xSemaphoreCreateMutex();

    if (xMPU6050Queue == NULL || xDisplaySemaphore == NULL || xGameStateMutex == NULL) {
        USART_SendString("ERROR: Failed to create queues/semaphores!\r\n");
        return;
    }

    srand(xTaskGetTickCount());
    
    // 初始化游戏
    snake_game_init();
    g_game.game_state = GAME_RUNNING;

    // 创建任务
    BaseType_t result1 = xTaskCreate(vMPU6050Task,  "MPU6050", 512,  NULL, 2, NULL);
    BaseType_t result2 = xTaskCreate(vGameLogicTask,"GameLogic",1024, NULL, 3, NULL);
    BaseType_t result3 = xTaskCreate(vDisplayTask,  "Display", 512,  NULL, 1, NULL);

    if (result1 != pdPASS || result2 != pdPASS || result3 != pdPASS) {
        USART_SendString("ERROR: Failed to create one or more tasks!\r\n");
        return;
    }

    xSemaphoreGive(xDisplaySemaphore);
}
