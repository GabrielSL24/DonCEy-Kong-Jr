// types.h
#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

// Tipos compartidos entre todos los módulos
typedef enum {
    ENEMY_RED_CROCODILE,
    ENEMY_BLUE_CROCODILE
} TipoEnemigo;

typedef enum {
    FRUIT_BANANA,
    FRUIT_APPLE, 
    FRUIT_PEAR,
    FRUIT_ORANGE
} TipoFruta;

typedef enum {
    PLAYER_STANDING,
    PLAYER_MOVING_LEFT,
    PLAYER_MOVING_RIGHT,
    PLAYER_CLIMBING,
    PLAYER_JUMPING,
    PLAYER_FALLING
} EstadoPlayerJSON;

typedef enum {
    CLIENT_PLAYER,
    CLIENT_SPECTATOR
} TipoCliente;

#endif