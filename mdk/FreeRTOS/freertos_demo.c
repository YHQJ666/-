////#include "freertos.h"
////#include "freertos.c"
////#include "FreeRTOS.h"
////#include "task.h"

////#include "task_game.h"
////#include "task_input.h"
////#include "task_display.h"
////#include "task_bullet.h"

//#include "freertos_demo.h"

//#include "led.h"
//#include "lcd.h"
//#include "key.h"
///*FreeRTOS*********************************************************************************************/
//#include "FreeRTOS.h"
//#include "task.h"

///******************************************************************************************************/
///*FreeRTOS配置*/

///* START_TASK 任务 配置
// * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
// */
//#define START_TASK_PRIO 1                   /* 任务优先级 */
//#define START_STK_SIZE  128                 /* 任务堆栈大小 */
//TaskHandle_t            StartTask_Handler;  /* 任务句柄 */
//void start_task(void *pvParameters);        /* 任务函数 */

///* TASK1 任务 配置
// * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
// */
//#define TASK1_PRIO      2                   /* 任务优先级 */
//#define TASK1_STK_SIZE  128                 /* 任务堆栈大小 */
//TaskHandle_t            Task1Task_Handler;  /* 任务句柄 */
//void task1(void *pvParameters);             /* 任务函数 */

///* TASK2 任务 配置
// * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
// */
//#define TASK2_PRIO      3                   /* 任务优先级 */
//#define TASK2_STK_SIZE  128                 /* 任务堆栈大小 */
//TaskHandle_t            Task2Task_Handler;  /* 任务句柄 */
//void task2(void *pvParameters);             /* 任务函数 */

///* TASK3 任务 配置
// * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
// */
//#define TASK3_PRIO      4                   /* 任务优先级 */
//#define TASK3_STK_SIZE  128                 /* 任务堆栈大小 */
//TaskHandle_t            Task3Task_Handler;  /* 任务句柄 */
//void task3(void *pvParameters);             /* 任务函数 */

///******************************************************************************************************/

///* LCD刷屏时使用的颜色 */
//uint16_t lcd_discolor[11] = {WHITE, BLACK, BLUE, RED,
//                             MAGENTA, GREEN, CYAN, YELLOW,
//                             BROWN, BRRED, GRAY};

///**
// * @brief       FreeRTOS例程入口函数
// * @param       无
// * @retval      无
// */
//void freertos_demo(void)
//{    
//   	USART_SendString("demo正在运行");
//    xTaskCreate((TaskFunction_t )start_task,            /* 任务函数 */
//                ( char*    )"start_task",          /* 任务名称 */
//                (uint16_t       )START_STK_SIZE,        /* 任务堆栈大小 */
//                (void*          )NULL,                  /* 传入给任务函数的参数 */
//                (UBaseType_t    )START_TASK_PRIO,       /* 任务优先级 */
//                (TaskHandle_t*  )&StartTask_Handler);   /* 任务句柄 */    
//	vTaskStartScheduler();
//	USART_SendString("Scheduler failed to start!\r\n");
//}
//// void start_task( void * pvParameters )
//// {
////	 while(1)
////	 {
////		 
////	 }
//// }

///**
// * @brief       start_task
// * @param       pvParameters : 传入参数(未用到)
// * @retval      无
// */
//void start_task(void *pvParameters)
//{
//    taskENTER_CRITICAL();           /* 进入临界区 */
////	vTaskDelay(1000);
//    /* 创建任务1 */
//    xTaskCreate((TaskFunction_t )task1,
//                (const char*    )"task1",
//                (uint16_t       )TASK1_STK_SIZE,
//                (void*          )NULL,
//                (UBaseType_t    )TASK1_PRIO,
//                (TaskHandle_t*  )&Task1Task_Handler);
//	USART_SendString("创建任务1\n");
//    /* 创建任务2 */
//    xTaskCreate((TaskFunction_t )task2,
//                (const char*    )"task2",
//                (uint16_t       )TASK2_STK_SIZE,
//                (void*          )NULL,
//                (UBaseType_t    )TASK2_PRIO,
//                (TaskHandle_t*  )&Task2Task_Handler);
//	USART_SendString("创建任务2\n");
//	/* 创建任务3 */
//    xTaskCreate((TaskFunction_t )task3,
//                (const char*    )"task3",
//                (uint16_t       )TASK3_STK_SIZE,
//                (void*          )NULL,
//                (UBaseType_t    )TASK3_PRIO,
//                (TaskHandle_t*  )&Task3Task_Handler);
//    USART_SendString("创建任务3\n");
//	vTaskDelete(StartTask_Handler); /* 删除开始任务 */
//    taskEXIT_CRITICAL();            /* 退出临界区 */
//}

///**
// * @brief       task1
// * @param       pvParameters : 传入参数(未用到)
// * @retval      无
// */
//void task1(void *pvParameters)
//{    
//    while(1)
//    {
//		USART_SendString("任务1正在运行\n");
//        led_toggle();													/*led闪烁*/
//		vTaskDelay(1000);                                               /* 延时1000ticks */
//    }
//}

///**
// * @brief       task2
// * @param       pvParameters : 传入参数(未用到)
// * @retval      无
// */
//void task2(void *pvParameters)
//{
//    while(1)
//    {
//		USART_SendString("任务2正在运行\n");
//        led2_toggle();
//        vTaskDelay(1000);                           /* 延时1000ticks */
//    }
//}
///**
// * @brief       task3
// * @param       pvParameters : 传入参数(未用到)
// * @retval      无
// */
//void task3(void *pvParameters) {
//    bool key_pressed = false;
//    uint32_t debounce_time = 0;
//	bool task_suspended = false;
//    
//    while (1) {
//        bool key_state = key_read();  // 非阻塞读取
//        // 按下事件（下降沿 + 去抖）
//        if (!key_pressed && key_state) {
//            debounce_time = xTaskGetTickCount();
//            key_pressed = true;
//        }
//        
//        // 确认有效按下（去抖后）
//        if (key_pressed && key_state && 
//            (xTaskGetTickCount() - debounce_time) > 20) {
//            
//			USART_SendString("按下\n");
//            if (!task_suspended) {
//                USART_SendString("挂起task2\r\n");
//                vTaskSuspend(Task2Task_Handler);
//                task_suspended = true;
//            } 
//			else {
//                USART_SendString("恢复task1\r\n");
//                vTaskResume(Task2Task_Handler);
//                task_suspended = false;
//            }
//            
//            // 等待按键释放（非阻塞）
//            while (key_read()) {
//                vTaskDelay(1);  // 每1ms检查一次
//            }
//            
//            key_pressed = false;
//            vTaskDelay(pdMS_TO_TICKS(200));  // 防连按
//        }
//        
//        vTaskDelay(1);  // 让出CPU
//    }
//}