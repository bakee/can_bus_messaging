#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>

#include "common.h"
#include "led.h"

#define HIGH 1
#define LOW 0

#define LED_PIN GPIO_NUM_2

void led_blink_task()
{
    gpio_set_direction(LED_PIN, GPIO_MODE_DEF_OUTPUT);
    for (;;)
    {
        int random_amount = 50 + rand() % 200;
        gpio_set_level(LED_PIN, HIGH);
        delay_ms(random_amount);
        gpio_set_level(LED_PIN, LOW);
        delay_ms(random_amount);
    }
}