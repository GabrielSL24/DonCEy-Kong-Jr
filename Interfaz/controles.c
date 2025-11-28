#include "controles.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>

// ==================== DETECCIÓN DE CONTROLES ====================

void actualizar_controles(Controles *ctrl) {
    // Solo actualizar estado actual de las teclas
    ctrl->izquierda = IsKeyDown(KEY_LEFT);
    ctrl->derecha = IsKeyDown(KEY_RIGHT);
    ctrl->arriba = IsKeyDown(KEY_UP);
    ctrl->abajo = IsKeyDown(KEY_DOWN);
    ctrl->espacio = IsKeyDown(KEY_SPACE);
}

// ==================== DETECCIÓN DE INPUTS NUEVOS ====================

bool hay_inputs_nuevos(const Controles *ctrl) {
    // Verificar si alguna tecla fue presionada (no solo mantenida)
    return IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT) || 
           IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN) ||
           IsKeyPressed(KEY_SPACE);
}

const char* obtener_input_presionado(const Controles *ctrl) {
    // Devolver el primer input presionado en este frame
    if (IsKeyPressed(KEY_LEFT)) return "LEFT_PRESSED";
    if (IsKeyPressed(KEY_RIGHT)) return "RIGHT_PRESSED";
    if (IsKeyPressed(KEY_UP)) return "UP_PRESSED";
    if (IsKeyPressed(KEY_DOWN)) return "DOWN_PRESSED";
    if (IsKeyPressed(KEY_SPACE)) return "JUMP_PRESSED";
    
    // También detectar releases para movimiento más preciso
    if (IsKeyReleased(KEY_LEFT)) return "LEFT_RELEASED";
    if (IsKeyReleased(KEY_RIGHT)) return "RIGHT_RELEASED";
    if (IsKeyReleased(KEY_UP)) return "UP_RELEASED";
    if (IsKeyReleased(KEY_DOWN)) return "DOWN_RELEASED";
    if (IsKeyReleased(KEY_SPACE)) return "JUMP_RELEASED";
    
    return NULL; // No hay inputs nuevos
}

void limpiar_controles(Controles *ctrl) {
    // Por ahora no es necesario limpiar, pero se mantiene la interfaz
    // para futuras extensiones (como input buffering)
}

// ==================== FUNCIONES DE CONSULTA DIRECTA ====================

bool tecla_izquierda_presionada(void) {
    return IsKeyPressed(KEY_LEFT);
}

bool tecla_derecha_presionada(void) {
    return IsKeyPressed(KEY_RIGHT);
}

bool tecla_arriba_presionada(void) {
    return IsKeyPressed(KEY_UP);
}

bool tecla_abajo_presionada(void) {
    return IsKeyPressed(KEY_DOWN);
}

bool tecla_espacio_presionada(void) {
    return IsKeyPressed(KEY_SPACE);
}

// ==================== DETECCIÓN POR FRAME (USANDO FrameInputs DEFINIDO) ====================

void detectar_inputs_frame(FrameInputs *frame_inputs) {
    // Inicializar el frame
    frame_inputs->num_inputs = 0;
    frame_inputs->enter_pressed = false;
    frame_inputs->escape_pressed = false;
    frame_inputs->backspace_pressed = false;
    frame_inputs->seleccion_menu = 0;
    
    // ✅ CORREGIDO: Usar solo KEY_PRESSED y KEY_RELEASED
    // KEY PRESSED (solo cuando se presiona por primera vez)
    if (IsKeyPressed(KEY_LEFT)) {
        strcpy(frame_inputs->inputs[frame_inputs->num_inputs++], "LEFT_PRESSED");
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        strcpy(frame_inputs->inputs[frame_inputs->num_inputs++], "RIGHT_PRESSED");
    }
    if (IsKeyPressed(KEY_UP)) {
        strcpy(frame_inputs->inputs[frame_inputs->num_inputs++], "UP_PRESSED");
    }
    if (IsKeyPressed(KEY_DOWN)) {
        strcpy(frame_inputs->inputs[frame_inputs->num_inputs++], "DOWN_PRESSED");
    }
    if (IsKeyPressed(KEY_SPACE)) {
        strcpy(frame_inputs->inputs[frame_inputs->num_inputs++], "JUMP_PRESSED");
    }
    
    // KEY RELEASED (cuando se suelta la tecla)
    if (IsKeyReleased(KEY_LEFT)) {
        strcpy(frame_inputs->inputs[frame_inputs->num_inputs++], "LEFT_RELEASED");
    }
    if (IsKeyReleased(KEY_RIGHT)) {
        strcpy(frame_inputs->inputs[frame_inputs->num_inputs++], "RIGHT_RELEASED");
    }
    if (IsKeyReleased(KEY_UP)) {
        strcpy(frame_inputs->inputs[frame_inputs->num_inputs++], "UP_RELEASED");
    }
    if (IsKeyReleased(KEY_DOWN)) {
        strcpy(frame_inputs->inputs[frame_inputs->num_inputs++], "DOWN_RELEASED");
    }
    if (IsKeyReleased(KEY_SPACE)) {
        strcpy(frame_inputs->inputs[frame_inputs->num_inputs++], "JUMP_RELEASED");
    }
    
    // Inputs para menú (se mantienen igual)
    frame_inputs->enter_pressed = IsKeyPressed(KEY_ENTER);
    frame_inputs->escape_pressed = IsKeyPressed(KEY_ESCAPE);
    frame_inputs->backspace_pressed = IsKeyPressed(KEY_BACKSPACE);
    
    // Navegación en menú
    if (IsKeyPressed(KEY_UP)) {
        frame_inputs->seleccion_menu = -1;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        frame_inputs->seleccion_menu = 1;
    }
    
    // Debug
    if (frame_inputs->num_inputs > 0) {
        printf("🎮 Inputs detectados: ");
        for (int i = 0; i < frame_inputs->num_inputs; i++) {
            printf("%s ", frame_inputs->inputs[i]);
        }
        printf("\n");
    }
}