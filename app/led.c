#include <stdbool.h>
#include <stm32f10x.h>
#include "led.h"

#define LED2_PORT GPIOA
#define LED2_PIN  GPIO_Pin_3

static bool led2_state =false;

void led2_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin  = LED2_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED2_PORT,&GPIO_InitStructure);
    GPIO_SetBits(LED2_PORT,LED2_PIN);
}

void led2_set(bool on)
{
    led2_state = on;
    GPIO_WriteBit(LED2_PORT,LED2_PIN,led2_state?Bit_RESET:Bit_SET);
}
void led2_on(void)
{
    led2_set(true);
}
void led2_off(void)
{
    led2_set(false);
}
void led2_toggle(void)
{
    led2_set(!led2_state);
}

#define LED_PORT GPIOA
#define LED_PIN  GPIO_Pin_2

static bool led_state =false;

void led_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin  = LED_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_PORT,&GPIO_InitStructure);
    GPIO_SetBits(LED_PORT,LED_PIN);
}

void led_set(bool on)
{
    led_state = on;
    GPIO_WriteBit(LED_PORT,LED_PIN,led_state?Bit_RESET:Bit_SET);
}
void led_on(void)
{
    led_set(true);
}
void led_off(void)
{
    led_set(false);
}
void led_toggle(void)
{
    led_set(!led_state);
}

