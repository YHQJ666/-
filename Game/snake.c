#include "snake.h"
#include "mpu6050.h"
#include "st7735.h"
#include "stfonts.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <lcd.h>
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
#define TILT_THRESHOLD  2000

static void draw_cell(uint8_t x, uint8_t y, uint16_t color) {
    st7735_fill_rect(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, color);
}

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
    
    // FreeRTOS对象在snake_create_tasks()中创建，这里不再重复创建
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
void snake_draw_step(Snake_t* snake, Position_t food) {
    if (!snake->alive) return;

    // 1. 画蛇头
    draw_cell(snake->head->pos.x, snake->head->pos.y, snake->color);

    // 2. 判断是否吃到食物
    if (snake->head->pos.x == food.x && snake->head->pos.y == food.y) {
        snake->length++;   // 蛇增长
        snake->score += 10;
        // 不擦尾巴（增长）
    } else {
        // 3. 擦掉旧尾巴
        draw_cell(snake->tail->pos.x, snake->tail->pos.y, COLOR_BG);
    }
}

void snake_move(Snake_t* snake) {
    if(!snake->alive) return;

//    char buf[64];
//    sprintf(buf, "snake_move dir=%d head_before=(%d,%d)\r\n",
//            snake->direction, snake->head->pos.x, snake->head->pos.y);
//    USART_SendString(buf);
	
	// 通过MPU6050获取当前倾斜方向
//    Direction_t tilt_dir = mpu6050_get_tilt_direction(0.5f); // 阈值0.5g
//    if (tilt_dir != DIR_NONE) {
//        snake->next_direction = tilt_dir; // 更新蛇的方向
//    }
	
    snake->direction = snake->next_direction;
    Position_t new_head = snake->head->pos;
    switch(snake->direction) {
        case DIR_UP:    new_head.y--; break;
        case DIR_DOWN:  new_head.y++; break;
        case DIR_LEFT:  new_head.x--; break;
        case DIR_RIGHT: new_head.x++; break;
        default: break;
    }
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
//    char buf[64];
//    sprintf(buf, "[snake_update_game] state=%d\n", game->game_state);
//    USART_SendString(buf);

    if(game->game_state != GAME_RUNNING) {
        USART_SendString("Game not running, skip update\n");
        return;
    }
    
    // 移动蛇
    snake_move(&game->snake1);
//	xSemaphoreGive(xDisplaySemaphore);//给显示任务一个刷新信号
    snake_move(&game->snake2);
//    xSemaphoreGive(xDisplaySemaphore);//给显示任务一个刷新信号

    // 检查食物碰撞
    if(snake_check_food_collision(&game->snake1, game->food)) {
        USART_SendString("Snake1 ate food!\n");
        game->snake1.score += 10;
        snake_generate_food(game);
    } else {
        snake_remove_tail(&game->snake1);
    }
    
    if(snake_check_food_collision(&game->snake2, game->food)) {
        USART_SendString("Snake2 ate food!\n");
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
    
    if(snake1_collision) {
        USART_SendString("Snake1 died!\n");
        game->snake1.alive = 0;
    }
    if(snake2_collision) {
        USART_SendString("Snake2 died!\n");
        game->snake2.alive = 0;
    }
    
    // 检查游戏结束
    if(!game->snake1.alive && !game->snake2.alive) {
        USART_SendString("Game over: draw!\n");
        game->game_state = GAME_OVER;
        game->winner = 3;
    } else if(!game->snake1.alive) {
        USART_SendString("Game over: player 2 wins!\n");
        game->game_state = GAME_OVER;
        game->winner = 2;
    } else if(!game->snake2.alive) {
        USART_SendString("Game over: player 1 wins!\n");
        game->game_state = GAME_OVER;
        game->winner = 1;
    }
}


/**
 * @brief 绘制游戏画面
 */
void snake_draw_init(Game_t* game) {
    st7735_fill_screen(COLOR_BG);   // 只在初始化用一次
    snake_draw_border();
    snake_draw_snake(&game->snake1);
    snake_draw_snake(&game->snake2);
    snake_draw_food(game->food);
    snake_draw_score(game);
}


/**
 * @brief 绘制蛇
 */
void snake_draw_snake(Snake_t* snake) {
    if(!snake->alive) return;
	
//    char buf[64];
//    sprintf(buf, "snake_draw len=%d head=(%d,%d)\r\n",
//            snake->length, snake->head->pos.x, snake->head->pos.y);
//    USART_SendString(buf);
//	
    SnakeNode_t* current = snake->head;
    while(current != NULL) {
        st7735_fill_rect(
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
    st7735_fill_rect(
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
        st7735_fill_rect(x * CELL_SIZE, 0, CELL_SIZE, 1, COLOR_BORDER);
        st7735_fill_rect(x * CELL_SIZE, (GAME_HEIGHT-1) * CELL_SIZE + CELL_SIZE - 1, CELL_SIZE, 1, COLOR_BORDER);
    }
    
    // 绘制左右边界
    for(int y = 0; y < GAME_HEIGHT; y++) {
        st7735_fill_rect(0, y * CELL_SIZE, 1, CELL_SIZE, COLOR_BORDER);
        st7735_fill_rect((GAME_WIDTH-1) * CELL_SIZE + CELL_SIZE - 1, y * CELL_SIZE, 1, CELL_SIZE, COLOR_BORDER);
    }
}

/**
 * @brief 绘制分数
 */
void snake_draw_score(Game_t* game) {
    char score_str[32];
    
    // 显示玩家1分数 (红色)
    sprintf(score_str, "P1:%d", game->snake1.score);
    st7735_write_string(2, 2, score_str, &font_ascii_8x16, COLOR_SNAKE1, COLOR_BG);
    
    // 显示玩家2分数 (蓝色)
    sprintf(score_str, "P2:%d", game->snake2.score);
    st7735_write_string(90, 2, score_str, &font_ascii_8x16, COLOR_SNAKE2, COLOR_BG);
    
    // 游戏结束时显示获胜者
    if(game->game_state == GAME_OVER) {
        if(game->winner == 1) {
            st7735_write_string(50, 60, "P1 WIN!", &font_ascii_8x16, COLOR_SNAKE1, COLOR_BG);
        } else if(game->winner == 2) {
            st7735_write_string(50, 60, "P2 WIN!", &font_ascii_8x16, COLOR_SNAKE2, COLOR_BG);
        } else {
            st7735_write_string(55, 60, "DRAW!", &font_ascii_8x16, COLOR_BORDER, COLOR_BG);
        }
    }
}
