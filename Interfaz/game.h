#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "config.h"

// Tipos de elementos en la matriz
#define TIPO_VACIO 0
#define TIPO_PLATAFORMA 1
#define TIPO_LIANA 2
#define TIPO_AGUA 3
#define TIPO_FRUTA 4
#define TIPO_COCODRILO 5
#define TIPO_JUGADOR 6
#define TIPO_PADRE 7
#define TIPO_MARIO 8

typedef enum {
    ESTADO_SUELO,
    ESTADO_AGARRADO_LIANA,
    ESTADO_SALTANDO,
    ESTADO_CAYENDO
} EstadoJugador;

typedef struct {
    float x, y;
    float velocidad_x, velocidad_y;
    int vidas;
    int puntuacion;
    bool activo;
    EstadoJugador estado;
    int liana_actual;
    bool en_suelo;
    bool puede_saltar;
    float tiempo_en_aire;
} Jugador;

typedef struct {
    float x, y;
    bool activo;
} Padre;

typedef struct {
    float x, y;
    int tipo;
    bool activo;
} Cocodrilo;

typedef struct {
    float x, y;
    int puntos;
    int tipo;
    bool activo;
} Fruta;

typedef struct {
    float x, y_inicio, y_fin;
} Liana;

typedef struct {
    float x, y, ancho;
} Plataforma;

typedef struct {
    float x, y, ancho;
    Color color_superior, color_inferior;
} Isla;

typedef struct {
    int tipo;
    int datos_extra;
} CeldaMatriz;

typedef struct {
    CeldaMatriz celdas[MATRIZ_FILAS][MATRIZ_COLUMNAS];
} MatrizJuego;

typedef struct {
    Jugador jugador;
    Padre padre;
    Cocodrilo cocodrilos[50];
    Fruta frutas[30];
    Liana lianas[20];
    Plataforma plataforma_superior;
    Isla islas[10];
    MatrizJuego matriz;
    int num_cocodrilos;
    int num_frutas;
    int num_lianas;
    int num_islas;
    bool juego_activo;
} EstadoJuego;

// Declaraciones de funciones
void inicializar_juego(EstadoJuego *estado);
void inicializar_matriz(MatrizJuego *matriz, EstadoJuego *estado);
void actualizar_matriz_desde_estado(EstadoJuego *estado);
void verificar_colisiones_matriz(EstadoJuego *estado);

// Funciones de utilidad para matriz
void coordenadas_a_matriz(float x, float y, int *fila, int *columna);
void matriz_a_coordenadas(int fila, int columna, float *x, float *y);
bool celda_es_solida(int tipo_celda);
bool celda_es_liana(int tipo_celda);
bool celda_es_mortal(int tipo_celda);

// Funciones de movimiento basadas en matriz
bool esta_en_superficie_matriz(Jugador *jugador, MatrizJuego *matriz);
int liana_mas_cercana_matriz(Jugador *jugador, MatrizJuego *matriz);

// Funciones obsoletas (mantener temporalmente para compilación)
bool esta_en_superficie(Jugador *jugador, EstadoJuego *estado);
int liana_mas_cercana(Jugador *jugador, Liana lianas[], int num_lianas);
bool esta_sobre_isla(Jugador *jugador, Isla islas[], int num_islas);

#endif