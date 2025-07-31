# 开发日志 DevLog

> 记录项目开发过程中遇到的问题、解决方案及思考，方便复盘和团队协作。

---

## [2025-07-22] 示例日志条目

### 一、环境说明  
- MCU：STM32F103  
- 开发环境：Keil MDK5  
- FreeRTOS版本：10.4.6  

### 二、问题描述  
任务调用 `vTaskDelay()` 后程序阻塞，串口只打印一次“进入任务”。

### 三、尝试解决思路  
1. 检查调度器是否启动  
2. 检查 `SysTick_Handler()` 是否实现  
3. 查看串口中断配置  

### 四、尝试过程  
- 注释掉 `SysTick_Handler()` 后任务阻塞  
- 添加 `SysTick_Handler()` 并调用 `xPortSysTickHandler()`  
- 修改中断优先级  

### 五、最终解决方案  
```c
void SysTick_Handler(void)
{
    xPortSysTickHandler();
}
## 五、最终解决方案  
void SysTick_Handler(void)
{
    xPortSysTickHandler();
}

```

* 保留并实现了 `SysTick_Handler()` 函数，解决调度器不工作问题。

### 六、结果验证

* 任务可以正常循环执行，串口持续打印日志
* `vTaskDelay()` 正常延时

### 七、备注与总结

* 注意 FreeRTOS 移植中 `SysTick_Handler` 必须正确实现
* 以后调试中断相关问题时优先检查中断向量表和中断函数实现