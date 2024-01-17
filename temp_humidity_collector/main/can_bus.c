
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <driver/twai.h>

#include "common.h"
#include "can_bus.h"

#define CAN_ID_TEMPERATURE 0x0A1
#define CAN_ID_HUMIDITY 0x0A2

#define CAN_BUS_TX_PIN GPIO_NUM_13
#define CAN_BUS_RX_PIN GPIO_NUM_14

QueueHandle_t temp_humd_queue_handle = NULL;
static twai_message_t temp_message;
static twai_message_t humd_message;

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
    temp_humd_queue_handle = xQueueCreate(10, sizeof(temp_humd_data_t));
}

static void setup_message()
{
    temp_message.identifier = CAN_ID_TEMPERATURE;
    temp_message.extd = 0;
    temp_message.data_length_code = 2;

    humd_message.identifier = CAN_ID_HUMIDITY;
    humd_message.extd = 0;
    humd_message.data_length_code = 2;
}

static void set_message_data(temp_humd_data_t *data)
{
    temp_message.data[0] = data->temp & 0xFF;
    temp_message.data[1] = (data->temp >> 8) & 0xFF;

    humd_message.data[0] = data->humd & 0xFF;
    humd_message.data[1] = (data->humd >> 8) & 0xFF;
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

void can_tx_task()
{
    setup_queue();
    setup_twai();
    setup_message();

    for (;;)
    {
        temp_humd_data_t data;
        if (pdTRUE == xQueueReceive(temp_humd_queue_handle, &data, pdMS_TO_TICKS(1000)))
        {
            set_message_data(&data);
            esp_err_t transmit_status = twai_transmit(&temp_message, pdMS_TO_TICKS(100));

            if (ESP_OK != transmit_status)
            {
                handle_transmit_error(transmit_status);
            }

            transmit_status = twai_transmit(&humd_message, pdMS_TO_TICKS(100));

            if (ESP_OK != transmit_status)
            {
                handle_transmit_error(transmit_status);
            }
        }
    }
}

void can_send_temp_humd_data(temp_humd_data_t data)
{
    xQueueSend(temp_humd_queue_handle, (void *)&data, pdMS_TO_TICKS(100));
}