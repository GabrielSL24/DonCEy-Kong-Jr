#ifndef CONEXION_H
#define CONEXION_H

#include "game.h"
#include <stdint.h>
#include "config.h"
#include "controles.h"

// Estructura para enviar inputs al servidor
typedef struct {
    bool izquierda, derecha, arriba, abajo, espacio;
    int jugador_id;
    uint32_t timestamp;
} PaqueteInput;

// Estructura para recibir estado del servidor  
typedef struct {
    int matriz[MATRIZ_FILAS][MATRIZ_COLUMNAS];
    int vidas;
    int puntuacion;
    bool juego_activo;
} PaqueteEstado;

bool conectar_servidor(const char* ip);
bool enviar_estado_actual_al_servidor(int matriz_x, int matriz_y, int vidas, int puntuacion);
bool recibir_consecuencias_del_servidor(int *vidas, int *puntuacion, bool *juego_activo);
void desconectar_servidor(void);

// Función temporal para simular servidor
//void simular_servidor_local(EstadoJuego *estado, Controles *ctrl);

extern bool servidor_conectado;
#endif