#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stm32f10x.h>
#include "stm32f10x_conf.h"
#include "USART.h"
#include <stdarg.h>

#define TX_BUFFER_SIZE 512  // 根据需求调整大小
#define USARTx USART1
#define USARTx_CLK RCC_APB2Periph_USART1
#define USART_APBxClkCmd RCC_APB2PeriphClockCmd  
#define USART1_BAUDRATE 115200

#define USARTx_GPIO_CLK RCC_APB2Periph_GPIOA
#define USART_GPIO_APBxClkCmd RCC_APB2PeriphClockCmd   

#define USARTx_GPIO_TXPORT GPIOA
#define USARTx_GPIO_TX_PIN GPIO_Pin_9
#define USARTx_GPIO_RXPORT GPIOA
#define USARTx_GPIO_RX_PIN GPIO_Pin_10

#define USART_IRQ USART1_IRQn
#define USART_IRQHANDLER USART1_IRQHandler


typedef struct {
    uint8_t buffer[TX_BUFFER_SIZE];
    volatile uint16_t head;  // 写入位置
    volatile uint16_t tail;  // 读取位置
} UART_TxBuffer_t;

static UART_TxBuffer_t txBuffer;
static volatile bool isTransmitting = false;

// 初始化串口（波特率等配置）
//void USART_Init(void) {
//    // 你的原有硬件初始化代码（GPIO、USART配置等）
//    // ...
//    
//    // 启用串口发送中断
//    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
//    NVIC_EnableIRQ(USART1_IRQn);
//}

// 非阻塞发送字符串（返回是否成功入队）
//bool USART_SendString(const char *str) {
//    if (!str) return false;
//    
//    // 将字符串写入缓冲区
//    while (*str != '\0') {
//        uint16_t nextHead = (txBuffer.head + 1) % TX_BUFFER_SIZE;
//        
//        // 缓冲区满时等待（可加超时机制）
//        if (nextHead == txBuffer.tail) {
//            return false;  // 缓冲区满，发送失败
//        }
//        
//        txBuffer.buffer[txBuffer.head] = *str++;
//        txBuffer.head = nextHead;
//    }
//    
//    // 如果当前没有在发送，则触发发送中断
//    if (!isTransmitting) {
//        isTransmitting = true;
//        USART_ITConfig(USART1, USART_IT_TXE, ENABLE);  // 启用TXE中断
//    }
//    
//    return true;
//}

//// 串口中断处理函数
//void USART1_IRQHandler(void) {
//    if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET) {
//        if (txBuffer.head != txBuffer.tail) {
//            // 发送缓冲区中的数据
//            USART_SendData(USART1, txBuffer.buffer[txBuffer.tail]);
//            txBuffer.tail = (txBuffer.tail + 1) % TX_BUFFER_SIZE;
//        } else {
//            // 缓冲区空，关闭TXE中断
//            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
//            isTransmitting = false;
//        }
//    }
//}




void usart_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    USART_APBxClkCmd(USARTx_CLK, ENABLE);          //打开串口外设时钟
    USART_GPIO_APBxClkCmd(USARTx_GPIO_CLK, ENABLE);//打开串口GPIO时钟

    GPIO_InitStructure.GPIO_Pin = USARTx_GPIO_TX_PIN | USARTx_GPIO_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;     //复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(USARTx_GPIO_TXPORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = USARTx_GPIO_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //浮空输入
    //USART RX是输入模式，GPIO_Speed参数无效。GPIO_Speed 主要影响的是 输出模式的驱动能力（如推挽输出时的翻转速度）。
    GPIO_Init(USARTx_GPIO_RXPORT, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = USART1_BAUDRATE;        //波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;  //数据位长度
    USART_InitStructure.USART_StopBits = USART_StopBits_1;       //停止位长度
    USART_InitStructure.USART_Parity = USART_Parity_No;          //无校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  //无硬件流控
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;  //收发模式
    USART_Init(USARTx, &USART_InitStructure);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);  // 先关闭发送中断
    USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);  // 可选：启用接收中断
    

//    NVIC_EnableIRQ(USART1_IRQn);

    USART_Cmd(USARTx, ENABLE);
}

#define USART_TIMEOUT 1000 // 1ms超时

bool USART_SendByte(uint8_t Byte) {
    uint32_t timeout = 0;
    while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET) {
        if (++timeout > USART_TIMEOUT) return false;
    }
    USART_SendData(USARTx, Byte);
    return true;
}
void USART_SendArray(uint8_t *Array, uint16_t Length)
{
    for (uint16_t i = 0; i < Length; i++)
    {
        USART_SendByte(Array[i]);
    }
}
void USART_SendString(char *String)
{
    while (*String != '\0')
    {
        USART_SendByte(*String++);
    }
}
void USART_SendNumber(uint32_t Number, uint8_t Length)
{
    char buffer[11]; // 32位无符号整数最大长度为10位，加上结束符'\0'，所以长度为11
    snprintf(buffer, sizeof(buffer), "%0*u", Length, Number); // 格式化输出
    USART_SendString(buffer); // 发送字符串
}
void USART_Printf(char *format, ...) {
    char String[100];
    va_list arg;               // 1. 声明一个 va_list 变量（指向可变参数的指针）
    va_start(arg, format);     // 2. 初始化 arg，使其指向第一个可变参数
    vsprintf(String, format, arg); // 3. 使用 arg 格式化字符串
    va_end(arg);               // 4. 清理 arg
    USART_SendString(String);
}





