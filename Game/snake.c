#include "snake.h"
#include "mpu6050.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// 全局变量定义
Game_t g_game;
QueueHandle_t xMPU6050Queue;
SemaphoreHandle_t xDisplaySemaphore;
SemaphoreHandle_t xGameStateMutex;

// 颜色定义 (RGB565)
#define COLOR_SNAKE1    0xF800  // 红色
#define COLOR_SNAKE2    0x001F  // 蓝色
#define COLOR_FOOD      0x07E0  // 绿色
#define COLOR_BORDER    0xFFFF  // 白色
#define COLOR_BG        0x0000  // 黑色

// MPU6050倾斜阈值
#define TILT_THRESHOLD  4000

/**
 * @brief 游戏初始化
 */
void snake_game_init(void) {
    // 初始化游戏状态
    memset(&g_game, 0, sizeof(Game_t));
    
    // 初始化蛇1 (左上角开始，向右移动)
    snake_init(&g_game.snake1, 3, 3, DIR_RIGHT, COLOR_SNAKE1);
    
    // 初始化蛇2 (右下角开始，向左移动)
    snake_init(&g_game.snake2, GAME_WIDTH-4, GAME_HEIGHT-4, DIR_LEFT, COLOR_SNAKE2);
    
    // 生成初始食物
    snake_generate_food(&g_game);
    
    g_game.game_state = GAME_READY;
    g_game.game_time = 0;
    g_game.winner = 0;
    
    // 创建FreeRTOS对象
    xMPU6050Queue = xQueueCreate(10, sizeof(MPU6050Data_t));
    xDisplaySemaphore = xSemaphoreCreateBinary();
    xGameStateMutex = xSemaphoreCreateMutex();
}

/**
 * @brief 初始化蛇
 */
void snake_init(Snake_t* snake, uint8_t start_x, uint8_t start_y, Direction_e dir, uint16_t color) {
    snake->head = NULL;
    snake->tail = NULL;
    snake->direction = dir;
    snake->next_direction = dir;
    snake->length = 0;
    snake->score = 0;
    snake->alive = 1;
    snake->color = color;
    
    // 创建初始蛇身
    for(int i = 0; i < INITIAL_SNAKE_LENGTH; i++) {
        Position_t pos;
        switch(dir) {
            case DIR_RIGHT:
                pos.x = (start_x >= i) ? (start_x - i) : 0;
                pos.y = start_y;
                break;
            case DIR_LEFT:
                pos.x = (start_x + i < GAME_WIDTH) ? (start_x + i) : (GAME_WIDTH - 1);
                pos.y = start_y;
                break;
            case DIR_DOWN:
                pos.x = start_x;
                pos.y = (start_y >= i) ? (start_y - i) : 0;
                break;
            case DIR_UP:
                pos.x = start_x;
                pos.y = (start_y + i < GAME_HEIGHT) ? (start_y + i) : (GAME_HEIGHT - 1);
                break;
            default:
                pos.x = start_x;
                pos.y = start_y;
                break;
        }
        snake_add_head(snake, pos);
    }
}

/**
 * @brief 添加蛇头
 */
void snake_add_head(Snake_t* snake, Position_t new_pos) {
    SnakeNode_t* new_node = (SnakeNode_t*)malloc(sizeof(SnakeNode_t));
    if(!new_node) return;
    
    new_node->pos = new_pos;
    new_node->next = snake->head;
    
    if(snake->head == NULL) {
        snake->tail = new_node;
    }
    
    snake->head = new_node;
    snake->length++;
}

/**
 * @brief 移除蛇尾
 */
void snake_remove_tail(Snake_t* snake) {
    if(snake->length <= 1) return;
    
    SnakeNode_t* current = snake->head;
    SnakeNode_t* prev = NULL;
    
    // 找到倒数第二个节点
    while(current->next != NULL) {
        prev = current;
        current = current->next;
    }
    
    if(prev) {
        prev->next = NULL;
        snake->tail = prev;
    }
    
    free(current);
    snake->length--;
}

/**
 * @brief 移动蛇
 */
