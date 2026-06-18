#ifndef APP_CONTEXT_HPP
#define APP_CONTEXT_HPP

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

struct AppQueues_t {
    QueueHandle_t cola_boton;
    QueueHandle_t cola_sensor;
    QueueHandle_t cola_cmd_servo;
    QueueHandle_t cola_estado_servo;
};

struct AppHandles_t {
    TaskHandle_t h_sensor;
    TaskHandle_t h_servo;
    TaskHandle_t h_boton_inicio;
    TaskHandle_t h_boton_vel;
    TaskHandle_t h_led_listo;
    TaskHandle_t h_gestor;
};

extern AppQueues_t g_queues;
extern AppHandles_t g_handles;

void app_context_init(void);

#endif