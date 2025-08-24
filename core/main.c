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
    ST7735_WriteString(30, 50, "SNAKE GAME", Font_11x18, 0xFFFF, 0x0000);
    ST7735_WriteString(20, 80, "Loading...", Font_7x10, 0x07E0, 0x0000);
    
    // 延时显示启动画面
    for(volatile int i = 0; i < 1000000; i++);
    
    // 设置FreeRTOS优先级组
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    
    // 创建贪吃蛇游戏任务
    snake_create_tasks();
    
    USART_SendString("Tasks created, starting scheduler...\r\n");
    
    // 启动FreeRTOS调度器
    vTaskStartScheduler();
    
    // 正常情况下不会执行到这里
    while(1) {
        USART_SendString("Scheduler failed!\r\n");
        for(volatile int i = 0; i < 1000000; i++);
    }
}