void snake_move(Snake_t* snake) {
    if(!snake->alive) return;
    
    // 更新方向
    snake->direction = snake->next_direction;
    
    // 计算新头部位置
    Position_t new_head = snake->head->pos;
    switch(snake->direction) {
        case DIR_UP:    new_head.y--; break;
        case DIR_DOWN:  new_head.y++; break;
        case DIR_LEFT:  new_head.x--; break;
        case DIR_RIGHT: new_head.x++; break;
        default: break;
    }
    
    // 添加新头部
    snake_add_head(snake, new_head);
}

/**
 * @brief 检查蛇自身碰撞
 */
uint8_t snake_check_self_collision(Snake_t* snake) {
    if(!snake->alive || snake->length < 4) return 0;
    
    Position_t head_pos = snake->head->pos;
    SnakeNode_t* current = snake->head->next;
    
    while(current != NULL) {
        if(current->pos.x == head_pos.x && current->pos.y == head_pos.y) {
            return 1;
        }
        current = current->next;
    }
    
    return 0;
}

/**
 * @brief 检查墙壁碰撞
 */
uint8_t snake_check_wall_collision(Snake_t* snake) {
    if(!snake->alive) return 0;
    
    Position_t head_pos = snake->head->pos;
    
    return (head_pos.x >= GAME_WIDTH || head_pos.y >= GAME_HEIGHT);
}

/**
 * @brief 检查两蛇碰撞
 */
uint8_t snake_check_snake_collision(Snake_t* snake1, Snake_t* snake2) {
    if(!snake1->alive || !snake2->alive) return 0;
    
    Position_t head1 = snake1->head->pos;
    
    // 检查蛇1头部是否撞到蛇2身体
    SnakeNode_t* current = snake2->head;
    while(current != NULL) {
        if(current->pos.x == head1.x && current->pos.y == head1.y) {
            return 1;
        }
        current = current->next;
    }
    
    return 0;
}

/**
 * @brief 检查食物碰撞
 */
uint8_t snake_check_food_collision(Snake_t* snake, Position_t food) {
    if(!snake->alive) return 0;
    
    Position_t head_pos = snake->head->pos;
    return (head_pos.x == food.x && head_pos.y == food.y);
}

/**
 * @brief 生成食物
 */
void snake_generate_food(Game_t* game) {
    uint8_t valid_position = 0;
    Position_t new_food;
    
    // 随机生成食物位置，确保不在蛇身上
    while(!valid_position) {
        new_food.x = rand() % GAME_WIDTH;
        new_food.y = rand() % GAME_HEIGHT;
        
        valid_position = 1;
        
        // 检查是否在蛇1身上
        SnakeNode_t* current = game->snake1.head;
        while(current != NULL) {
            if(current->pos.x == new_food.x && current->pos.y == new_food.y) {
                valid_position = 0;
                break;
            }
            current = current->next;
        }
        
        // 检查是否在蛇2身上
        if(valid_position) {
            current = game->snake2.head;
            while(current != NULL) {
                if(current->pos.x == new_food.x && current->pos.y == new_food.y) {
                    valid_position = 0;
                    break;
                }
                current = current->next;
            }
        }
    }
    
    game->food = new_food;
}

/**
 * @brief 处理玩家输入
 */
void snake_process_input(Game_t* game, uint8_t player_id, Direction_e direction) {
    Snake_t* snake = (player_id == 1) ? &game->snake1 : &game->snake2;
    
    // 防止反向移动
    Direction_e current_dir = snake->direction;
    if((current_dir == DIR_UP && direction == DIR_DOWN) ||
       (current_dir == DIR_DOWN && direction == DIR_UP) ||
       (current_dir == DIR_LEFT && direction == DIR_RIGHT) ||
       (current_dir == DIR_RIGHT && direction == DIR_LEFT)) {
        return;
    }
    
    snake->next_direction = direction;
}

/**
 * @brief 更新游戏逻辑
 */
