#include "servo_task.hpp"
#include "app_config.hpp"
#include "app_context.hpp"
#include "messages.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "SERVO";

static int angulo_a_ciclo_trabajo(int angulo) {
    if (angulo < SERVO_MIN_ANGLE) angulo = SERVO_MIN_ANGLE;
    if (angulo > SERVO_MAX_ANGLE) angulo = SERVO_MAX_ANGLE;

    int pulso_us = SERVO_MIN_US + ((SERVO_MAX_US - SERVO_MIN_US) * angulo) / 180;
    return (pulso_us * 65535) / 20000;
}

static void escribir_angulo(const ServoTaskConfig_t *cfg, int angulo) {
    ledc_set_duty(cfg->modo, cfg->canal, angulo_a_ciclo_trabajo(angulo));
    ledc_update_duty(cfg->modo, cfg->canal);
}

void ServoTask(void *pvParameters) {
    const ServoTaskConfig_t *cfg = static_cast<const ServoTaskConfig_t *>(pvParameters);

    ledc_timer_config_t conf_timer = {};
    conf_timer.speed_mode      = cfg->modo;
    conf_timer.timer_num       = cfg->timer;
    conf_timer.duty_resolution = LEDC_TIMER_16_BIT;
    conf_timer.freq_hz         = SERVO_FREQ_HZ;
    conf_timer.clk_cfg         = LEDC_AUTO_CLK;
    ledc_timer_config(&conf_timer);

    ledc_channel_config_t conf_canal = {};
    conf_canal.gpio_num   = cfg->pin;
    conf_canal.speed_mode = cfg->modo;
    conf_canal.channel    = cfg->canal;
    conf_canal.intr_type  = LEDC_INTR_DISABLE;
    conf_canal.timer_sel  = cfg->timer;
    conf_canal.duty       = 0;
    conf_canal.hpoint     = 0;
    ledc_channel_config(&conf_canal);

    int  angulo_actual  = 0;
    int  angulo_obj     = 0;
    bool modo_rapido    = false;

    escribir_angulo(cfg, angulo_actual);

    ESP_LOGI(TAG, "[%s] iniciado", cfg->nombre);

    while (true) {
        ServoCmd_t cmd = {};

        if (xQueueReceive(g_queues.cola_cmd_servo, &cmd, portMAX_DELAY) == pdTRUE) {
            angulo_obj  = cmd.angulo_objetivo;
            modo_rapido = cmd.modo_rapido;

            ESP_LOGI(TAG, "objetivo=%d modo=%s", angulo_obj, modo_rapido ? "RAPIDO" : "LENTO");

            while (angulo_actual != angulo_obj) {
                ServoCmd_t cmd_nuevo = {};
                if (xQueueReceive(g_queues.cola_cmd_servo, &cmd_nuevo, 0) == pdTRUE) {
                    angulo_obj  = cmd_nuevo.angulo_objetivo;
                    modo_rapido = cmd_nuevo.modo_rapido;
                    ESP_LOGI(TAG, "actualizado objetivo=%d modo=%s", angulo_obj, modo_rapido ? "RAPIDO" : "LENTO");
                }

                if (angulo_actual < angulo_obj)      angulo_actual++;
                else if (angulo_actual > angulo_obj) angulo_actual--;

                escribir_angulo(cfg, angulo_actual);

                ServoStatusMsg_t estado = {};
                estado.angulo_actual   = angulo_actual;
                estado.angulo_objetivo = angulo_obj;
                estado.llegado         = false;
                xQueueSend(g_queues.cola_estado_servo, &estado, 0);

                vTaskDelay(pdMS_TO_TICKS(modo_rapido ? SERVO_FAST_STEP_MS : SERVO_SLOW_STEP_MS));
            }

            ServoStatusMsg_t estado = {};
            estado.angulo_actual   = angulo_actual;
            estado.angulo_objetivo = angulo_obj;
            estado.llegado         = true;
            xQueueSend(g_queues.cola_estado_servo, &estado, 0);

            ESP_LOGI(TAG, "llegado a angulo=%d", angulo_actual);
        }
    }
}
