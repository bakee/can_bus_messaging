#include <stdio.h>
#include <driver/twai.h>

#include "common.h"
#include "display.h"
#include "can_bus.h"

#define CAN_BUS_TX_PIN GPIO_NUM_33
#define CAN_BUS_RX_PIN GPIO_NUM_32

#define MAKE_WORD(byte0, byte1) ((byte0) | ((byte1) << 8))

static void handle_temperature_data(uint8_t *data)
{
    set_temperature_data(MAKE_WORD(data[0], data[1]));
}

static void handle_humidity_data(uint8_t *data)
{
    set_humidity_data(MAKE_WORD(data[0], data[1]));
}

static void handle_accelaration_data(uint8_t *data)
{
    set_acce_data(MAKE_WORD(data[0], data[1]),
                  MAKE_WORD(data[2], data[3]),
                  MAKE_WORD(data[4], data[5]));
}

static void handle_gyroscope_data(uint8_t *data)
{
    set_gyro_data(MAKE_WORD(data[0], data[1]),
                  MAKE_WORD(data[2], data[3]),
                  MAKE_WORD(data[4], data[5]));
}

void log_message(twai_message_t *message)
{
    printf("Received : %d bytes from 0x%X: ", message->data_length_code, (int)message->identifier);
    for (int i = 0; i < message->data_length_code && i < 8; i++)
    {
        printf("0x%02X ", message->data[i]);
    }
    printf("\n");
}

static void handle_message(twai_message_t *message)
{
    log_message(message);
    int message_id = message->identifier;
    switch (message_id)
    {
    case CAN_ID_TEMPERATURE:
        handle_temperature_data(message->data);
        break;
    case CAN_ID_HUMIDITY:
        handle_humidity_data(message->data);
        break;
    case CAN_ID_ACCELEROMETER:
        handle_accelaration_data(message->data);
        break;
    case CAN_ID_GYROSCOPE:
        handle_gyroscope_data(message->data);
        break;
    default:
        break;
    }
}

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

static void handle_receive_error(const esp_err_t status)
{
    switch (status)
    {
    case ESP_ERR_TIMEOUT:
        printf("Timed out waiting for message\n");
        break;

    case ESP_ERR_INVALID_ARG:
        printf("Arguments are invalid\n");
        break;

    case ESP_ERR_INVALID_STATE:
        printf("TWAI driver is not installed\n");
        restart_can_bus();
        break;

    default:
        printf("TWAI UNKNOWN\n");
        break;
    }
}

void can_recv_task()
{
    setup_twai();

    for (;;)
    {
        twai_message_t message;
        esp_err_t status = twai_receive(&message, pdMS_TO_TICKS(5000));
        if (ESP_OK == status)
        {
            handle_message(&message);
        }
        else
        {
            handle_receive_error(status);
        }
    }
}
