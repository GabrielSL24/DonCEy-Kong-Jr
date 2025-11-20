#include "mapa.h"
#include "config.h"
#include <stdio.h>
#include <string.h>

Mapa mapa_global;

void cargar_mapa(Mapa *mapa) {
    if (FileExists(SPRITE_FONDO)) {
        // Carga y escala la imagen al tamaño de la ventana
        Image imagen_fondo = LoadImage(SPRITE_FONDO);
        ImageResize(&imagen_fondo, SCREEN_WIDTH, SCREEN_HEIGHT);
        mapa->fondo = LoadTextureFromImage(imagen_fondo);
        UnloadImage(imagen_fondo);
        mapa->cargado = true;
        printf("✓ Mapa cargado y escalado: %s -> %dx%d\n", SPRITE_FONDO, SCREEN_WIDTH, SCREEN_HEIGHT);
    } else {
        mapa->cargado = false;
        printf("✗ No se encontró: %s\n", SPRITE_FONDO);
    }
    
    // Inicializa matriz de colisiones vacía
    for (int f = 0; f < MATRIZ_FILAS; f++) {
        for (int c = 0; c < MATRIZ_COLUMNAS; c++) {
            mapa->matriz_colisiones[f][c] = 0;
        }
    }
}

void descargar_mapa(Mapa *mapa) {
    if (mapa->cargado) {
        UnloadTexture(mapa->fondo);
    }
}

void dibujar_mapa(Mapa *mapa) {
    if (mapa->cargado) {
        // Dibuja el fondo escalado para que ocupe toda la pantalla
        DrawTexture(mapa->fondo, 0, 0, WHITE);
    } else {
        // Fallback: fondo de color sólido
        ClearBackground(COLOR_FONDO);
        DrawText("NO SE ENCONTRO fondo.png", 100, SCREEN_HEIGHT/2, 20, RED);
    }
}

void configurar_mapa_completo(Mapa *mapa, EstadoJuego *estado) {
    printf("=== CONFIGURANDO MAPA PARA DEBUG ===\n");
    
    // Sincroniza con el estado actual del juego
    for (int f = 0; f < MATRIZ_FILAS; f++) {
        for (int c = 0; c < MATRIZ_COLUMNAS; c++) {
            int tipo = estado->matriz.celdas[f][c].tipo;
            
            switch (tipo) {
                case TIPO_PLATAFORMA:
                case TIPO_PADRE:
                    mapa->matriz_colisiones[f][c] = COLISION_SUELO;
                    break;
                case TIPO_LIANA:
                    mapa->matriz_colisiones[f][c] = COLISION_LIANA;
                    break;
                case TIPO_AGUA:
                case TIPO_COCODRILO:
                    mapa->matriz_colisiones[f][c] = COLISION_MORTAL;
                    break;
                default:
                    mapa->matriz_colisiones[f][c] = 0;
            }
        }
    }
    printf("Mapa de debug sincronizado con estado del juego\n");
}

void dibujar_superficies_debug(Mapa *mapa) {
    if (!DEBUG_COLISIONES) return;
    
    for (int f = 0; f < MATRIZ_FILAS; f++) {
        for (int c = 0; c < MATRIZ_COLUMNAS; c++) {
            if (mapa->matriz_colisiones[f][c] != 0) {
                Color color_debug;
                switch (mapa->matriz_colisiones[f][c]) {
                    case COLISION_SUELO: color_debug = COLOR_SUELO_DEBUG; break;
                    case COLISION_LIANA: color_debug = COLOR_LIANA_DEBUG; break;
                    case COLISION_MORTAL: color_debug = COLOR_MORTAL_DEBUG; break;
                    case COLISION_PLATAFORMA: color_debug = COLOR_PLATAFORMA_DEBUG; break;
                    default: color_debug = (Color){128, 128, 128, 50};
                }
                DrawRectangle(c * TAMANIO_CELDA, f * TAMANIO_CELDA, 
                             TAMANIO_CELDA, TAMANIO_CELDA, color_debug);
            }
        }
    }
}