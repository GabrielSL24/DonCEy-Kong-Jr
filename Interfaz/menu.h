#ifndef MENU_H
#define MENU_H

#include "types.h"
#include "conexion.h"
#include "controles.h"

// ==================== FUNCIONES DE ACTUALIZACIÓN DE MENÚ ====================
void actualizar_menu_principal(FrameInputs* inputs, EstadoMenu* estado_menu, int* seleccion_actual);
void actualizar_seleccion_partida(FrameInputs* inputs, EstadoMenu* estado_menu, int* seleccion_actual);
void actualizar_modo_jugador(FrameInputs* inputs, EstadoMenu* estado_menu, EstadoJuego* estado_juego); // ← QUITAR partida_seleccionada
void actualizar_modo_espectador(FrameInputs* inputs, EstadoMenu* estado_menu, EstadoJuego* estado_juego); // ← QUITAR partida_seleccionada
void actualizar_estado_espera(FrameInputs* inputs, EstadoMenu* estado_menu, EstadoJuego* estado_juego); // ← AGREGAR ESTA DECLARACIÓN

// ==================== FUNCIONES DE RENDERIZADO DE MENÚ ====================
void dibujar_menu_principal(int seleccion);
void dibujar_seleccion_partida(int seleccion, InfoPartida partidas[], int count);
void dibujar_hud_espectador(const char* partida_actual);
void dibujar_hud_jugador(const char* partida_actual);

#endif