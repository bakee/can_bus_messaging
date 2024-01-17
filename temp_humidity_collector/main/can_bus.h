#ifndef __CAN_BUS_H_
#define __CAN_BUS_H_

#include <stdio.h>

typedef struct temp_humd_data_t
{
    int16_t temp;
    int16_t humd;
} temp_humd_data_t;

void can_tx_task();
void can_send_temp_humd_data(temp_humd_data_t data);

#endif