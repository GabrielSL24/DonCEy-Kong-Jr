#ifndef MAPA_H
#define MAPA_H

#include "raylib.h"
#include "config.h"
#include "game.h"

// Tipos de colisión para debug visual
#define COLISION_SUELO 1
#define COLISION_LIANA 2
#define COLISION_MORTAL 3
#define COLISION_PLATAFORMA 4

typedef struct {
    Texture2D fondo;
    int matriz_colisiones[MATRIZ_FILAS][MATRIZ_COLUMNAS];
    bool cargado;
} Mapa;

extern Mapa mapa_global;

void cargar_mapa(Mapa *mapa);
void descargar_mapa(Mapa *mapa);
void dibujar_mapa(Mapa *mapa);
void configurar_mapa_completo(Mapa *mapa, EstadoJuego *estado);
void dibujar_superficies_debug(Mapa *mapa);

#endif