#ifndef CONTROLES_H
#define CONTROLES_H

#include "game.h"

typedef struct {
    bool izquierda, derecha, arriba, abajo, espacio;
} Controles;

void actualizar_controles(Controles *ctrl);
void aplicar_movimiento(EstadoJuego *estado, Controles *ctrl);

//FUNCIONES MODULARES PARA MANEJO DE ESTADOS
void manejar_estado_suelo(EstadoJuego *estado, Controles *ctrl);
void manejar_estado_aereo(EstadoJuego *estado, Controles *ctrl);
void manejar_estado_liana(EstadoJuego *estado, Controles *ctrl);
void aplicar_fisica_jugador(Jugador *jugador);
void limitar_movimiento_jugador(Jugador *jugador);
void verificar_aterrizaje(Jugador *j, EstadoJuego *estado);

// Funciones especificas
void aplicar_gravedad(Jugador *j);
void procesar_movimiento_horizontal_suelo(Jugador *j, Controles *ctrl);
void procesar_movimiento_horizontal_aereo(Jugador *j, Controles *ctrl);
void iniciar_salto(Jugador *j, Controles *ctrl);
bool intentar_agarrar_liana(Jugador *j, EstadoJuego *estado);
void saltar_desde_liana(Jugador *j, Controles *ctrl);
bool cambiar_liana(Jugador *j, EstadoJuego *estado, float direccion);

#endif