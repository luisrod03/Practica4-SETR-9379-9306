#include "sensor_task.hpp"
#include "app_config.hpp"
#include "app_context.hpp"
#include "messages.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

static const char *TAG = "SENSOR";
static adc_oneshot_unit_handle_t manejador_adc;

static void ordenar_burbuja(int *datos, int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (datos[j] < datos[i]) {
                int tmp   = datos[i];
                datos[i]  = datos[j];
                datos[j]  = tmp;
            }
        }
    }
}

static int calcular_mediana(int *datos, int n) {
    ordenar_burbuja(datos, n);
    return datos[n / 2];
}

static int angulo_desde_luz(int lectura) {
    return (lectura >= LIGHT_THRESHOLD) ? 180 : 0;
}

void SensorTask(void *pvParameters) {
    const SensorTaskConfig_t *cfg = static_cast<const SensorTaskConfig_t *>(pvParameters);

    adc_oneshot_unit_init_cfg_t conf_unidad = {};
    conf_unidad.unit_id  = LDR_ADC_UNIT;
    conf_unidad.ulp_mode = ADC_ULP_MODE_DISABLE;
    adc_oneshot_new_unit(&conf_unidad, &manejador_adc);

    adc_oneshot_chan_cfg_t conf_canal = {};
    conf_canal.atten    = ADC_ATTEN_DB_12;
    conf_canal.bitwidth = ADC_BITWIDTH_12;
    adc_oneshot_config_channel(manejador_adc, LDR_ADC_CHANNEL, &conf_canal);

    ESP_LOGI(TAG, "[%s] iniciado", cfg->nombre);

    while (true) {
        int muestras[15] = {};
        int ventana = cfg->ventana_mediana;

        if (ventana > 15) ventana = 15;
        if (ventana < 3)  ventana = 3;

        for (int i = 0; i < ventana; i++) {
            adc_oneshot_read(manejador_adc, LDR_ADC_CHANNEL, &muestras[i]);
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        int filtrado = calcular_mediana(muestras, ventana);
        int objetivo = angulo_desde_luz(filtrado);

        SensorMsg_t msg = {};
        msg.crudo           = muestras[ventana / 2];
        msg.filtrado        = filtrado;
        msg.angulo_objetivo = objetivo;

        xQueueSend(g_queues.cola_sensor, &msg, 0);

        ESP_LOGI(TAG, "crudo=%d filtrado=%d objetivo=%d", msg.crudo, msg.filtrado, msg.angulo_objetivo);

        vTaskDelay(pdMS_TO_TICKS(cfg->periodo_ms));
    }
}
