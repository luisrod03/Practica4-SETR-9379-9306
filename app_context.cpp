#include "app_context.hpp"
#include "messages.hpp"

AppQueues_t g_queues;
AppHandles_t g_handles;

void app_context_init(void) {
    g_queues.cola_boton        = xQueueCreate(10, sizeof(ButtonMsg_t));
    g_queues.cola_sensor       = xQueueCreate(10, sizeof(SensorMsg_t));
    g_queues.cola_cmd_servo    = xQueueCreate(10, sizeof(ServoCmd_t));
    g_queues.cola_estado_servo = xQueueCreate(10, sizeof(ServoStatusMsg_t));

    g_handles.h_sensor       = nullptr;
    g_handles.h_servo        = nullptr;
    g_handles.h_boton_inicio = nullptr;
    g_handles.h_boton_vel    = nullptr;
    g_handles.h_led_listo    = nullptr;
    g_handles.h_gestor       = nullptr;
}
