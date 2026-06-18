#include "task_manager.hpp"
#include "app_context.hpp"
#include "messages.hpp"
#include "ready_led_task.hpp"
#include "app_config.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "MANAGER";

enum class EstadoSistema {
    LISTO     = 0,
    OPERANDO  = 1,
    ESPERANDO = 2
};

static void vaciar_cola(QueueHandle_t cola) {
    if (cola == nullptr) return;
    uint8_t basura[32];
    while (xQueueReceive(cola, basura, 0) == pdTRUE) {}
}

void TaskManager(void *pvParameters) {
    const ManagerTaskConfig_t *cfg = static_cast<const ManagerTaskConfig_t *>(pvParameters);
    (void)cfg;

    EstadoSistema estado = EstadoSistema::LISTO;

    bool modo_rapido  = false;
    int  ultimo_obj   = 0;

    TickType_t inicio_espera = 0;
    TickType_t ultimo_log    = 0;

    vTaskSuspend(g_handles.h_sensor);
    vTaskSuspend(g_handles.h_servo);
    vTaskSuspend(g_handles.h_boton_vel);

    ESP_LOGI(TAG, "Sistema listo. Presiona INICIO");

    while (true) {
        ButtonMsg_t     msg_boton  = {};
        SensorMsg_t     msg_sensor = {};
        ServoStatusMsg_t est_servo  = {};

        if (xQueueReceive(g_queues.cola_boton, &msg_boton, pdMS_TO_TICKS(50)) == pdTRUE) {
            if (msg_boton.id == IdBoton::INICIO && msg_boton.accion == AccionBoton::PRESIONADO) {
                if (estado == EstadoSistema::LISTO) {
                    estado     = EstadoSistema::OPERANDO;
                    modo_rapido = false;
                    ultimo_obj  = 0;

                    vaciar_cola(g_queues.cola_sensor);
                    vaciar_cola(g_queues.cola_cmd_servo);
                    vaciar_cola(g_queues.cola_estado_servo);

                    ready_led_force_off();

                    vTaskSuspend(g_handles.h_led_listo);
                    vTaskSuspend(g_handles.h_boton_inicio);

                    vTaskResume(g_handles.h_sensor);
                    vTaskResume(g_handles.h_servo);
                    vTaskResume(g_handles.h_boton_vel);

                    ESP_LOGI(TAG, "Operacion iniciada");
                } else {
                    ESP_LOGW(TAG, "INICIO ignorado: operacion en curso");
                }
            }

            if (msg_boton.id == IdBoton::VELOCIDAD) {
                if (estado == EstadoSistema::OPERANDO) {
                    modo_rapido = (msg_boton.accion == AccionBoton::PRESIONADO);

                    ServoCmd_t cmd = {};
                    cmd.angulo_objetivo = ultimo_obj;
                    cmd.modo_rapido     = modo_rapido;
                    xQueueSend(g_queues.cola_cmd_servo, &cmd, 0);

                    ESP_LOGI(TAG, "Velocidad=%s", modo_rapido ? "RAPIDA" : "LENTA");
                } else {
                    ESP_LOGW(TAG, "Velocidad ignorada: sistema no operando");
                }
            }
        }

        if (estado == EstadoSistema::OPERANDO) {
            if (xQueueReceive(g_queues.cola_sensor, &msg_sensor, 0) == pdTRUE) {
                ultimo_obj = msg_sensor.angulo_objetivo;

                ServoCmd_t cmd = {};
                cmd.angulo_objetivo = ultimo_obj;
                cmd.modo_rapido     = modo_rapido;
                xQueueSend(g_queues.cola_cmd_servo, &cmd, 0);

                ESP_LOGI(TAG, "Sensor filtrado=%d objetivo=%d", msg_sensor.filtrado, msg_sensor.angulo_objetivo);
            }

            if (xQueueReceive(g_queues.cola_estado_servo, &est_servo, 0) == pdTRUE) {
                if (est_servo.llegado) {
                    estado        = EstadoSistema::ESPERANDO;
                    inicio_espera = xTaskGetTickCount();

                    vaciar_cola(g_queues.cola_sensor);
                    vaciar_cola(g_queues.cola_cmd_servo);
                    vaciar_cola(g_queues.cola_estado_servo);

                    ESP_LOGI(TAG, "Objetivo alcanzado. Esperando %d ms", HOLD_TARGET_MS);
                }
            }
        }

        if (estado == EstadoSistema::ESPERANDO) {
            TickType_t transcurrido = xTaskGetTickCount() - inicio_espera;

            if (transcurrido >= pdMS_TO_TICKS(HOLD_TARGET_MS)) {
                estado = EstadoSistema::LISTO;

                vaciar_cola(g_queues.cola_sensor);
                vaciar_cola(g_queues.cola_cmd_servo);
                vaciar_cola(g_queues.cola_estado_servo);
                vaciar_cola(g_queues.cola_boton);

                vTaskSuspend(g_handles.h_sensor);
                vTaskSuspend(g_handles.h_servo);
                vTaskSuspend(g_handles.h_boton_vel);

                vTaskResume(g_handles.h_boton_inicio);
                vTaskResume(g_handles.h_led_listo);

                ESP_LOGI(TAG, "Operacion terminada. Sistema listo");
            }
        }

        if ((xTaskGetTickCount() - ultimo_log) >= pdMS_TO_TICKS(3000)) {
            ultimo_log = xTaskGetTickCount();
            ESP_LOGI(TAG, "Estado=%d Sensor=%d Servo=%d BtnIni=%d BtnVel=%d",
                     static_cast<int>(estado),
                     static_cast<int>(eTaskGetState(g_handles.h_sensor)),
                     static_cast<int>(eTaskGetState(g_handles.h_servo)),
                     static_cast<int>(eTaskGetState(g_handles.h_boton_inicio)),
                     static_cast<int>(eTaskGetState(g_handles.h_boton_vel)));
        }
    }
}
