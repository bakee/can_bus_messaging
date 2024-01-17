#ifndef __DISPLAY_H_
#define __DISPLAY_H_

typedef enum
{
    TEMPERATURE,
    HUMIDTY,
    ACCELERAROMETER,
    GYROSCOPE
} display_data_type_t;

typedef struct
{
    display_data_type_t type;
    int16_t words[3];
} display_data_t;

void display_task();
void set_temperature_data(int16_t temp);
void set_humidity_data(int16_t humd);
void set_acce_data(int16_t x, int16_t y, int16_t z);
void set_gyro_data(int16_t x, int16_t y, int16_t z);

#endif
