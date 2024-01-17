#include <stdio.h>
#include <freertos/FreeRTOS.h>

#include "dht.h"
#include "common.h"
#include "can_bus.h"
#include "temp_humd.h"

static void sendOverCan(int16_t temp, int16_t humd)
{
    temp_humd_data_t data = {
        .temp = temp,
        .humd = humd};
    can_send_temp_humd_data(data);
}

void temp_humd_task()
{
    const int tempPin = 5;
    int counter = 1;
    for (;;)
    {
        delay_ms(5000);
        int16_t temp = -1;
        int16_t hum = -1;
        dht_read_data(DHT_TYPE_DHT11, tempPin, &hum, &temp);
        sendOverCan(temp, hum);
        printf("%d -> Temp: %d, Hum: %d\n", counter++, temp, hum);
    }
}