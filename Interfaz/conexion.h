#ifndef CONEXION_H
#define CONEXION_H

#include "game.h"
#include <stdint.h>
#include "config.h"
#include "controles.h"

// Estructura para enviar inputs al servidor
typedef struct {
    float x, y;          // Posición actual del jugador
    int vidas;           // Vidas actuales  
    int puntuacion;      // Puntuación actual
    int jugador_id;
    uint32_t timestamp;
} PaquetePosicion;

// Estructura para recibir estado del servidor  
typedef struct {
    int matriz[15][20];
    float jugador_x, jugador_y;
    int vidas;
    int puntuacion;
    bool juego_activo;
    int jugador_id;
    uint32_t timestamp;
} PaqueteEstado;

// Placeholders actualizados
bool conectar_servidor(const char* ip);
bool enviar_estado_actual_al_servidor(float x, float y, int vidas, int puntos);
bool recibir_consecuencias_del_servidor(int *vidas, int *puntos, bool *activo);
void desconectar_servidor(void);

// Función temporal para simular servidor
//void simular_servidor_local(EstadoJuego *estado, Controles *ctrl);

//Variables globales de conexion
//extern SOCKET socket_servidor;
extern bool servidor_conectado;

#endif