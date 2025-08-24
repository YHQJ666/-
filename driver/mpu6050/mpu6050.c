#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "main.h"
#include "swi2c.h"
#include "mpu6050.h"
#include "USART.h"

#define WHO_AM_I_REG        0x75
#define PWR_MGMT_1_REG      0x6B
#define SMPLRT_DIV_REG      0x19
#define ACCEL_CONFIG_REG    0x1C
#define ACCEL_XOUT_H_REG    0x3B
#define TEMP_OUT_H_REG      0x41
#define GYRO_CONFIG_REG     0x1B
#define GYRO_XOUT_H_REG     0x43

#define MPU6050_ADDR    0x68

float accel_bias_x = 0.02; //  X ����ƫ.11
float accel_bias_y = -0.01; // Y ����ƫ.03
float accel_bias_z = 0.; // Z ����ƫ.05

float gyro_bias_x = -1.10;  //  X ����ƫ
float gyro_bias_y = -0.98; // Y ����ƫ
float gyro_bias_z = 0.35; // Z ����ƫ

	// �����˲�ϵ����״̬����
float alpha = 0.2f;  // ��ͨ�˲�ϵ��
#define GRAVITY_OFFSET 16384    // 1g��Ӧ��ֵ
float filtered_accel_x = 0, filtered_accel_y = 0, filtered_accel_z = 0;
float filtered_gyro_x = 0, filtered_gyro_y = 0, filtered_gyro_z = 0;

#define MPU6050_ADDR1   0x68   // AD0=0
#define MPU6050_ADDR2   0x69   // AD0=1

#define MPU6050_ACCEL_XOUT_H  0x3B
#define MPU6050_GYRO_XOUT_H   0x43

#define MPU6050_ACCEL_SENS    16384.0f   // ��2g
#define MPU6050_GYRO_SENS     131.0f     // ��250��/s

// === I2C ��ʼ�� ===
#include "stm32f10x.h"
#include <stdbool.h>

// �������
#define I2C_TIMEOUT 10000
static float round2(float x) {
    return roundf(x * 100.0f) / 100.0f;
}
static bool I2C_WaitEvent(uint32_t event) {
    uint32_t timeout = I2C_TIMEOUT;
    while (!I2C_CheckEvent(I2C1, event)) {
        if (timeout-- == 0) return false;
    }
    return true;
}

void I2C1_Init(void) {
    /*����ʱ��*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);    // ����I2C1��ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);   // ����GPIOB��ʱ��
    
    /*GPIO��ʼ��*/
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; // I2C1ʹ��PB6��PB7
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /*I2C��ʼ��*/
    I2C_InitTypeDef I2C_InitStructure;
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_ClockSpeed = 400000;              // ����ٶȵ�400KHz
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_Init(I2C1, &I2C_InitStructure);
    
    /*I2Cʹ��*/
    I2C_Cmd(I2C1, ENABLE);
}

bool I2C_WriteByte(uint8_t devAddr, uint8_t regAddr, uint8_t data) {
    // �ȴ����߿���
    uint32_t timeout = I2C_TIMEOUT;
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)) {
        if (timeout-- == 0) return false;
    }
    
    // ������ʼ����
    I2C_GenerateSTART(I2C1, ENABLE);
    if (!I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) return false;
    
    // �����豸��ַ(дģʽ)
    I2C_Send7bitAddress(I2C1, devAddr << 1, I2C_Direction_Transmitter);
    if (!I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) return false;
    
    // ���ͼĴ�����ַ
    I2C_SendData(I2C1, regAddr);
    if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) return false;
    
    // ��������
    I2C_SendData(I2C1, data);
    if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) return false;
    
    // ����ֹͣ����
    I2C_GenerateSTOP(I2C1, ENABLE);
    
    return true;
}

