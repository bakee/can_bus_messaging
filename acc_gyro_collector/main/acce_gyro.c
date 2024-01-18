#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <driver/i2c.h>

#include "mpu6050.h"

#include "common.h"
#include "can_bus.h"
#include "acce_gyro.h"

#define MPU_6050_ADDRESS 0x68
#define I2C_SDA_PIN GPIO_NUM_23
#define I2C_SCL_PIN GPIO_NUM_22
#define I2C_FREQUENCY 100000
#define I2C_PORT_NO I2C_NUM_0

static esp_err_t i2c_master_driver_initialize(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_SCL_PIN,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FREQUENCY,
    };
    return i2c_param_config(I2C_PORT_NO, &conf);
}

static void send_over_can_bus(const mpu6050_raw_acce_value_t *acce_value, const mpu6050_raw_gyro_value_t *gyro_value)
{
    acce_gyro_data_t data = {
        .acce_x = acce_value->raw_acce_x,
        .acce_y = acce_value->raw_acce_y,
        .acce_z = acce_value->raw_acce_z,
        .gyro_x = gyro_value->raw_gyro_x,
        .gyro_y = gyro_value->raw_gyro_y,
        .gyro_z = gyro_value->raw_gyro_z};
    can_send_acce_gyro_data(&data);
}

static void log_to_console(const mpu6050_raw_acce_value_t *acce_value, const mpu6050_raw_gyro_value_t *gyro_value)
{
    printf("Raw Acce: x:%d, y:%d, z:%d \tRaw Gyro: x:%d, y:%d, z:%d\n",
           acce_value->raw_acce_x,
           acce_value->raw_acce_y,
           acce_value->raw_acce_z,
           gyro_value->raw_gyro_x,
           gyro_value->raw_gyro_y,
           gyro_value->raw_gyro_z);
}

void acce_gyro_task()
{
    delay_ms(2000);
    mpu6050_handle_t mpu6050;

    i2c_driver_install(I2C_PORT_NO, I2C_MODE_MASTER, 0, 0, 0);
    i2c_master_driver_initialize();

    mpu6050 = mpu6050_create(I2C_PORT_NO, MPU_6050_ADDRESS);
    mpu6050_config(mpu6050, ACCE_FS_4G, GYRO_FS_500DPS);
    mpu6050_wake_up(mpu6050);

    mpu6050_raw_acce_value_t raw_acce_value;
    mpu6050_raw_gyro_value_t raw_gyro_value;
    for (;;)
    {
        delay_ms(5000);
        mpu6050_get_raw_acce(mpu6050, &raw_acce_value);
        mpu6050_get_raw_gyro(mpu6050, &raw_gyro_value);
        send_over_can_bus(&raw_acce_value, &raw_gyro_value);
        log_to_console(&raw_acce_value, &raw_gyro_value);
    }
}