/**
 * @file system_timer.c
 * @brief 系统定时器实现 - 提供毫秒级时间戳
 * @author Your Name
 * @date 2025-08-05
 */

#include <stdint.h>

// 系统滴答计数器（需要在定时器中断中递增）
static volatile uint32_t system_tick_counter = 0;

/**
 * @brief 获取系统时间戳（毫秒）
 * @return 当前系统时间戳
 */
uint32_t get_system_tick(void) {
    return system_tick_counter;
}

/**
 * @brief 系统滴答中断处理函数
 * @note 需要在1ms定时器中断中调用此函数
 */
void system_tick_handler(void) {
    system_tick_counter++;
}

/**
 * @brief 延时函数（毫秒）
 * @param ms 延时时间（毫秒）
 */
void delay_ms(uint32_t ms) {
    uint32_t start_time = get_system_tick();
    while ((get_system_tick() - start_time) < ms) {
        // 等待
    }
}

/**
 * @brief 重置系统时间戳
 */
void reset_system_tick(void) {
    system_tick_counter = 0;
}

/**
 * @brief 计算两个时间戳之间的差值（处理溢出）
 * @param start_time 开始时间
 * @param end_time 结束时间
 * @return 时间差（毫秒）
 */
uint32_t get_time_diff(uint32_t start_time, uint32_t end_time) {
    if (end_time >= start_time) {
        return end_time - start_time;
    } else {
        // 处理32位溢出情况
        return (0xFFFFFFFF - start_time) + end_time + 1;
    }
}
