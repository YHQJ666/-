#ifndef __CONFIG_H
#define __CONFIG_H

// ===== 贪吃蛇游戏配置 =====

// === 显示屏配置 ===
#define SCREEN_WIDTH    160     // ST7735S屏幕宽度
#define SCREEN_HEIGHT   128     // ST7735S屏幕高度

// === 游戏区域配置 ===
#define GAME_WIDTH      20      // 游戏区域宽度（格子数）
#define GAME_HEIGHT     16      // 游戏区域高度（格子数）
#define CELL_SIZE       8       // 每个格子的像素大小

// === 蛇游戏配置 ===
#define MAX_SNAKE_LENGTH    100     // 蛇的最大长度
#define INITIAL_SNAKE_LENGTH 3      // 蛇的初始长度
#define SNAKE_SPEED_MS      200     // 蛇移动间隔（毫秒）

// === MPU6050配置 ===
#define MPU6050_ADDR1       0x68    // 第一个MPU6050地址
#define MPU6050_ADDR2       0x69    // 第二个MPU6050地址
#define TILT_THRESHOLD      4000    // 倾斜检测阈值
#define MPU_SAMPLE_MS       50      // MPU6050采样间隔（毫秒）

// === FreeRTOS任务优先级 ===
#define TASK_PRIORITY_HIGH    3     // MPU6050任务优先级
#define TASK_PRIORITY_NORMAL  2     // 游戏逻辑任务优先级
#define TASK_PRIORITY_LOW     1     // 显示任务优先级

// === 游戏颜色定义（RGB565） ===
#define COLOR_SNAKE1    0xF800      // 红色 - 玩家1的蛇
#define COLOR_SNAKE2    0x001F      // 蓝色 - 玩家2的蛇
#define COLOR_FOOD      0x07E0      // 绿色 - 食物
#define COLOR_BORDER    0xFFFF      // 白色 - 边界
#define COLOR_BG        0x0000      // 黑色 - 背景
#define COLOR_TEXT      0xFFFF      // 白色 - 文字

// === 兼容性定义（保持原有颜色名称） ===
#define RED     COLOR_SNAKE1
#define BLUE    COLOR_SNAKE2
#define GREEN   COLOR_FOOD
#define WHITE   COLOR_BORDER
#define BLACK   COLOR_BG
#define YELLOW  0xFFE0
#define GRAY    0x8410
#define ORANGE  0xFD20

// === 调试配置 ===
#define SNAKE_DEBUG_MODE        1   // 调试模式开关
#define SNAKE_USE_TEST_INPUT    1   // 使用测试输入（不依赖真实MPU6050）

#endif // __CONFIG_H

