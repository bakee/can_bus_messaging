#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "led.h"
#include "acce_gyro.h"
#include "can_bus.h"

void app_main(void)
{
    xTaskCreate(led_blink_task, "Blink", 5000, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(acce_gyro_task, "Acc_Gyro", 5000, NULL, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(can_send_task, "CAN_Tx", 5000, NULL, tskIDLE_PRIORITY + 2, NULL);
}