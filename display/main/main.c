#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "led.h"
#include "can_bus.h"
#include "display.h"

void app_main(void)
{
    xTaskCreate(led_blink_task, "Blink", 5000, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(can_recv_task, "CAN_RECV", 5000, NULL, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(display_task, "Display", 5000, NULL, tskIDLE_PRIORITY + 3, NULL);
}