#include <stdio.h>
#include <freeRTOS/freeRTOS.h>
#include <freertos/queue.h>
#include <driver/i2c.h>

#include "ssd1306.h"
#include "font8x8_basic.h"

#include "common.h"
#include "display.h"

QueueHandle_t display_queue;

#define I2C_SDA_PIN GPIO_NUM_23
#define I2C_SCL_PIN GPIO_NUM_22

static SSD1306_t dev;

static display_data_t display_data[4];
static display_data_type_t current_display = TEMPERATURE;

static void show_start_screen()
{
    const char *welcome = "Welcome";
    const char *dots = ".....";
    for (int i = 0; i < 5; i++)
    {
        ssd1306_display_text_x3(&dev, 0, welcome + (i < 2 ? i : 2), 5, false);
        ssd1306_display_text_x3(&dev, 4, dots, i + 1, false);
        delay_ms(500);
    }
    ssd1306_clear_screen(&dev, false);
}

static void initialize_display()
{
    i2c_master_init(&dev, I2C_SDA_PIN, I2C_SCL_PIN, -1);
    ssd1306_init(&dev, 128, 64);
    ssd1306_contrast(&dev, 0xff);
    ssd1306_clear_screen(&dev, false);
    show_start_screen();
}

static void diplay_temperature()
{
    ssd1306_clear_screen(&dev, false);
    ssd1306_display_text_x3(&dev, 0, "TEMPR", 5, false);
    char str[16] = {0};
    display_data_t data = display_data[TEMPERATURE];
    sprintf(str, "%d.%d C", data.words[0] / 10, data.words[0] % 10);
    ssd1306_display_text(&dev, 5, str, 16, false);
}

static void display_humidity()
{
    ssd1306_clear_screen(&dev, false);
    ssd1306_display_text_x3(&dev, 0, "HUMDT", 5, false);
    char str[16] = {0};
    display_data_t data = display_data[HUMIDTY];
    sprintf(str, "%d.%d%%", data.words[0] / 10, data.words[0] % 10);
    ssd1306_display_text(&dev, 5, str, 16, false);
}

static void display_acce()
{
    ssd1306_clear_screen(&dev, false);
    ssd1306_display_text_x3(&dev, 0, "ACCLM", 5, false);
    char line1[16] = {0};
    display_data_t data = display_data[ACCELERAROMETER];
    sprintf(line1, "X = %d", data.words[0]);
    ssd1306_display_text(&dev, 5, line1, 16, false);
    char line2[16] = {0};
    sprintf(line2, "Y = %d", data.words[1]);
    ssd1306_display_text(&dev, 6, line2, 16, false);
    char line3[16] = {0};
    sprintf(line3, "Z = %d", data.words[2]);
    ssd1306_display_text(&dev, 7, line3, 16, false);
}

static void display_gyro()
{
    ssd1306_clear_screen(&dev, false);
    ssd1306_display_text_x3(&dev, 0, "GYROS", 5, false);
    char line1[16] = {0};
    display_data_t data = display_data[GYROSCOPE];
    sprintf(line1, "X = %d", data.words[0]);
    ssd1306_display_text(&dev, 5, line1, 16, false);
    char line2[16] = {0};
    sprintf(line2, "Y = %d", data.words[1]);
    ssd1306_display_text(&dev, 6, line2, 16, false);
    char line3[16] = {0};
    sprintf(line3, "Z = %d", data.words[2]);
    ssd1306_display_text(&dev, 7, line3, 16, false);
}

static void fetch_data()
{
    display_data_t data;
    if (xQueueReceive(display_queue, &data, pdMS_TO_TICKS(10)) == pdTRUE)
    {
        display_data[data.type] = data;
    }
}

static void show_next_display()
{
    switch (current_display)
    {
    case TEMPERATURE:
        diplay_temperature();
        current_display = HUMIDTY;
        break;
    case HUMIDTY:
        display_humidity();
        current_display = ACCELERAROMETER;
        break;
    case ACCELERAROMETER:
        display_acce();
        current_display = GYROSCOPE;
        break;
    case GYROSCOPE:
        display_gyro();
        current_display = TEMPERATURE;
        break;
    default:
        current_display = TEMPERATURE;
        break;
    }
}

void display_task()
{
    display_queue = xQueueCreate(10, sizeof(display_data_t));
    delay_ms(500);
    initialize_display();
    int fetch_count = 0;
    while (1)
    {
        delay_ms(100);
        fetch_data();
        fetch_count++;
        if (fetch_count == 20)
        {
            fetch_count = 0;
            show_next_display();
        }
    }
}

void set_temperature_data(int16_t temp)
{
    display_data_t display_data = {
        .type = TEMPERATURE,
        .words = {temp}};
    xQueueSend(display_queue, &display_data, pdMS_TO_TICKS(10));
}

void set_humidity_data(int16_t humd)
{
    display_data_t display_data = {
        .type = HUMIDTY,
        .words = {humd}};
    xQueueSend(display_queue, &display_data, pdMS_TO_TICKS(10));
}

void set_acce_data(int16_t x, int16_t y, int16_t z)
{
    display_data_t display_data = {
        .type = ACCELERAROMETER,
        .words = {x, y, z}};
    xQueueSend(display_queue, &display_data, pdMS_TO_TICKS(10));
}

void set_gyro_data(int16_t x, int16_t y, int16_t z)
{
    display_data_t display_data = {
        .type = GYROSCOPE,
        .words = {x, y, z}};
    xQueueSend(display_queue, &display_data, pdMS_TO_TICKS(10));
}