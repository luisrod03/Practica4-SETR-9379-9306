#include "ready_led_task.hpp"
#include "app_config.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "READY_LED";
static gpio_num_t pin_led = PIN_READY_LED;

void ready_led_force_off(void) {
    gpio_set_level(pin_led, 0);
}

void ReadyLedTask(void *pvParameters) {
    const ReadyLedConfig_t *cfg = static_cast<const ReadyLedConfig_t *>(pvParameters);
    pin_led = cfg->pin;

    gpio_config_t conf_gpio = {};
    conf_gpio.pin_bit_mask = (1ULL << cfg->pin);
    conf_gpio.mode         = GPIO_MODE_OUTPUT;
    conf_gpio.pull_up_en   = GPIO_PULLUP_DISABLE;
    conf_gpio.pull_down_en = GPIO_PULLDOWN_DISABLE;
    conf_gpio.intr_type    = GPIO_INTR_DISABLE;
    gpio_config(&conf_gpio);

    ESP_LOGI(TAG, "[%s] iniciado", cfg->nombre);

    while (true) {
        gpio_set_level(cfg->pin, 1);
        vTaskDelay(pdMS_TO_TICKS(READY_LED_ON_MS));

        gpio_set_level(cfg->pin, 0);
        vTaskDelay(pdMS_TO_TICKS(READY_LED_OFF_MS));
    }
}
