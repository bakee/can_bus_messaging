#include <stdio.h>
#include <freertos/FreeRTOS.h>

#include "dht.h"
#include "common.h"
#include "can_bus.h"
#include "temp_humd.h"

#define DHT_PIN GPIO_NUM_5

static void sendOverCan(int16_t temp, int16_t humd)
{
    temp_humd_data_t data = {
        .temp = temp,
        .humd = humd};
    can_send_temp_humd_data(data);
}

void temp_humd_task()
{
    int16_t temp;
    int16_t humd;
    for (;;)
    {
        delay_ms(5000);
        dht_read_data(DHT_TYPE_DHT11, DHT_PIN, &humd, &temp);
        sendOverCan(temp, humd);
        printf("Temp: %d, Hum: %d\n", temp, humd);
    }
}