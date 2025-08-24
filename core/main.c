#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "main.h"
#include "led.h"
#include "lcd.h"
#include "key.h"
#include "USART.h"
#include "FreeRTOS.h"
#include "task.h"
#include "delay.h"
#include "stm32f10x_usart.h"
#include "mpu6050.h"
#include "st7735.h"
#include "stfonts.h"
#include "snake.h"
#include <stdlib.h> 
// 贪吃蛇游戏主程序

static void board_lowlevel_init(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
}
// 贪吃蛇游戏 - 已清理坦克相关代码

int main(void)
{
    // 硬件初始化
    board_lowlevel_init();
    usart_Init();
    st7735_init();
    
    // 发送启动信息
    USART_SendString("Snake Game Starting...\r\n");
    
    // 显示启动画面
    st7735_fill_screen(0x0000); // 黑色背景
    st7735_write_string(30, 50, "SNAKE GAME", &font_ascii_8x16, 0xFFFF, 0x0000);
    st7735_write_string(20, 80, "Loading...", &font_ascii_8x16, 0x07E0, 0x0000);
    
    // 延时显示启动画面
    for(volatile int i = 0; i < 1000000; i++);
    
    // 设置FreeRTOS优先级组
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    
    // 创建贪吃蛇游戏任务
    snake_create_tasks();
    USART_SendString("snake_create_tasks!\r\n");
    
    // 启动FreeRTOS调度器
    vTaskStartScheduler();
    
    // 如果执行到这里，说明调度器启动失败
    USART_SendString("ERROR: FreeRTOS scheduler failed to start!\r\n");
    
    // 正常情况下不会执行到这里
    while(1) {
        USART_SendString("Scheduler failed!\r\n");
        for(volatile int i = 0; i < 1000000; i++);
    }
}

/**
 * @brief SysTick中断处理函数 - FreeRTOS任务调度的关键函数
 * @note 这个函数必须存在，否则FreeRTOS任务无法正常调度
 */
void SysTick_Handler(void)
{
    // 调用FreeRTOS的SysTick处理函数
    extern void xPortSysTickHandler(void);
    xPortSysTickHandler();
}
