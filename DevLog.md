# 开发日志 
## [2025-07-22] 示例日志条目

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



# 开发日志：陀螺仪数据滤波方案实现

## 日期：2023-11-20  
## 目标：实现MPU6050陀螺仪数据的低通滤波与校准优化  

---

## 一、问题背景
### 原始数据问题
- 静止状态下陀螺仪噪声：±0.2~0.5°/s  
- 快速运动时数据延迟明显  
- 零偏（Bias）导致静止时非零输出  

---

## 二、解决方案设计
### 1. 低通滤波（一阶指数平滑）
**公式**：  
`filtered = α * raw + (1-α) * prev_filtered`  
**参数**：  
- `α=0.2`（经测试平衡延迟与降噪）  
- 采样频率：100Hz  

### 2. 校准优化
```c
// 校准流程
1. 传感器静止放置
2. 采集1000个样本求均值
3. 计算各轴零偏：
   gyro_bias = Σ(raw_data) / sample_count

### 3.代码实现：
void mpu6050_read_gyro(mpu6050_gyro_t *gyro) {
    // 1. 读取原始数据
    uint8_t raw_data[6];
    swi2c_read(MPU6050_ADDR, GYRO_XOUT_H_REG, raw_data, 6);
   // 2. 转换并校准
    float x = (int16_t)(raw_data[0]<<8 | raw_data[1]) / 131.0f - gyro_bias_x;
    // 3. 低通滤波
    gyro_filter.x = gyro_filter.alpha * x + (1-gyro_filter.alpha)*gyro_filter.x;
    // 4. 输出
    gyro->x = gyro_filter.x;
    // y,z 轴同理...
}

### 4.测试结果
将静态噪声降低至±0.1°/s以内，满足嵌入式应用需求。