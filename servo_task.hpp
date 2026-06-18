#ifndef SERVO_TASK_HPP
#define SERVO_TASK_HPP

#include "driver/gpio.h"
#include "driver/ledc.h"

struct ServoTaskConfig_t {
    const char     *nombre;
    gpio_num_t      pin;
    ledc_channel_t  canal;
    ledc_timer_t    timer;
    ledc_mode_t     modo;
};

void ServoTask(void *pvParameters);

#endif
