/**
 * @file system_timer.h
 * @brief 系统定时器头文件
 * @author Your Name
 * @date 2025-08-05
 */

#ifndef __SYSTEM_TIMER_H
#define __SYSTEM_TIMER_H

#include <stdint.h>

// 系统时间函数
uint32_t get_system_tick(void);
void system_tick_handler(void);
void delay_ms(uint32_t ms);
void reset_system_tick(void);
uint32_t get_time_diff(uint32_t start_time, uint32_t end_time);

#endif
