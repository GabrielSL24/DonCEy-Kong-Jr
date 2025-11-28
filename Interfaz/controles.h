#ifndef CONTROLES_H
#define CONTROLES_H

#include <stdbool.h>

#define MAX_INPUTS_PER_FRAME 10

// Estructura para inputs por frame (EXTENDIDA)
typedef struct {
    char inputs[MAX_INPUTS_PER_FRAME][20]; // "LEFT_PRESSED", "RIGHT_RELEASED", etc.
    int num_inputs;
    
    // Nuevos inputs para menú
    bool enter_pressed;
    bool escape_pressed;
    bool backspace_pressed;
    int seleccion_menu; // Para navegación (-1 = arriba, 1 = abajo, 0 = sin cambio)
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