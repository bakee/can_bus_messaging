#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "led.h"
#include "temp_humd.h"
#include "can_bus.h"

void app_main(void)
{
    xTaskCreate(led_blink_task, "Blink", 1700, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(can_tx_task, "Temp", 2100, NULL, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(temp_humd_task, "Temp_Humd", 1700, NULL, tskIDLE_PRIORITY + 2, NULL);
}
