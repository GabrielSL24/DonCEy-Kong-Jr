#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

// ==================== TIPOS COMPARTIDOS CON SERVIDOR ====================

// Tipos de enemigos
typedef enum {
    ENEMY_RED_CROCODILE,
    ENEMY_BLUE_CROCODILE
} TipoEnemigo;

// Tipos de frutas  
typedef enum {
    FRUIT_BANANA,
    FRUIT_APPLE, 
    FRUIT_PEAR,
    FRUIT_ORANGE
} TipoFruta;

// Estados del jugador (para JSON)
typedef enum {
    PLAYER_STANDING,
    PLAYER_MOVING_LEFT,
    PLAYER_MOVING_RIGHT, 
    PLAYER_CLIMBING,
    PLAYER_JUMPING,
    PLAYER_FALLING
} EstadoPlayerJSON;

// Tipos de cliente
typedef enum {
    CLIENT_PLAYER,
    CLIENT_SPECTATOR
} TipoCliente;

// ==================== NUEVOS ENUMS PARA PROTOCOLO COMPLETO ====================

// Tipos de request (COMPLETAR según protocolo)
typedef enum {
    REQUEST_CREATE_GAME,
    REQUEST_JOIN_GAME,
    REQUEST_LEAVE_GAME,
    REQUEST_START_GAME,    // ← FALTABA
    REQUEST_LIST_GAMES,
    REQUEST_GAME_INPUT     // ← FALTABA
} TipoRequest;

// Tipos de response (COMPLETAR según protocolo)  
typedef enum {
    RESPONSE_GAME_CREATED,
    RESPONSE_GAME_JOINED,
    RESPONSE_GAME_STARTED,
    RESPONSE_GAME_LEFT,
    RESPONSE_SPECTATOR_JOINED,
    RESPONSE_SPECTATOR_LEFT, 
    RESPONSE_GAME_LIST,
    RESPONSE_GAME_STATE,
    RESPONSE_ERROR
} TipoResponse;

// Estados de respuesta del servidor
typedef enum {
    STATUS_SUCCESS,
    STATUS_ERROR
} EstadoRespuesta;

// Tipos de input (para JSON)
typedef enum {
    INPUT_KEY_PRESSED,
    INPUT_KEY_RELEASED
} TipoInput;

// ==================== TIPOS INTERNOS DEL CLIENTE ====================

// Estados internos del jugador (para renderizado)
typedef enum {
    ESTADO_SUELO,
    ESTADO_AGARRADO_LIANA,
    ESTADO_SALTANDO,
    ESTADO_CAYENDO
} EstadoJugador;

// Estados del menú (ESTÁ BIEN)
typedef enum {
    MENU_MAIN,
    MENU_SELECT_GAME, 
    MENU_PLAYING,
    MENU_SPECTATING,
    MENU_CREATING_GAME,
    MENU_JOINING_GAME,  
} EstadoMenu;

// Información de partida (ESTÁ BIEN)
typedef struct {
    char game_id[50];
    int player_count;
    int spectators;
    bool active;
} InfoPartida;

// ==================== ESTRUCTURAS DEL JUEGO ====================

// Entidades del juego (solo para renderizado) - ESTÁN BIEN
typedef struct {
    float x, y;
    int vidas;
    int puntuacion;
    bool activo;
    EstadoJugador estado;
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

// Estado completo del juego (solo para renderizar) - ESTÁ BIEN
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

#endif