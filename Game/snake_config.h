#ifndef SNAKE_CONFIG_H
#define SNAKE_CONFIG_H

// 贪吃蛇游戏配置文件

// 游戏参数配置
#define SNAKE_GAME_SPEED_MS     200     // 游戏更新间隔(毫秒)
#define SNAKE_MPU_SAMPLE_MS     50      // MPU6050采样间隔(毫秒)
#define SNAKE_DISPLAY_PRIORITY  1       // 显示任务优先级
#define SNAKE_LOGIC_PRIORITY    2       // 游戏逻辑任务优先级
#define SNAKE_INPUT_PRIORITY    3       // 输入任务优先级

// 调试配置
#define SNAKE_DEBUG_MODE        1       // 调试模式开关
#define SNAKE_USE_TEST_INPUT    1       // 使用测试输入(不依赖真实MPU6050)

// 显示配置
#define SNAKE_SHOW_GRID         0       // 是否显示网格线
#define SNAKE_SHOW_SCORE        1       // 是否显示分数

// 兼容性配置
#ifndef abs
#define abs(x) ((x) < 0 ? -(x) : (x))
#endif

#ifndef sprintf
extern int sprintf(char *str, const char *format, ...);
#endif

#endif // SNAKE_CONFIG_H
