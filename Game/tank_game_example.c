/**
 * @file tank_game_example.c
 * @brief 坦克游戏使用示例 - 展示如何集成陀螺仪控制和子弹发射
 * @author Your Name
 * @date 2025-08-05
 */

#include "tank.h"
#include "bullet.h"
#include "st7735.h"
#include "config.h"

// 假设的陀螺仪接口函数（需要根据你的实际MPU6050驱动实现）
extern float mpu6050_get_processed_angle_z(void);
extern uint8_t button_is_pressed(void); // 发射按钮

/**
 * @brief 游戏主循环示例
 */
void tank_game_main_loop(void) {
    // 初始化系统
    tank_init();
    bullet_init();
    st7735_init();
    
    // 游戏主循环
    while (1) {
        // 1. 清除屏幕
        st7735_fill_screen(BLACK);
        
        // 2. 读取陀螺仪数据并更新坦克旋转
        float gyro_angle = mpu6050_get_processed_angle_z();
        tank_update_from_gyro(0, gyro_angle); // 更新玩家1坦克
        
        // 3. 检查发射按钮
        if (button_is_pressed()) {
            bullet_fire(0); // 玩家1发射子弹
            vTaskDelay(pdMS_TO_TICKS(200)); // 防止连续发射
        }
        
        // 4. 更新子弹位置
        bullet_update();
        
        // 5. 处理碰撞检测
        bullet_handle_collisions();
        
        // 6. 绘制游戏对象
        tank_draw(0);           // 绘制坦克
        bullet_draw_all();      // 绘制所有子弹
        
        // 7. 延时控制帧率
        vTaskDelay(pdMS_TO_TICKS(50)); // 20FPS
    }
}

/**
 * @brief 双人游戏模式示例
 */
void tank_game_two_player_mode(void) {
    // 初始化系统
    tank_init();
    bullet_init();
    st7735_init();
    
    // 设置两个坦克的初始位置
    tank_set_position(0, 40, 60);   // 玩家1
    tank_set_position(1, 120, 60);  // 玩家2
    
    while (1) {
        // 清除屏幕
        st7735_fill_screen(BLACK);
        
        // 玩家1控制（陀螺仪）
        float gyro_angle = mpu6050_get_processed_angle_z();
        tank_update_from_gyro(0, gyro_angle);
        
        // 玩家1发射
        if (button_is_pressed()) {
            bullet_fire(0);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        
        // 玩家2可以添加其他控制方式（如按键）
        // 这里暂时让玩家2自动旋转作为演示
        static float player2_angle = 0;
        player2_angle += 1.0f;
        if (player2_angle >= 360.0f) player2_angle = 0;
        tank_set_angle(1, player2_angle);
        
        // 更新子弹和碰撞
        bullet_update();
        bullet_handle_collisions();
        
        // 绘制所有对象
        tank_draw(0);
        tank_draw(1);
        bullet_draw_all();
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/**
 * @brief 测试坦克旋转功能
 */
void test_tank_rotation(void) {
    tank_init();
    st7735_init();
    
    // 测试不同角度的坦克绘制
    for (float angle = 0; angle < 360; angle += 10) {
        st7735_fill_screen(BLACK);
        
        tank_set_angle(0, angle);
        tank_draw(0);
        
        // 显示炮口位置（用于调试）
        muzzle_pos_t muzzle = tank_get_muzzle_position(0);
        st7735_fill_rect(muzzle.x-1, muzzle.y-1, 3, 3, RED);
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief 测试子弹发射功能
 */
void test_bullet_firing(void) {
    tank_init();
    bullet_init();
    st7735_init();
    
    // 设置坦克位置和角度
    tank_set_position(0, 80, 64);
    tank_set_angle(0, 45.0f); // 45度角
    
    // 发射几发子弹测试
    for (int i = 0; i < 3; i++) {
        bullet_fire(0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    // 观察子弹飞行
    for (int frame = 0; frame < 100; frame++) {
        st7735_fill_screen(BLACK);
        
        bullet_update();
        tank_draw(0);
        bullet_draw_all();
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/**
 * @brief FreeRTOS任务：陀螺仪数据处理任务
 */
void gyro_task(void *pvParameters) {
    while (1) {
        // 读取陀螺仪数据
        float gyro_z = mpu6050_get_processed_angle_z();
        
        // 更新坦克旋转
        tank_update_from_gyro(0, gyro_z);
        
        // 任务延时
        vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz更新频率
    }
}

/**
 * @brief FreeRTOS任务：游戏渲染任务
 */
void render_task(void *pvParameters) {
    while (1) {
        // 清除屏幕
        st7735_fill_screen(BLACK);
        
        // 更新子弹
        bullet_update();
        bullet_handle_collisions();
        
        // 绘制游戏对象
        tank_draw(0);
        bullet_draw_all();
        
        // 控制帧率
        vTaskDelay(pdMS_TO_TICKS(50)); // 20FPS
    }
}

/**
 * @brief FreeRTOS任务：输入处理任务
 */
void input_task(void *pvParameters) {
    while (1) {
        // 检查发射按钮
        if (button_is_pressed()) {
            bullet_fire(0);
            vTaskDelay(pdMS_TO_TICKS(200)); // 防抖和射击间隔
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/**
 * @brief 创建游戏任务
 */
void create_game_tasks(void) {
    // 创建陀螺仪处理任务
    xTaskCreate(gyro_task, "GyroTask", 256, NULL, 2, NULL);
    
    // 创建渲染任务
    xTaskCreate(render_task, "RenderTask", 512, NULL, 1, NULL);
    
    // 创建输入处理任务
    xTaskCreate(input_task, "InputTask", 128, NULL, 3, NULL);
}