void snake_update_game(Game_t* game) {
    if(game->game_state != GAME_RUNNING) return;
    
    // 移动蛇
    snake_move(&game->snake1);
    snake_move(&game->snake2);
    
    // 检查食物碰撞
    if(snake_check_food_collision(&game->snake1, game->food)) {
        game->snake1.score += 10;
        snake_generate_food(game);
    } else {
        snake_remove_tail(&game->snake1);
    }
    
    if(snake_check_food_collision(&game->snake2, game->food)) {
        game->snake2.score += 10;
        snake_generate_food(game);
    } else {
        snake_remove_tail(&game->snake2);
    }
    
    // 检查碰撞
    uint8_t snake1_collision = snake_check_wall_collision(&game->snake1) ||
                              snake_check_self_collision(&game->snake1) ||
                              snake_check_snake_collision(&game->snake1, &game->snake2);
                              
    uint8_t snake2_collision = snake_check_wall_collision(&game->snake2) ||
                              snake_check_self_collision(&game->snake2) ||
                              snake_check_snake_collision(&game->snake2, &game->snake1);
    
    if(snake1_collision) game->snake1.alive = 0;
    if(snake2_collision) game->snake2.alive = 0;
    
    // 检查游戏结束
    if(!game->snake1.alive && !game->snake2.alive) {
        game->game_state = GAME_OVER;
        game->winner = 3; // 平局
    } else if(!game->snake1.alive) {
        game->game_state = GAME_OVER;
        game->winner = 2; // 玩家2获胜
    } else if(!game->snake2.alive) {
        game->game_state = GAME_OVER;
        game->winner = 1; // 玩家1获胜
    }
}

/**
 * @brief 绘制游戏画面
 */
void snake_draw_game(Game_t* game) {
    // 清屏
    ST7735_FillScreen(COLOR_BG);
    
    // 绘制边界
    snake_draw_border();
    
    // 绘制蛇
    snake_draw_snake(&game->snake1);
    snake_draw_snake(&game->snake2);
    
    // 绘制食物
    snake_draw_food(game->food);
    
    // 绘制分数
    snake_draw_score(game);
}

/**
 * @brief 绘制蛇
 */
void snake_draw_snake(Snake_t* snake) {
    if(!snake->alive) return;
    
    SnakeNode_t* current = snake->head;
    while(current != NULL) {
        ST7735_FillRect(
            current->pos.x * CELL_SIZE,
            current->pos.y * CELL_SIZE,
            CELL_SIZE,
            CELL_SIZE,
            snake->color
        );
        current = current->next;
    }
}

/**
 * @brief 绘制食物
 */
void snake_draw_food(Position_t food) {
    ST7735_FillRect(
        food.x * CELL_SIZE,
        food.y * CELL_SIZE,
        CELL_SIZE,
        CELL_SIZE,
        COLOR_FOOD
    );
}

/**
 * @brief 绘制边界
 */
void snake_draw_border(void) {
    // 绘制上下边界
    for(int x = 0; x < GAME_WIDTH; x++) {
        ST7735_FillRect(x * CELL_SIZE, 0, CELL_SIZE, 1, COLOR_BORDER);
        ST7735_FillRect(x * CELL_SIZE, (GAME_HEIGHT-1) * CELL_SIZE + CELL_SIZE - 1, CELL_SIZE, 1, COLOR_BORDER);
    }
    
    // 绘制左右边界
    for(int y = 0; y < GAME_HEIGHT; y++) {
        ST7735_FillRect(0, y * CELL_SIZE, 1, CELL_SIZE, COLOR_BORDER);
        ST7735_FillRect((GAME_WIDTH-1) * CELL_SIZE + CELL_SIZE - 1, y * CELL_SIZE, 1, CELL_SIZE, COLOR_BORDER);
    }
}

/**
 * @brief 绘制分数
 */
void snake_draw_score(Game_t* game) {
    char score_str[32];
    
    // 显示玩家1分数 (红色)
    sprintf(score_str, "P1:%d", game->snake1.score);
    ST7735_WriteString(2, 2, score_str, Font_7x10, COLOR_SNAKE1, COLOR_BG);
    
    // 显示玩家2分数 (蓝色)
    sprintf(score_str, "P2:%d", game->snake2.score);
    ST7735_WriteString(90, 2, score_str, Font_7x10, COLOR_SNAKE2, COLOR_BG);
    
    // 游戏结束时显示获胜者
    if(game->game_state == GAME_OVER) {
        if(game->winner == 1) {
            ST7735_WriteString(50, 60, "P1 WIN!", Font_11x18, COLOR_SNAKE1, COLOR_BG);
        } else if(game->winner == 2) {
            ST7735_WriteString(50, 60, "P2 WIN!", Font_11x18, COLOR_SNAKE2, COLOR_BG);
        } else {
            ST7735_WriteString(55, 60, "DRAW!", Font_11x18, COLOR_BORDER, COLOR_BG);
        }
    }
}
