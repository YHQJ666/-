#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "main.h"
#include "swi2c.h"
#include "mpu6050.h"


#define WHO_AM_I_REG        0x75
#define PWR_MGMT_1_REG      0x6B
#define SMPLRT_DIV_REG      0x19
#define ACCEL_CONFIG_REG    0x1C
#define ACCEL_XOUT_H_REG    0x3B
#define TEMP_OUT_H_REG      0x41
#define GYRO_CONFIG_REG     0x1B
#define GYRO_XOUT_H_REG     0x43

#define MPU6050_ADDR    0x68

float accel_bias_x = 0.11; // 你的 X 轴零偏
float accel_bias_y = -0.03; // Y 轴零偏
float accel_bias_z = 0.05; // Z 轴零偏

float gyro_bias_x = -1.10;  // 你的 X 轴零偏
float gyro_bias_y = -0.98; // Y 轴零偏
float gyro_bias_z = 0.35; // Z 轴零偏

	// 定义滤波系数和状态变量
float alpha = 0.2f;  // 低通滤波系数
float filtered_accel_x = 0, filtered_accel_y = 0, filtered_accel_z = 0;
float filtered_gyro_x = 0, filtered_gyro_y = 0, filtered_gyro_z = 0;

bool mpu6050_init(void)
{
    swi2c_init();

    uint8_t whoami;
    swi2c_read(MPU6050_ADDR, WHO_AM_I_REG, &whoami, 1);

    if (whoami != 0x68)
    {
        return false;
    }

    uint8_t data;

    // reset the device
    data = 0;
    swi2c_write(MPU6050_ADDR, PWR_MGMT_1_REG, &data, 1);

    Delay(100);

    // set the sample rate to 1kHz
    data = 0x07;
    swi2c_write(MPU6050_ADDR, SMPLRT_DIV_REG, &data, 1);

    // set the accelerometer configuration to 2g
    data = 0x00;
    swi2c_write(MPU6050_ADDR, ACCEL_CONFIG_REG, &data, 1);

    // set the gyro configuration to 250deg/s
    data = 0x00;
    swi2c_write(MPU6050_ADDR, GYRO_CONFIG_REG, &data, 1);

    return true;
}

void mpu6050_read_accel(mpu6050_accel_t *accel)
{


    uint8_t raw_data[6];
    swi2c_read(MPU6050_ADDR, ACCEL_XOUT_H_REG, raw_data, 6);

    // 转换原始数据（注意符号和比例）
    accel->x_raw = (int16_t)(raw_data[0] << 8 | raw_data[1]);
    accel->y_raw = (int16_t)(raw_data[2] << 8 | raw_data[3]);
    accel->z_raw = (int16_t)(raw_data[4] << 8 | raw_data[5]);

    // 转换为g值并校准零偏（取消注释）
    accel->x = (accel->x_raw / 16384.0f) - accel_bias_x;  // 16384 for ±2g range
    accel->y = (accel->y_raw / 16384.0f) - accel_bias_y;
    accel->z = (accel->z_raw / 16384.0f) - accel_bias_z;

    // 对加速度计单独低通滤波
    filtered_accel_x = alpha * accel->x + (1 - alpha) * filtered_accel_x;
    filtered_accel_y = alpha * accel->y + (1 - alpha) * filtered_accel_y;
    filtered_accel_z = alpha * accel->z + (1 - alpha) * filtered_accel_z;

    // 更新输出
    accel->x = filtered_accel_x;
    accel->y = filtered_accel_y;
    accel->z = filtered_accel_z;
}

	
void mpu6050_read_gyro(mpu6050_gyro_t *gyro) {
    uint8_t raw_data[6];
    
    // 1. 读取原始数据
    swi2c_read(MPU6050_ADDR, GYRO_XOUT_H_REG, raw_data, 6);
    gyro->x_raw = (int16_t)(raw_data[0] << 8 | raw_data[1]);
    gyro->y_raw = (int16_t)(raw_data[2] << 8 | raw_data[3]);
    gyro->z_raw = (int16_t)(raw_data[4] << 8 | raw_data[5]);

    // 2. 转换为°/s并校准零偏 (±250dps范围: 131.0 LSB/(°/s))
    gyro->x = (gyro->x_raw / 131.0f) - gyro_bias_x;
    gyro->y = (gyro->y_raw / 131.0f) - gyro_bias_y;
    gyro->z = (gyro->z_raw / 131.0f) - gyro_bias_z;

    // 3. 应用低通滤波
    filtered_gyro_x = alpha * gyro->x + (1 - alpha) * filtered_gyro_x;
    filtered_gyro_y = alpha * gyro->y + (1 - alpha) * filtered_gyro_y;
    filtered_gyro_z = alpha * gyro->z + (1 - alpha) * filtered_gyro_z;

    // 4. 输出结果
    gyro->x = filtered_gyro_x;
    gyro->y = filtered_gyro_y;
    gyro->z = filtered_gyro_z;
}

float mpu6050_read_temper(void)
{
    uint8_t raw_data[2];

    swi2c_read(MPU6050_ADDR, TEMP_OUT_H_REG, raw_data, 2);

    int16_t temp_raw = (int16_t)(raw_data[0] << 8 | raw_data[1]);

    return (float)temp_raw / 340.0f + 36.53f;
}

