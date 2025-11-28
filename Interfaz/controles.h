#ifndef CONTROLES_H
#define CONTROLES_H

#include <stdbool.h>

#define MAX_INPUTS_PER_FRAME 10

// Estructura para inputs por frame 
typedef struct {
    char inputs[MAX_INPUTS_PER_FRAME][20]; // "LEFT_PRESSED", "RIGHT_RELEASED"..
    int num_inputs;
    bool enter_pressed;
    bool escape_pressed;
    bool backspace_pressed;
    int seleccion_menu; // Para navegacion (-1 = arriba, 1 = abajo, 0 = sin cambio)
} FrameInputs;

// Estructura para estado actual de controles
typedef struct {
    bool izquierda, derecha, arriba, abajo, espacio;
} Controles;

// ==================== FUNCIONES PRINCIPALES ====================
void detectar_inputs_frame(FrameInputs *frame_inputs);
void actualizar_controles(Controles *ctrl);

// ==================== FUNCIONES DE CONVERSION ====================
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