bool I2C_ReadBytes(uint8_t devAddr, uint8_t regAddr, uint8_t *buf, uint16_t len) {
    if (len == 0) return true;
    
    // �ȴ����߿���
    uint32_t timeout = I2C_TIMEOUT;
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)) {
        if (timeout-- == 0) return false;
    }
    
    // ������ʼ����
    I2C_GenerateSTART(I2C1, ENABLE);
    if (!I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) return false;
    
    // �����豸��ַ(дģʽ)
    I2C_Send7bitAddress(I2C1, devAddr << 1, I2C_Direction_Transmitter);
    if (!I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) return false;
    
    // ����Ҫ��ȡ�ļĴ�����ַ
    I2C_SendData(I2C1, regAddr);
    if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) return false;
    
    // �����ظ���ʼ����
    I2C_GenerateSTART(I2C1, ENABLE);
    if (!I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) return false;
    
    // �����豸��ַ(��ģʽ)
    I2C_Send7bitAddress(I2C1, devAddr << 1, I2C_Direction_Receiver);
    if (!I2C_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) return false;
    
    // ��ȡ����
    for (uint16_t i = 0; i < len; i++) {
        if (i == len - 1) {
            // ���һ���ֽڣ�����NACK��ֹͣ����
            I2C_AcknowledgeConfig(I2C1, DISABLE);
            if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED)) return false;
            buf[i] = I2C_ReceiveData(I2C1);
            I2C_GenerateSTOP(I2C1, ENABLE);
        } else {
            if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED)) return false;
            buf[i] = I2C_ReceiveData(I2C1);
        }
    }
    
    // ��������ACK
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    
    return true;
}

bool MPU6050_Init(uint8_t addr) {
    I2C1_Init();
    
    // ����оƬ (PWR_MGMT_1 = 0)
	if (!I2C_WriteByte(addr, 0x6B, 0x00)) return false;
    
    // ���ò�����
    if (!I2C_WriteByte(addr, 0x19, 0x07)) return false;
    
    // ���������� ��2000dps
    if (!I2C_WriteByte(addr, 0x1B, 0x18)) return false;
    
    // ���ٶȼ����� ��2g
    if (!I2C_WriteByte(addr, 0x1C, 0x00)) return false;
    
    return true;
}
bool MPU6050_ReadAccel(uint8_t addr, mpu6050_accel_t *accel) {
    uint8_t buf[6];
//	swi2c_read(addr, MPU6050_ACCEL_XOUT_H, buf, 6);
	I2C_ReadBytes(addr, MPU6050_ACCEL_XOUT_H, buf, 6);

    accel->x_raw = (int16_t)((buf[0] << 8) | buf[1]);
    accel->y_raw = (int16_t)((buf[2] << 8) | buf[3]);
    accel->z_raw = (int16_t)((buf[4] << 8) | buf[5]);

    accel->x = round2((float)accel->x_raw / MPU6050_ACCEL_SENS) - accel_bias_x;
    accel->y = round2((float)accel->y_raw / MPU6050_ACCEL_SENS) - accel_bias_y;
    accel->z = round2((float)accel->z_raw / MPU6050_ACCEL_SENS) - accel_bias_z;
 
	return true;
}
//bool mpu6050_init(void)
//{
////    swi2c_init();
////	usart_Init();

//	// ����豸ID
//    uint8_t whoami = 0xFF;
//    swi2c_read(MPU6050_ADDR, WHO_AM_I_REG, &whoami, 1);
//	
//	swi2c_read(MPU6050_ADDR, WHO_AM_I_REG, &whoami, 1);
//	
//    if (whoami != 0x68)
//    {
//        return false;
//    }

//    uint8_t data;

//    // reset the device
//    data = 0;
//    swi2c_write(MPU6050_ADDR, PWR_MGMT_1_REG, &data, 1);

//    Delay(100);

//    // set the sample rate to 1kHz
//    data = 0x07;
//    swi2c_write(MPU6050_ADDR, SMPLRT_DIV_REG, &data, 1);

//    // set the accelerometer configuration to 2g
//    data = 0x00;
//    swi2c_write(MPU6050_ADDR, ACCEL_CONFIG_REG, &data, 1);

//    // set the gyro configuration to 250deg/s
//    data = 0x00;
//    swi2c_write(MPU6050_ADDR, GYRO_CONFIG_REG, &data, 1);

