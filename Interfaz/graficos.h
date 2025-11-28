#ifndef GRAFICOS_H
#define GRAFICOS_H

#include "raylib.h"
#include "types.h"

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
    Sprite fruta_banana;
    Sprite fruta_apple;
    Sprite fruta_pear;
    Sprite fruta_orange;
} SistemaSprites;

// Declarar la variable global
extern SistemaSprites sprites_global;

// ==================== FUNCIONES DE INICIALIZACIÓN ====================
void inicializar_graficos(void);
void cerrar_graficos(void);
void cargar_sprites(SistemaSprites *sprites);
void descargar_sprites(SistemaSprites *sprites);

// ==================== FUNCIONES DE MAPA/FONDO ====================
void cargar_fondo(SistemaSprites *sprites);
void descargar_fondo(SistemaSprites *sprites);
void dibujar_fondo(const SistemaSprites *sprites);

// ==================== FUNCIONES DE RENDERIZADO ====================
void dibujar_escena_completa(const EstadoJuego *estado, const SistemaSprites *sprites);
void dibujar_ui(const EstadoJuego *estado);
void dibujar_interfaz_menu(EstadoMenu estado, int seleccion, InfoPartida partidas[], int count, const char* partida_actual);

// ==================== FUNCIONES DE DIBUJO DE ENTIDADES ====================
void dibujar_jugador(const Jugador *jugador, const SistemaSprites *sprites);
void dibujar_padre(const Padre *padre, const SistemaSprites *sprites);
void dibujar_cocodrilo(const Cocodrilo *cocodrilo, const SistemaSprites *sprites);
void dibujar_fruta(const Fruta *fruta, const SistemaSprites *sprites);
void dibujar_lianas(const Liana lianas[], int num_lianas);
void dibujar_plataformas(const Plataforma plataformas[], int num_plataformas);

// ==================== FUNCIONES DE DEBUG ====================
void dibujar_debug_info(const EstadoJuego *estado);
void dibujar_hitboxes(const EstadoJuego *estado);



#endif