#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "config.h"
#include "types.h"

// ==================== ESTRUCTURAS DE DATOS ====================

// Estados del jugador (para renderizado)
typedef enum {
    ESTADO_SUELO,
    ESTADO_AGARRADO_LIANA, 
    ESTADO_SALTANDO,
    ESTADO_CAYENDO
} EstadoJugador;

// Entidades del juego
typedef struct {
    float x, y;
    float velocidad_x, velocidad_y;
    int vidas;
    int puntuacion;
    bool activo;
    EstadoJugador estado;
    int liana_actual;
    bool en_suelo;
} Jugador;

typedef struct {
    float x, y;
    bool activo;
} Padre;

typedef struct {
    float x, y;
    TipoEnemigo tipo;
    bool activo;
} Cocodrilo;

typedef struct {
    float x, y;
    int puntos;
    TipoFruta tipo;
    bool activo;
} Fruta;

typedef struct {
    float x, y_inicio, y_fin;
} Liana;

typedef struct {
    float x, y, ancho;
} Plataforma;

typedef struct {
    Jugador jugador;
    Padre padre;
    Cocodrilo cocodrilos[50];
    Fruta frutas[30];
    Liana lianas[20];
    Plataforma plataformas[15];
    int num_cocodrilos;
    int num_frutas;
    int num_lianas;
    int num_plataformas;
    bool juego_activo;
} EstadoJuego;

// ==================== FUNCIONES PRINCIPALES ====================

// Inicialización
void inicializar_estado_juego(EstadoJuego *estado);
void inicializar_mapa_estatico(EstadoJuego *estado);

// Conversiones JSON
EstadoPlayerJSON estado_jugador_a_json(EstadoJugador estado);
EstadoJugador estado_jugador_desde_json(EstadoPlayerJSON estado_json);

// Utilidades
void limpiar_estado_juego(EstadoJuego *estado);
void resetear_estado_juego(EstadoJuego *estado);

#endif