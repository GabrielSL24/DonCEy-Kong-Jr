#include "raylib.h"
#include "controles.h"
#include "config.h"
#include <math.h>
#include <stdio.h>

void actualizar_controles(Controles *ctrl) {
    ctrl->izquierda = IsKeyDown(KEY_LEFT);
    ctrl->derecha = IsKeyDown(KEY_RIGHT);
    ctrl->arriba = IsKeyDown(KEY_UP);
    ctrl->abajo = IsKeyDown(KEY_DOWN);
    ctrl->espacio = IsKeyDown(KEY_SPACE);
}

// ==================== FUNCIONES PEQUEÑAS Y ESPECÍFICAS ====================
void aplicar_gravedad(Jugador *j) {
    if (j->estado != ESTADO_AGARRADO_LIANA) {
        j->velocidad_y += 0.5f;
        if (j->velocidad_y > 10.0f) {
            j->velocidad_y = 10.0f;
        }
    }
}

void procesar_movimiento_horizontal_suelo(Jugador *j, Controles *ctrl) {
    j->velocidad_x = 0;
    if (ctrl->izquierda) j->velocidad_x = -VELOCIDAD_HORIZONTAL_SUELO;
    if (ctrl->derecha) j->velocidad_x = VELOCIDAD_HORIZONTAL_SUELO;
}

void procesar_movimiento_horizontal_aereo(Jugador *j, Controles *ctrl) {
    if (ctrl->izquierda) {
        j->velocidad_x -= 0.5f;
        if (j->velocidad_x < -VELOCIDAD_HORIZONTAL_AIRE) 
            j->velocidad_x = -VELOCIDAD_HORIZONTAL_AIRE;
    } 
    else if (ctrl->derecha) {
        j->velocidad_x += 0.5f;
        if (j->velocidad_x > VELOCIDAD_HORIZONTAL_AIRE) 
            j->velocidad_x = VELOCIDAD_HORIZONTAL_AIRE;
    }
    else {
        j->velocidad_x *= FRENADO_AIRE;
        if (fabs(j->velocidad_x) < 0.3f) j->velocidad_x = 0;
    }
}

void iniciar_salto(Jugador *j, Controles *ctrl) {
    j->estado = ESTADO_SALTANDO;
    j->velocidad_y = FUERZA_SALTO;
    j->puede_saltar = false;
    j->tiempo_en_aire = 0;
    
    if (ctrl->izquierda) j->velocidad_x = -VELOCIDAD_HORIZONTAL_AIRE;
    else if (ctrl->derecha) j->velocidad_x = VELOCIDAD_HORIZONTAL_AIRE;
    
    printf("DEBUG: Salto iniciado - velY=%.1f\n", j->velocidad_y);
}

// ==================== FUNCION IMPORTANTE ==================== TODO corregir para version final
bool intentar_agarrar_liana(Jugador *j, EstadoJuego *estado) {
    int liana_cercana = liana_mas_cercana_matriz(j, &estado->matriz);
    if (liana_cercana != -1) {
        // Calcula posicion exacta de la liana
        float x_liana = liana_cercana * TAMANIO_CELDA + TAMANIO_CELDA / 2;
        
        // Verifica distancia horizontal ACTUAL del jugador
        float distancia_horizontal = fabs(j->x - x_liana);
        
        printf("Liana en columna %d, posición X=%.1f, jugador X=%.1f, distancia=%.1f (límite=%.1f)\n", 
               liana_cercana, x_liana, j->x, distancia_horizontal, DISTANCIA_AGARRE_LIANA);
        
        //Solo agarrar si YA está suficientemente cerca
        //NO ajustar la posicion automaticamente
        if (distancia_horizontal <= DISTANCIA_AGARRE_LIANA) {
            j->estado = ESTADO_AGARRADO_LIANA;
            // Solo cambiar el estado y resetear velocidades   
            j->velocidad_y = 0;
            j->velocidad_x = 0;
            j->liana_actual = liana_cercana;
            
            printf("Se agarro la liana, Manteniendo posicion: (%.1f,%.1f)\n", j->x, j->y);
            return true;
        } else {
            printf("Liana demasiado lejana para agarrar: %.1f > %.1f\n", 
                   distancia_horizontal, DISTANCIA_AGARRE_LIANA);
        }
    } else {
        printf("No se encontraron lianas cercanas\n");
    }
    return false;
}

void saltar_desde_liana(Jugador *j, Controles *ctrl) {
    j->estado = ESTADO_SALTANDO;
    j->velocidad_y = FUERZA_SALTO * 0.7f;
    j->velocidad_x = (ctrl->derecha ? VELOCIDAD_HORIZONTAL_AIRE : 
                     (ctrl->izquierda ? -VELOCIDAD_HORIZONTAL_AIRE : 0));
    j->liana_actual = -1;
    j->puede_saltar = false;
}

