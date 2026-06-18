#include "button_task.hpp"
#include "app_context.hpp"
#include "app_config.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "BUTTON";

void ButtonTask(void *pvParameters) {
    const ButtonTaskConfig_t *cfg = static_cast<const ButtonTaskConfig_t *>(pvParameters);

    gpio_config_t conf_gpio = {};
    conf_gpio.pin_bit_mask = (1ULL << cfg->pin);
    conf_gpio.mode         = GPIO_MODE_INPUT;
    conf_gpio.pull_up_en   = GPIO_PULLUP_ENABLE;
    conf_gpio.pull_down_en = GPIO_PULLDOWN_DISABLE;
    conf_gpio.intr_type    = GPIO_INTR_DISABLE;
    gpio_config(&conf_gpio);

    int estado_previo = 1;

    ESP_LOGI(TAG, "[%s] iniciado en GPIO%d", cfg->nombre, cfg->pin);

    while (true) {
        int estado_actual = gpio_get_level(cfg->pin);

        if (estado_previo == 1 && estado_actual == 0) {
            ButtonMsg_t msg = {cfg->id, AccionBoton::PRESIONADO};
            xQueueSend(g_queues.cola_boton, &msg, 0);
            ESP_LOGI(TAG, "[%s] PRESIONADO", cfg->nombre);
            vTaskDelay(pdMS_TO_TICKS(BUTTON_DEBOUNCE_MS));
        }

        if (cfg->reportar_liberacion && estado_previo == 0 && estado_actual == 1) {
            ButtonMsg_t msg = {cfg->id, AccionBoton::LIBERADO};
            xQueueSend(g_queues.cola_boton, &msg, 0);
            ESP_LOGI(TAG, "[%s] LIBERADO", cfg->nombre);
            vTaskDelay(pdMS_TO_TICKS(BUTTON_DEBOUNCE_MS));
        }

        estado_previo = estado_actual;
        vTaskDelay(pdMS_TO_TICKS(BUTTON_POLL_MS));
    }
}
