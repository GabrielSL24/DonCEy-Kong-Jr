#ifndef GRAFICOS_H
#define GRAFICOS_H

#include "raylib.h"
#include "game.h"
#include "mapa.h"

// Sistema de sprites
typedef struct {
    Texture2D textura;
    bool cargado;
} Sprite;

typedef struct {
    Sprite fondo;
    Sprite jugador;
    Sprite padre;
    Sprite cocodrilo_rojo;
    Sprite cocodrilo_azul;
    Sprite fruta_naranja;
    Sprite fruta_roja;
    Sprite fruta_celeste;
    Sprite fruta_banano;
} SistemaSprites;

// Declarar la variable global
extern SistemaSprites sprites_global;

// Funciones de dibujo
void inicializar_graficos(void);
void cerrar_graficos(void);
void cargar_sprites(SistemaSprites *sprites);
void descargar_sprites(SistemaSprites *sprites);
void dibujar_escena_completa(EstadoJuego *estado, Mapa *mapa, SistemaSprites *sprites);
void dibujar_jugador_con_sprite(Jugador *j, SistemaSprites *sprites);
void dibujar_padre_con_sprite(Padre *p, SistemaSprites *sprites);
void dibujar_cocodrilo_con_sprite(Cocodrilo *c, SistemaSprites *sprites);
void dibujar_fruta_con_sprite(Fruta *f, SistemaSprites *sprites);
void dibujar_ui(EstadoJuego *estado);
void dibujar_escena(EstadoJuego *estado);

#endif