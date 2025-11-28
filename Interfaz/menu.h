#ifndef MENU_H
#define MENU_H

#include "types.h"
#include "conexion.h"
#include "controles.h"

// ==================== FUNCIONES DE ACTUALIZACION DE MENU ====================
void actualizar_menu_principal(FrameInputs* inputs, EstadoMenu* estado_menu, int* seleccion_actual);
void actualizar_seleccion_partida(FrameInputs* inputs, EstadoMenu* estado_menu, int* seleccion_actual);
void actualizar_modo_jugador(FrameInputs* inputs, EstadoMenu* estado_menu, EstadoJuego* estado_juego);
void actualizar_modo_espectador(FrameInputs* inputs, EstadoMenu* estado_menu, EstadoJuego* estado_juego); 
void actualizar_estado_espera(FrameInputs* inputs, EstadoMenu* estado_menu, EstadoJuego* estado_juego); 

// ==================== FUNCIONES DE RENDERIZADO DE MENU ====================
void dibujar_menu_principal(int seleccion);
void dibujar_seleccion_partida(int seleccion, InfoPartida partidas[], int count);
void dibujar_hud_espectador(const char* partida_actual);
void dibujar_hud_jugador(const char* partida_actual);
void dibujar_pantalla_game_over(void);
void dibujar_pantalla_victoria(void);
void dibujar_estado_espera(EstadoMenu estado);

#endif 