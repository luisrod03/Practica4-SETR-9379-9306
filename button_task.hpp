#ifndef BUTTON_TASK_HPP
#define BUTTON_TASK_HPP

#include "driver/gpio.h"
#include "messages.hpp"

struct ButtonTaskConfig_t {
    const char *nombre;
    gpio_num_t  pin;
    IdBoton     id;
    bool        reportar_liberacion;
};

void ButtonTask(void *pvParameters);

#endif
