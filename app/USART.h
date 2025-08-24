#ifndef __USART_H
#define __USART_H


#include <stdio.h>
#include <stdbool.h>
#include "stm32f10x.h"

//void usart_Init(void);
bool USART_SendString(const char *str);  // 非阻塞发送函数
void USART_IRQHandler(void);                // 串口中断处理函数

void usart_Init(void);
bool USART_SendByte(uint8_t Byte);
void USART_SendArray(uint8_t *Array, uint16_t Length);
//void USART_SendString(char *String);
void USART_SendNumber(uint32_t Number, uint8_t Length);
void USART_Printf(char *format, ...);

#endif
