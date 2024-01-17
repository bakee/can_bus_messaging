#ifndef __CAN_BUS_H_
#define __CAN_BUS_H_

#include <stdio.h>

typedef struct
{
    int16_t acce_x;
    int16_t acce_y;
    int16_t acce_z;

    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
} acce_gyro_data_t;

void can_send_task();
void can_send_acce_gyro_data(acce_gyro_data_t *data);

#endif