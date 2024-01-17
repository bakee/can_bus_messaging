
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <driver/twai.h>

#include "common.h"
#include "can_bus.h"

#define CAN_ID_ACCELEROMETER 0x0B1
#define CAN_ID_GYROSCOPE 0x0B2

#define CAN_BUS_TX_PIN GPIO_NUM_33
#define CAN_BUS_RX_PIN GPIO_NUM_32

#define LSB(x) ((x) & 0xFF)
#define MSB(x) (((x) >> 8) & 0xFF)

QueueHandle_t acce_gyro_tx_queue_handle = NULL;
static twai_message_t acce_message;
static twai_message_t gyro_message;

static void setup_twai()
{
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_BUS_TX_PIN, CAN_BUS_RX_PIN, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    ESP_ERROR_CHECK(twai_start());
}

static void stop_twai()
{
    ESP_ERROR_CHECK(twai_stop());
    ESP_ERROR_CHECK(twai_driver_uninstall());
}

static void restart_can_bus()
{
    printf("Restarting CAN bus in 2 seconds...\n");
    delay_ms(2000);
    stop_twai();
    setup_twai();
    printf("CAN bus restarted\n");
}

static void setup_queue()
{
    acce_gyro_tx_queue_handle = xQueueCreate(5, sizeof(acce_gyro_data_t));
}

static void setup_message()
{
    acce_message.identifier = CAN_ID_ACCELEROMETER;
    acce_message.extd = 0;
    acce_message.data_length_code = 6;

    gyro_message.identifier = CAN_ID_GYROSCOPE;
    gyro_message.extd = 0;
    gyro_message.data_length_code = 6;
}

static void put_message_payload(acce_gyro_data_t *data)
{
    acce_message.data[0] = LSB(data->acce_x);
    acce_message.data[1] = MSB(data->acce_x);

    acce_message.data[2] = LSB(data->acce_y);
    acce_message.data[3] = MSB(data->acce_y);

    acce_message.data[4] = LSB(data->acce_z);
    acce_message.data[5] = MSB(data->acce_z);

    gyro_message.data[0] = LSB(data->gyro_x);
    gyro_message.data[1] = MSB(data->gyro_x);

    gyro_message.data[2] = LSB(data->gyro_y);
    gyro_message.data[3] = MSB(data->gyro_y);

    gyro_message.data[4] = LSB(data->gyro_z);
    gyro_message.data[5] = MSB(data->gyro_z);
}

static void handle_transmit_error(const esp_err_t status)
{
    switch (status)
    {
    case ESP_ERR_INVALID_ARG:
        printf("Arguments are invalid\n");
        break;
    case ESP_ERR_TIMEOUT:
        printf("Timed out waiting for space on TX queue\n");
        break;
    case ESP_FAIL:
        printf("TX queue is disabled and another message is currently transmitting\n");
        break;
    case ESP_ERR_INVALID_STATE:
        printf("TWAI driver is not in running state, or is not installed\n");
        restart_can_bus();
        break;
    case ESP_ERR_NOT_SUPPORTED:
        printf("Listen Only Mode does not support transmissions\n");
        break;
    default:
        printf("UNKNOWN\n");
        break;
    }
}

void can_send_task()
{
    setup_queue();
    setup_twai();
    setup_message();

    for (;;)
    {
        acce_gyro_data_t data;
        if (pdTRUE == xQueueReceive(acce_gyro_tx_queue_handle, &data, pdMS_TO_TICKS(1000)))
        {
            put_message_payload(&data);
            esp_err_t transmit_status = twai_transmit(&acce_message, pdMS_TO_TICKS(100));

            if (ESP_OK != transmit_status)
            {
                handle_transmit_error(transmit_status);
            }

            transmit_status = twai_transmit(&gyro_message, pdMS_TO_TICKS(100));

            if (ESP_OK != transmit_status)
            {
                handle_transmit_error(transmit_status);
            }
        }
    }
}

void can_send_acce_gyro_data(acce_gyro_data_t *data)
{
    xQueueSend(acce_gyro_tx_queue_handle, (void *)data, pdMS_TO_TICKS(100));
}