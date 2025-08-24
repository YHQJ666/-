/**
 * @file tank_game_freertos.c
 * @brief FreeRTOS坦克游戏完整示例 - 支持ABGR8888格式和双线性插值旋转
 * @author Your Name
 * @date 2025-08-05
 */

#include "tank.h"
#include "bullet.h"
#include "st7735.h"
#include "config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// 任务优先级定义
#define GYRO_TASK_PRIORITY      (tskIDLE_PRIORITY + 3)
#define RENDER_TASK_PRIORITY    (tskIDLE_PRIORITY + 2)
#define INPUT_TASK_PRIORITY     (tskIDLE_PRIORITY + 3)
#define GAME_TASK_PRIORITY      (tskIDLE_PRIORITY + 1)

// 任务堆栈大小
#define GYRO_TASK_STACK_SIZE    256
#define RENDER_TASK_STACK_SIZE  512
#define INPUT_TASK_STACK_SIZE   128
#define GAME_TASK_STACK_SIZE    256

// 游戏状态
typedef struct {
    uint8_t game_running;
    uint8_t player1_fire_request;
    uint8_t player2_fire_request;
    float gyro_angle;
} game_state_t;

static game_state_t game_state = {0};
static SemaphoreHandle_t game_state_mutex;

// 假设的外部接口函数（需要根据您的硬件实现）
extern float mpu6050_get_processed_angle_z(void);
extern uint8_t button_fire_pressed(void);
extern uint8_t button_player2_fire_pressed(void);

/**
 * @brief 陀螺仪数据处理任务
 */
static void gyro_task(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(20); // 50Hz更新频率
    
    while (1) {
        // 读取陀螺仪数据
        float gyro_z = mpu6050_get_processed_angle_z();
        
        // 更新游戏状态
        if (xSemaphoreTake(game_state_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            game_state.gyro_angle = gyro_z;
            xSemaphoreGive(game_state_mutex);
        }
        
        // 更新坦克旋转
        tank_update_from_gyro(0, gyro_z);
        
        // 等待下次唤醒
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/**
 * @brief 输入处理任务
 */
static void input_task(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(10); // 100Hz输入检测
    
    while (1) {
        // 检查发射按钮
        if (button_fire_pressed()) {
            if (xSemaphoreTake(game_state_mutex, pdMS_TO_TICKS(5)) == pdTRUE) {
                game_state.player1_fire_request = 1;
                xSemaphoreGive(game_state_mutex);
            }
        }
        
        if (button_player2_fire_pressed()) {
            if (xSemaphoreTake(game_state_mutex, pdMS_TO_TICKS(5)) == pdTRUE) {
                game_state.player2_fire_request = 1;
                xSemaphoreGive(game_state_mutex);
            }
        }
        
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/**
 * @brief 游戏逻辑处理任务
 */
static void game_task(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(50); // 20Hz游戏逻辑更新
    
    while (1) {
        if (xSemaphoreTake(game_state_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            // 处理发射请求
            if (game_state.player1_fire_request) {
                bullet_fire(0);
                game_state.player1_fire_request = 0;
            }
            
            if (game_state.player2_fire_request) {
                bullet_fire(1);
                game_state.player2_fire_request = 0;
            }
            
            xSemaphoreGive(game_state_mutex);
        }
        
        // 更新子弹
        bullet_update();
        bullet_handle_collisions();
        
        // 玩家2自动旋转（演示用）
        static float player2_angle = 0;
        player2_angle += 2.0f;
        if (player2_angle >= 360.0f) player2_angle = 0;
        tank_set_angle(1, player2_angle);
        
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/**
 * @brief 渲染任务
 */
static void render_task(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(50); // 20FPS渲染
    
    while (1) {
        // 清除屏幕
        st7735_fill_screen(0x0000); // 黑色背景
        
        // 绘制游戏对象
        tank_draw(0);           // 绘制玩家1坦克（红色）
        tank_draw(1);           // 绘制玩家2坦克（蓝色）
        bullet_draw_all();      // 绘制所有子弹
        
        // 显示游戏信息（可选）
        // st7735_write_string(5, 5, "Tank Game", &Font_7x10, 0xFFFF, 0x0000);
        
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/**
 * @brief 初始化游戏系统
 */
void tank_game_init(void) {
    // 创建互斥量
    game_state_mutex = xSemaphoreCreateMutex();
    if (game_state_mutex == NULL) {
        // 错误处理
        while(1);
    }
    
    // 初始化游戏子系统
    st7735_init();
    tank_init();
    bullet_init();
    
    // 设置坦克初始位置
    tank_set_position(0, 40, 80);   // 玩家1
    tank_set_position(1, 88, 80);   // 玩家2
    
    game_state.game_running = 1;
}

/**
 * @brief 创建游戏任务
 */
void tank_game_create_tasks(void) {
    BaseType_t result;
    
    // 创建陀螺仪处理任务
    result = xTaskCreate(gyro_task, "GyroTask", GYRO_TASK_STACK_SIZE, 
                        NULL, GYRO_TASK_PRIORITY, NULL);
    if (result != pdPASS) {
        // 任务创建失败处理
        while(1);
    }
    
    // 创建输入处理任务
    result = xTaskCreate(input_task, "InputTask", INPUT_TASK_STACK_SIZE,
                        NULL, INPUT_TASK_PRIORITY, NULL);
    if (result != pdPASS) {
        while(1);
    }
    
    // 创建游戏逻辑任务
    result = xTaskCreate(game_task, "GameTask", GAME_TASK_STACK_SIZE,
                        NULL, GAME_TASK_PRIORITY, NULL);
    if (result != pdPASS) {
        while(1);
    }
    
    // 创建渲染任务
    result = xTaskCreate(render_task, "RenderTask", RENDER_TASK_STACK_SIZE,
                        NULL, RENDER_TASK_PRIORITY, NULL);
    if (result != pdPASS) {
        while(1);
    }
}

/**
 * @brief 主函数示例
 */
void tank_game_main(void) {
    // 初始化游戏
    tank_game_init();
    
    // 创建任务
    tank_game_create_tasks();
    
    // 启动调度器
    vTaskStartScheduler();
    
    // 不应该到达这里
    while(1);
}

/**
 * @brief 简单的单任务游戏循环（如果不想使用多任务）
 */
void tank_game_simple_loop(void) {
    // 初始化
    st7735_init();
    tank_init();
    bullet_init();
    
    tank_set_position(0, 40, 80);
    tank_set_position(1, 88, 80);
    
    while (1) {
        // 读取陀螺仪并更新坦克旋转
        float gyro_angle = mpu6050_get_processed_angle_z();
        tank_update_from_gyro(0, gyro_angle);
        
        // 检查发射按钮
        if (button_fire_pressed()) {
            bullet_fire(0);
            vTaskDelay(pdMS_TO_TICKS(200)); // 防止连续发射
        }
        
        // 玩家2自动旋转
        static float player2_angle = 0;
        player2_angle += 1.0f;
        if (player2_angle >= 360.0f) player2_angle = 0;
        tank_set_angle(1, player2_angle);
        
        // 更新游戏状态
        bullet_update();
        bullet_handle_collisions();
        
        // 渲染
        st7735_fill_screen(0x0000);
        tank_draw(0);
        tank_draw(1);
        bullet_draw_all();
        
        vTaskDelay(pdMS_TO_TICKS(50)); // 20FPS
    }
}
