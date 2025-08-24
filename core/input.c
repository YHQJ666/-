#include "input.h"
#include "FreeRTOS.h"
#include "task.h"

#include "mpu6050.h"
#include "key.h"
#include "tank.h"        // ���Ʒ����ƶ�
#include "config.h"

void input_update_tank_rotation(uint8_t tank_id, float dt) {
    mpu6050_gyro_t gyro;
    mpu6050_read_gyro(&gyro);  // ���˲����

    float delta_angle = gyro.z * dt;  // z��Ϊƽ������ת

    float old_angle = tank_get_angle(tank_id);
    float new_angle = old_angle + delta_angle;

    tank_set_angle(tank_id, new_angle);  // �Զ� wrap �� 0~360
}

