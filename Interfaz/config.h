#ifndef CONFIG_H
#define CONFIG_H

// Configuracion de pantalla
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define FPS 60

// Configuracion de matriz de juego
#define MATRIZ_FILAS 30
#define MATRIZ_COLUMNAS 40
#define TAMANIO_CELDA 20

// Tamaños
#define JUGADOR_SIZE 20
#define COCODRILO_SIZE 18
#define FRUTA_SIZE 15
#define LIANA_WIDTH 5
#define PLATAFORMA_HEIGHT 20
#define AGUA_HEIGHT 80

// Colores
#define COLOR_FONDO BLACK
#define COLOR_JUGADOR (Color){184, 134, 72, 255}
#define COLOR_PADRE (Color){101, 67, 33, 255}
#define COLOR_AGUA (Color){100, 149, 237, 255}
#define COLOR_LIANA YELLOW
#define COLOR_PLATAFORMA YELLOW
#define COLOR_MARIO_BASE BLUE
#define COLOR_MARIO_DETALLE RED
#define COLOR_COCODRILO_ROJO RED
#define COLOR_COCODRILO_AZUL (Color){0, 191, 255, 255}
#define COLOR_ISLA_VERDE (Color){34, 139, 34, 255}
#define COLOR_ISLA_SUELO (Color){139, 69, 19, 255}
#define COLOR_NARANJA (Color){255, 165, 0, 255}
#define COLOR_FRUTA_CELESTE (Color){135, 206, 250, 255}
#define COLOR_FRUTA_ROJA (Color){220, 20, 60, 255}
#define COLOR_BANANO (Color){255, 225, 0, 255}

// Tipos de frutas
#define FRUTA_NARANJA 0
#define FRUTA_CELESTE 1
#define FRUTA_ROJA 2
#define FRUTA_BANANO 3

// Configuración de juego
#define VELOCIDAD_JUGADOR 3.0f
#define VELOCIDAD_TREPADO 3.0f 
#define DISTANCIA_SALTO 60.0f
#define DISTANCIA_AGARRE_LIANA 40.0f

// Fisica y movimiento
#define GRAVEDAD 0.6f
#define FUERZA_SALTO -10.0f
#define VELOCIDAD_HORIZONTAL_AIRE 5.5f
#define VELOCIDAD_HORIZONTAL_SUELO 3.5f
#define FRENADO_AIRE 0.85f
#define VELOCIDAD_MAXIMA_CAIDA 8.0f

// Hitboxes
#define JUGADOR_HITBOX 12
#define COCODRILO_HITBOX 14
#define FRUTA_HITBOX 10

// Red
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

// Sprites
#define SPRITE_SIZE 32
#define SPRITE_FONDO "assets/fondo.png"
#define SPRITE_DKJ "assets/dkj.png"           // DK Jr - el personaje principal
#define SPRITE_DKJ_STATIC "assets/dkj_s.png"  // DK Jr estático
#define SPRITE_DK "assets/dk.png"             // Donkey Kong padre
#define SPRITE_MARIO "assets/mario.png"       // Mario (enemigo?)
#define SPRITE_COCODRILO_ROJO "assets/cocorojo.png"
#define SPRITE_COCODRILO_AZUL "assets/cocoazul.png"
#define SPRITE_FRUTA_MANZANA "assets/manzana.png"
#define SPRITE_FRUTA_PLATANO "assets/platano.png"
#define SPRITE_FRUTA_PERAS "assets/peras.png"

// Tamaños de sprites escalados
#define SPRITE_JUGADOR_SIZE 32     // Tamaño visual del jugador
#define SPRITE_PADRE_SIZE 40       // DK padre más grande
#define SPRITE_COCODRILO_SIZE 28   // Cocodrilos
#define SPRITE_FRUTA_SIZE 20       // Frutas

#define DEBUG_COLISIONES true
#define COLOR_SUELO_DEBUG (Color){0, 255, 0, 100}
#define COLOR_LIANA_DEBUG (Color){255, 255, 0, 100}
#define COLOR_MORTAL_DEBUG (Color){255, 0, 0, 100}
#define COLOR_PLATAFORMA_DEBUG (Color){0, 0, 255, 100}
// Debug
#define DEBUG_COLISIONES true
#define COLOR_SUELO_DEBUG (Color){0, 255, 0, 100}
#define COLOR_LIANA_DEBUG (Color){255, 255, 0, 100}
#define COLOR_MORTAL_DEBUG (Color){255, 0, 0, 100}
#define COLOR_PLATAFORMA_DEBUG (Color){0, 0, 255, 100}

#endif