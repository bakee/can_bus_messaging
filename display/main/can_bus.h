#ifndef __CAN_BUS_H
#define __CAN_BUS_H

#define CAN_ID_TEMPERATURE 0x0A1
#define CAN_ID_HUMIDITY 0x0A2

#define CAN_ID_ACCELEROMETER 0x0B1
#define CAN_ID_GYROSCOPE 0x0B2

void can_recv_task();

#endif