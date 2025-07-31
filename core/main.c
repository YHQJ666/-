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
#include "freertos_demo.h"
#include "task.h"
#include "delay.h"
#include "stm32f10x_usart.h"
#include "mpu6050.h"
#include "st7735.h"
#include "stfonts.h"

TaskHandle_t myTaskHandler;

//// 实现钩子函数
//void vApplicationIdleHook(void) {
//    static uint32_t lastPrintTick = 0;
//    uint32_t currentTick = xTaskGetTickCount();
//    
//    // 每隔一段时间打印（避免刷屏）
//    if (currentTick - lastPrintTick >= 1000) {  // 每1000个tick打印一次
//        lastPrintTick = currentTick;
//        }
//}
//void myTask(void * arg)
//{ 
//	USART_SendString_IT("进入任务\n");
//	while(1)
//	{
////		GPIO_SetBits(GPIOA, GPIO_Pin_2);
////		
////		USART_SendString_IT("LED ON\n");
//		vTaskDelay(5);
//		USART_SendString_IT("1\n");
//		
//		GPIO_ResetBits(GPIOA, GPIO_Pin_2);
//		
//		USART_SendString_IT("LED OFF\n");
//		vTaskDelay(500 / portTICK_PERIOD_MS);
//		USART_SendString_IT("2\n");
//	}
//}
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

int main(void)
{
    board_lowlevel_init();
	usart_Init();
    st7735_init();
	lcd_init();
    mpu6050_init();
	led_init();

	key_init();
	
	st7735_fill_screen(RED); // 填红色
	st7735_fill_screen(GRAY);
    
	USART_SendString("66666688888\r\n");  // 测试串口
    USART_SendString("System Started!\n");  // 测试串口
	st7735_fill_screen(RED);
	char str[64];
    while (1)
    {
        Delay(20);
		USART_SendString("循环中 6");
        mpu6050_accel_t accel;
        mpu6050_read_accel(&accel);

        mpu6050_gyro_t gyro;
        mpu6050_read_gyro(&gyro);

        float temp = mpu6050_read_temper();

        sprintf(str, "Accel: %.2f, %.2f, %.2f\r\n", accel.x, accel.y, accel.z);
		USART_SendString(str);
		st7735_fill_rect(0, 0, 128, 2, GREEN);
        st7735_write_string(0,0,str,&font_ascii_8x16,RED,GREEN);
		Delay(200);
	}
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_ResetBits(GPIOA, GPIO_Pin_2);
	
	NVIC_PriorityGroupConfig (NVIC_PriorityGroup_4);  //将所有优先级位指定为抢占优先级位，不留下任何优先级位作为子优先级位
	freertos_demo();
	
}
