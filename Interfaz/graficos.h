#ifndef GRAFICOS_H
#define GRAFICOS_H

#include "game.h"
//solo funciones de dibujo
void inicializar_graficos(void);
void cerrar_graficos(void);
void dibujar_escena(EstadoJuego *estado);
void dibujar_ui(EstadoJuego *estado);
void dibujar_jugador(Jugador *j);
void dibujar_cocodrilo(Cocodrilo *c);
void dibujar_fruta(Fruta *f);
void dibujar_agua(void);
void dibujar_plataforma_superior(Plataforma *p);
void dibujar_padre(Padre *p);
void dibujar_isla(Isla *isla);
void dibujar_mario(void);

#endif