bool cambiar_liana(Jugador *j, EstadoJuego *estado, float direccion) {
    if (direccion == 0) return false;
    
    int fila_actual, col_actual;
    coordenadas_a_matriz(j->x, j->y, &fila_actual, &col_actual);
    
    int nueva_columna = col_actual + (int)direccion;
    printf("Intentando cambiar de liana: %d -> %d\n", col_actual, nueva_columna);
    
    if (nueva_columna >= 0 && nueva_columna < MATRIZ_COLUMNAS) {
        // Verifica si hay liana en la nueva columna en un rango vertical
        for (int f_offset = -2; f_offset <= 2; f_offset++) {
            int fila_check = fila_actual + f_offset;
            if (fila_check < 0 || fila_check >= MATRIZ_FILAS) continue;
            
            if (estado->matriz.celdas[fila_check][nueva_columna].tipo == TIPO_LIANA) {
                //Cambio suave entre lianas
                float x_nueva_liana = nueva_columna * TAMANIO_CELDA + TAMANIO_CELDA / 2;
                j->x = x_nueva_liana; //Cambio suave a la nueva posicion
                j->liana_actual = nueva_columna;
                printf("Cambió a liana en columna %d\n", nueva_columna);
                return true;
            }
        }
    }
    
    return false;
}

// ==================== MANEJADORES DE ESTADO ====================

void manejar_estado_suelo(EstadoJuego *estado, Controles *ctrl) {
    Jugador *j = &estado->jugador;
    
    bool sigue_en_isla = esta_sobre_isla(j, estado->islas, estado->num_islas);
    bool en_fondo = (j->y >= SCREEN_HEIGHT - JUGADOR_HITBOX/2 - 5);
    
    if (!sigue_en_isla && !en_fondo) {
        j->estado = ESTADO_CAYENDO;
        printf("DEBUG: Salio de la isla. Cambiando a CAYENDO\n");
        return;
    }
    
    j->puede_saltar = true;
    j->tiempo_en_aire = 0;
    
    procesar_movimiento_horizontal_suelo(j, ctrl);
    
    // INTENTAR AGARRAR LIANA DESDE SUELO
    if (ctrl->arriba && !ctrl->espacio) {
        printf("Intentando agarrar liana desde suelo...\n");
        if (intentar_agarrar_liana(j, estado)) {
            printf("¡Agarro liana desde el suelo!\n");
            return;
        } else {
            printf("No hay lianas cercanas desde suelo\n");
        }
    }
    
    if (ctrl->espacio && j->puede_saltar) {
        iniciar_salto(j, ctrl);
    }
}

void manejar_estado_aereo(EstadoJuego *estado, Controles *ctrl) {
    Jugador *j = &estado->jugador;
    
    j->tiempo_en_aire++;
    procesar_movimiento_horizontal_aereo(j, ctrl);
    
    if (j->velocidad_y > 2.0f && j->estado == ESTADO_SALTANDO) {
        j->estado = ESTADO_CAYENDO;
    }
    
    //Intentar agarrar liana en cualquier momento en el aire
    if (ctrl->arriba) {
        printf("Intentando agarrar liana en aire...\n");
        if (intentar_agarrar_liana(j, estado)) {
            printf("¡Agarro liana en el aire!\n");
        }
    }
}

void manejar_estado_liana(EstadoJuego *estado, Controles *ctrl) {
    Jugador *j = &estado->jugador;
    
    printf("=== EN LIANA - Posicion: (%.1f,%.1f) ===\n", j->x, j->y);
    
    j->velocidad_x = 0;
    j->velocidad_y = 0;
    
    // Movimiento vertical
    if (ctrl->arriba) {
        float y_anterior = j->y;
        j->y -= VELOCIDAD_TREPADO;
        printf("↑ TREPANDO: %.1f -> %.1f (delta: %.1f)\n", y_anterior, j->y, VELOCIDAD_TREPADO);
    }
    if (ctrl->abajo) {
        float y_anterior = j->y;
        j->y += VELOCIDAD_TREPADO;
        printf("↓ BAJANDO: %.1f -> %.1f (delta: %.1f)\n", y_anterior, j->y, VELOCIDAD_TREPADO);
    }
    
    // Límites verticales
    if (j->y < TAMANIO_CELDA * 3) {
        j->y = TAMANIO_CELDA * 3;
        printf("Llego al tope superior\n");
    }
    if (j->y > SCREEN_HEIGHT - AGUA_HEIGHT - JUGADOR_HITBOX - 10) {
        j->y = SCREEN_HEIGHT - AGUA_HEIGHT - JUGADOR_HITBOX - 10;
        printf("Llego al tope inferior\n");
    }
    
    // Salto desde liana
    if (ctrl->espacio) {
        printf("🚀 SALTANDO DESDE LIANA\n");
        saltar_desde_liana(j, ctrl);
        return;
    }
    
    // Cambio de liana
    if (ctrl->derecha && !ctrl->izquierda) {
        printf("→ CAMBIANDO A DERECHA\n");
        if (cambiar_liana(j, estado, 1.0f)) {
            printf("✅ Cambió a liana derecha\n");
        } else {
            printf("❌ No hay liana derecha - SOLTÁNDOSE\n");
            j->estado = ESTADO_CAYENDO;
        }
    }
    else if (ctrl->izquierda && !ctrl->derecha) {
        printf("← CAMBIANDO A IZQUIERDA\n");
        if (cambiar_liana(j, estado, -1.0f)) {
            printf("✅ Cambió a liana izquierda\n");
        } else {
            printf("❌ No hay liana izquierda - SOLTÁNDOSE\n");
            j->estado = ESTADO_CAYENDO;
        }
    }
}

