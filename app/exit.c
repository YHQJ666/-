#include <stdbool.h>
#include <stm32f10x.h>
//#include <stm32f10x_gpio.h>
#include "key.h"
#include "exit.h"

#define KEY_PORT    GPIOB
#define KEY_PIN     GPIO_Pin_10

#define KEY_EXTI_PORT_SRC   GPIO_PortSourceGPIOB
#define KEY_EXTI_PIN_SRC    GPIO_PinSource10
#define KEY_EXTI_LINE      EXTI_Line10

void key_exti_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_Init(KEY_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //上拉输入
    GPIO_InitStructure.GPIO_Pin  = KEY_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(KEY_PORT, &GPIO_InitStructure);
    
    GPIO_EXTILineConfig(KEY_EXTI_PORT_SRC, KEY_EXTI_PIN_SRC);
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = KEY_EXTI_LINE;       // 选择 EXTI 线（如 EXTI_Line0）
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; // 设置为中断模式（非事件模式）
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;           // 使能该 EXTI 线
    EXTI_Init(&EXTI_InitStructure);                    // 应用配置
                        
    NVIC_InitTypeDef NVIC_InitStructure;                    
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;  // 使用 EXTI10-15 的 IRQ 通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5; // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        // 子优先级（最低）
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;       // 使能该中断通道
    NVIC_Init(&NVIC_InitStructure);                       // 应用配置
}

/* 定义按键回调函数指针变量（静态全局，仅当前文件可见） */
static key_press_callback_t key_press_callback;

// @brief 检测按键是否按下
static bool key_pressed(void)
{
    /* 读取按键GPIO输入电平，Bit_SET表示按下（假设按键硬件是上拉，按下接地） */
    return GPIO_ReadInputDataBit(KEY_PORT, KEY_PIN) == Bit_SET;
}


 /* @brief 阻塞等待按键释放
 *  @note 该函数会循环检测直到按键释放*/
static void key_wait_release(void)
{
    while (key_pressed());
}

/**
 * @brief 注册按键回调函数
 * @param callback 用户定义的回调函数指针
 * @note 注册的回调函数将在按键中断服务中被调用
 */
void key_exti_register(key_press_callback_t callback)
{
    /* 将用户提供的回调函数保存到静态变量中 */
    key_press_callback = callback;
}
void EXTI15_10_IRQHandler(void)
{
    if(key_pressed()) // 检查按键是否被按下
    {
        if (key_press_callback) // 如果注册了回调函数
        {
            key_press_callback(); // 调用回调函数
        }
        key_wait_release(); // 等待按键释放
    }
    EXTI_ClearITPendingBit(KEY_EXTI_LINE); // 清除中断标志位
}
