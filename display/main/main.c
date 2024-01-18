#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "led.h"
#include "can_bus.h"
#include "display.h"

void app_main(void)
{
    xTaskCreate(led_blink_task, "Blink", 1700, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(can_recv_task, "CAN_RECV", 2200, NULL, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(display_task, "Display", 1900, NULL, tskIDLE_PRIORITY + 3, NULL);
}