// ==================== FUNCIONES FÍSICAS ====================

void aplicar_fisica_jugador(Jugador *jugador) {
    aplicar_gravedad(jugador);
    jugador->x += jugador->velocidad_x;
    jugador->y += jugador->velocidad_y;
}

void limitar_movimiento_jugador(Jugador *j) {
    // Limites horizontales
    if (j->x < JUGADOR_HITBOX/2) {
        j->x = JUGADOR_HITBOX/2;
        j->velocidad_x = 0;
    }
    if (j->x > SCREEN_WIDTH - JUGADOR_HITBOX/2) {
        j->x = SCREEN_WIDTH - JUGADOR_HITBOX/2;
        j->velocidad_x = 0;
    }
    
    // Limites verticales
    if (j->y < JUGADOR_HITBOX/2) {
        j->y = JUGADOR_HITBOX/2;
        j->velocidad_y = 0;
        j->estado = ESTADO_SUELO;
    }
    if (j->y > SCREEN_HEIGHT - JUGADOR_HITBOX/2) {
        j->y = SCREEN_HEIGHT - JUGADOR_HITBOX/2;
        j->velocidad_y = 0;
        j->estado = ESTADO_SUELO;
    }
    
    printf("DEBUG Limites: Jugador en (%.1f, %.1f)\n", j->x, j->y);
}

void verificar_aterrizaje(Jugador *j, EstadoJuego *estado) {
    if (j->estado == ESTADO_AGARRADO_LIANA) {
        return;
    }
    
    bool en_isla = esta_sobre_isla(j, estado->islas, estado->num_islas);
    bool en_fondo = (j->y >= SCREEN_HEIGHT - JUGADOR_HITBOX/2 - 5);
    
    j->en_suelo = (en_isla || en_fondo);
    
    if (j->en_suelo && j->velocidad_y > 0) {
        j->velocidad_y = 0;
        j->estado = ESTADO_SUELO;
        j->puede_saltar = true;
        
        if (en_isla) {
            printf("DEBUG: Aterrizo en isla\n");
        } else {
            printf("DEBUG: Toco el fondo de la pantalla\n");
        }
    }
    
    if (!j->en_suelo && j->estado == ESTADO_SUELO) {
        j->estado = ESTADO_CAYENDO;
        printf("DEBUG: Dejo de estar en suelo. Cambiando a CAYENDO\n");
    }
}

// ==================== FUNCION PRINCIPAL ====================
void aplicar_movimiento(EstadoJuego *estado, Controles *ctrl) {
    Jugador *j = &estado->jugador;
    
    printf("=== FRAME: Estado=%d, Pos=(%.1f,%.1f) ===\n", j->estado, j->x, j->y);
    
    j->en_suelo = false;
    
    switch (j->estado) {
        case ESTADO_SUELO:
            printf("Ejecutando ESTADO_SUELO\n");
            manejar_estado_suelo(estado, ctrl);
            break;
        case ESTADO_SALTANDO:
        case ESTADO_CAYENDO:
            printf("Ejecutando ESTADO_AEREO\n");
            manejar_estado_aereo(estado, ctrl);
            break;
        case ESTADO_AGARRADO_LIANA:
            printf("Ejecutando ESTADO_AGARRADO_LIANA\n");
            manejar_estado_liana(estado, ctrl);
            break;
    }
    
    aplicar_fisica_jugador(j);
    limitar_movimiento_jugador(j);
    verificar_aterrizaje(j, estado);
    
    printf("Estado final del frame: %d\n", j->estado);
}