#ifndef MESSAGES_HPP
#define MESSAGES_HPP

#include <stdint.h>

enum class IdBoton {
    INICIO,
    VELOCIDAD
};

enum class AccionBoton {
    PRESIONADO,
    LIBERADO
};

struct ButtonMsg_t {
    IdBoton     id;
    AccionBoton accion;
};

struct SensorMsg_t {
    int crudo;
    int filtrado;
    int angulo_objetivo;
};

struct ServoCmd_t {
    int  angulo_objetivo;
    bool modo_rapido;
};

struct ServoStatusMsg_t {
    int  angulo_actual;
    int  angulo_objetivo;
    bool llegado;
};

#endif
