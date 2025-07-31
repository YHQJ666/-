#include <stdbool.h>
#include <stm32f10x.h>
#include <stm32f10x_gpio.h>
#include "key.h"
#include "Delay.h"

#define KEY_PORT    GPIOA
#define KEY_PIN     GPIO_Pin_7

void key_init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // Input Pull Up
    GPIO_InitStructure.GPIO_Pin  = KEY_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(KEY_PORT,&GPIO_InitStructure);
}

bool key_read(void)
{
	return (GPIO_ReadInputDataBit(KEY_PORT, KEY_PIN) == 0);
}

void key_wait_release(void)
{
    while(key_read());
}