//    return true;
//}


//void mpu6050_read_accel(mpu6050_accel_t *accel)
//{


//    uint8_t raw_data[6];
//    swi2c_read(MPU6050_ADDR, ACCEL_XOUT_H_REG, raw_data, 6);


//    // ת��ԭʼ���ݣ�ע����źͱ�����
//    accel->x_raw = (int16_t)(raw_data[0] << 8 | raw_data[1]);
//    accel->y_raw = (int16_t)(raw_data[2] << 8 | raw_data[3]);
//    accel->z_raw = (int16_t)(raw_data[4] << 8 | raw_data[5]);

//    // ת��Ϊgֵ��У׼��ƫ��ȡ��ע�ͣ�
//    accel->x = (accel->x_raw / 16384.0f) - accel_bias_x;  // 16384 for ��2g range
//    accel->y = (accel->y_raw / 16384.0f) - accel_bias_y;
//    accel->z = (accel->z_raw / 16384.0f) - accel_bias_z;

//    // �Լ��ٶȼƵ�����ͨ�˲�
//    filtered_accel_x = alpha * accel->x + (1 - alpha) * filtered_accel_x;
//    filtered_accel_y = alpha * accel->y + (1 - alpha) * filtered_accel_y;
//    filtered_accel_z = alpha * accel->z + (1 - alpha) * filtered_accel_z;

//    // �������
//    accel->x = filtered_accel_x;
//    accel->y = filtered_accel_y;
//    accel->z = filtered_accel_z;
//}

//	
//void mpu6050_read_gyro(mpu6050_gyro_t *gyro) {
//    uint8_t raw_data[6];
//    
//    // 1. ��ȡԭʼ����
//    swi2c_read(MPU6050_ADDR, GYRO_XOUT_H_REG, raw_data, 6);
//    gyro->x_raw = (int16_t)(raw_data[0] << 8 | raw_data[1]);
//    gyro->y_raw = (int16_t)(raw_data[2] << 8 | raw_data[3]);
//    gyro->z_raw = (int16_t)(raw_data[4] << 8 | raw_data[5]);

//    // 2. ת��Ϊ��/s��У׼��ƫ (��250dps��Χ: 131.0 LSB/(��/s))
//    gyro->x = (gyro->x_raw / 131.0f) - gyro_bias_x;
//    gyro->y = (gyro->y_raw / 131.0f) - gyro_bias_y;
//    gyro->z = (gyro->z_raw / 131.0f) - gyro_bias_z;

//    // 3. Ӧ�õ�ͨ�˲�
//    filtered_gyro_x = alpha * gyro->x + (1 - alpha) * filtered_gyro_x;
//    filtered_gyro_y = alpha * gyro->y + (1 - alpha) * filtered_gyro_y;
//    filtered_gyro_z = alpha * gyro->z + (1 - alpha) * filtered_gyro_z;

//    // 4. ������
//    gyro->x = filtered_gyro_x;
//    gyro->y = filtered_gyro_y;
//    gyro->z = filtered_gyro_z;
//}

//// �Զ�У׼�������ھ�ֹʱ���ã�
//void calibrate_mpu6050() {
//    int32_t sum_x = 0, sum_y = 0, sum_z = 0;
//    const int samples = 100;
//    
//    for(int i = 0; i < samples; i++) {
//        mpu6050_accel_t accel;
//        mpu6050_read_accel(&accel);
//        sum_x += accel.x;
//        sum_y += accel.y;
//        sum_z += accel.z;

//    }
//    
//    // ����У׼ƫ����
//    accel_bias_z = (sum_z / samples) - GRAVITY_OFFSET;
//    
//}
//float mpu6050_read_temper(void)
//{
//    uint8_t raw_data[2];

//    swi2c_read(MPU6050_ADDR, TEMP_OUT_H_REG, raw_data, 2);

//    int16_t temp_raw = (int16_t)(raw_data[0] << 8 | raw_data[1]);

//    return (float)temp_raw / 340.0f + 36.53f;
//}

