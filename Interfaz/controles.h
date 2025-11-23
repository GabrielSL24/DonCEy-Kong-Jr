#ifndef CONTROLES_H
#define CONTROLES_H

#include <stdbool.h>

#define MAX_INPUTS_PER_FRAME 10

// Estructura para inputs por frame (DEFINIDA UNA SOLA VEZ)
typedef struct {
    char inputs[MAX_INPUTS_PER_FRAME][20]; // "LEFT_PRESSED", "RIGHT_RELEASED", etc.
    int num_inputs;
} FrameInputs;

// Estructura simplificada alternativa
typedef struct {
    bool izquierda, derecha, arriba, abajo, espacio;
} Controles;

// ==================== FUNCIONES PRINCIPALES ====================
void detectar_inputs_frame(FrameInputs *frame_inputs);
void actualizar_controles(Controles *ctrl);

// ==================== FUNCIONES DE CONVERSIÓN ====================
bool hay_inputs_nuevos(const Controles *ctrl);
const char* obtener_input_presionado(const Controles *ctrl);
void limpiar_controles(Controles *ctrl);

// ==================== FUNCIONES DE CONSULTA DIRECTA ====================
bool tecla_izquierda_presionada(void);
bool tecla_derecha_presionada(void);
bool tecla_arriba_presionada(void);
bool tecla_abajo_presionada(void);
bool tecla_espacio_presionada(void);

#endif