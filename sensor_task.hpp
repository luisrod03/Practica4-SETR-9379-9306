#ifndef SENSOR_TASK_HPP
#define SENSOR_TASK_HPP

struct SensorTaskConfig_t {
    const char *nombre;
    int periodo_ms;
    int ventana_mediana;
};

void SensorTask(void *pvParameters);

#endif
