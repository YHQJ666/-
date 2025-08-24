#ifndef SNAKE_H
#define SNAKE_H

#include "stm32f10x.h"
#include "st7735.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// 游戏配置
#define GAME_WIDTH          20      // 游戏区域宽度（格子数）
#define GAME_HEIGHT         16      // 游戏区域高度（格子数）
#define CELL_SIZE           8       // 每个格子的像素大小
#define MAX_SNAKE_LENGTH    100     // 蛇的最大长度
#define INITIAL_SNAKE_LENGTH 3      // 蛇的初始长度

// 游戏状态
typedef enum {
    GAME_READY = 0,
    GAME_RUNNING = 1,
    GAME_PAUSED = 2,
    GAME_OVER = 3
} GameState_e;

// 方向枚举
typedef enum {
    DIR_UP = 0,
    DIR_RIGHT = 1,
    DIR_DOWN = 2,
    DIR_LEFT = 3,
    DIR_INVALID = 4
} Direction_e;

// 坐标结构
typedef struct {
    uint8_t x;
    uint8_t y;
} Position_t;

// 蛇身节点
typedef struct SnakeNode {
    Position_t pos;
    struct SnakeNode* next;
} SnakeNode_t;

// 蛇结构
typedef struct {
    SnakeNode_t* head;
    SnakeNode_t* tail;
    Direction_e direction;
    Direction_e next_direction;
    uint8_t length;
    uint16_t score;
    uint8_t alive;
    uint16_t color;
} Snake_t;

// 游戏状态结构
typedef struct {
    Snake_t snake1;
    Snake_t snake2;
    Position_t food;
    GameState_e game_state;
    uint32_t game_time;
    uint8_t winner;  // 0:无, 1:玩家1, 2:玩家2, 3:平局
} Game_t;

// MPU6050数据结构
typedef struct {
    uint8_t player_id;  // 1或2
    Direction_e direction;
} MPU6050Data_t;

// 全局变量声明
extern Game_t g_game;
extern QueueHandle_t xMPU6050Queue;
extern SemaphoreHandle_t xDisplaySemaphore;
extern SemaphoreHandle_t xGameStateMutex;

// 函数声明
void snake_game_init(void);
void snake_init(Snake_t* snake, uint8_t start_x, uint8_t start_y, Direction_e dir, uint16_t color);
void snake_add_head(Snake_t* snake, Position_t new_pos);
void snake_remove_tail(Snake_t* snake);
void snake_move(Snake_t* snake);
uint8_t snake_check_self_collision(Snake_t* snake);
uint8_t snake_check_wall_collision(Snake_t* snake);
uint8_t snake_check_snake_collision(Snake_t* snake1, Snake_t* snake2);
uint8_t snake_check_food_collision(Snake_t* snake, Position_t food);
void snake_generate_food(Game_t* game);
void snake_process_input(Game_t* game, uint8_t player_id, Direction_e direction);
void snake_update_game(Game_t* game);
void snake_draw_game(Game_t* game);
void snake_draw_snake(Snake_t* snake);
void snake_draw_food(Position_t food);
void snake_draw_border(void);
void snake_draw_score(Game_t* game);

// FreeRTOS任务函数
void vMPU6050Task(void *pvParameters);
void vGameLogicTask(void *pvParameters);
void vDisplayTask(void *pvParameters);
void snake_create_tasks(void);

#endif // SNAKE_H
