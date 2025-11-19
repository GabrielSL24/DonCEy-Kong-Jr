#include "game.h"
#include "config.h"
#include <math.h>
#include <stdio.h>
#include "raymath.h"

void inicializar_juego(EstadoJuego *estado) {
    //printf("=== INICIALIZANDO JUEGO ===\n");
    
    // INICIALIZAR EL PADRE PRIMERO
    estado->padre = (Padre){
        (MATRIZ_COLUMNAS/2) * TAMANIO_CELDA + TAMANIO_CELDA/2,  // Centro de la pantalla
        2 * TAMANIO_CELDA + TAMANIO_CELDA/2,                    // Sobre plataforma superior
        true
    };
    //printf("Padre inicializado en: x=%.1f, y=%.1f\n", estado->padre.x, estado->padre.y);
    
    // Islas - POSICIONES FIJAS PARA DEBUG
    estado->num_islas = 3;
    estado->islas[0] = (Isla){100, 400, 200, COLOR_ISLA_VERDE, COLOR_ISLA_SUELO};
    estado->islas[1] = (Isla){300, 300, 150, COLOR_ISLA_VERDE, COLOR_ISLA_SUELO};
    estado->islas[2] = (Isla){500, 450, 180, COLOR_ISLA_VERDE, COLOR_ISLA_SUELO};
    
    //printf("Islas disponibles:\n");
    for (int i = 0; i < estado->num_islas; i++) {
        //printf("Isla %d: x=%.1f, y=%.1f, ancho=%.1f\n", 
               //i, estado->islas[i].x, estado->islas[i].y, estado->islas[i].ancho);
    }
    
    // Jugador - posición CORREGIDA sobre isla 1
    estado->jugador = (Jugador){
        estado->islas[1].x + estado->islas[1].ancho/2,  // 300 + 75 = 375
        estado->islas[1].y - JUGADOR_HITBOX/2 - 1,      // 300 - 6 - 1 = 293 (SOBRE la isla)
        0, 0, 3, 0, true, ESTADO_SUELO, -1, true, 0
    };
        
    //printf("Jugador inicializado en: x=%.1f, y=%.1f\n", 
           //estado->jugador.x, estado->jugador.y);
    
    // Plataforma superior
    estado->plataforma_superior = (Plataforma){0, 2 * TAMANIO_CELDA, SCREEN_WIDTH};
    
    // Lianas - posiciones basadas en columnas de matriz
    estado->num_lianas = 4;
    estado->lianas[0] = (Liana){4 * TAMANIO_CELDA, 2 * TAMANIO_CELDA, 13 * TAMANIO_CELDA};  // Columna 4
    estado->lianas[1] = (Liana){8 * TAMANIO_CELDA, 2 * TAMANIO_CELDA, 13 * TAMANIO_CELDA};  // Columna 8  
    estado->lianas[2] = (Liana){12 * TAMANIO_CELDA, 2 * TAMANIO_CELDA, 13 * TAMANIO_CELDA}; // Columna 12
    estado->lianas[3] = (Liana){16 * TAMANIO_CELDA, 2 * TAMANIO_CELDA, 13 * TAMANIO_CELDA}; // Columna 16
    
    // Frutas - posiciones en celdas específicas
    estado->num_frutas = 4;
    estado->frutas[0] = (Fruta){5 * TAMANIO_CELDA, 6 * TAMANIO_CELDA, 100, FRUTA_NARANJA, true};
    estado->frutas[1] = (Fruta){9 * TAMANIO_CELDA, 5 * TAMANIO_CELDA, 150, FRUTA_CELESTE, true};
    estado->frutas[2] = (Fruta){13 * TAMANIO_CELDA, 7 * TAMANIO_CELDA, 200, FRUTA_ROJA, true};
    estado->frutas[3] = (Fruta){15 * TAMANIO_CELDA, 9 * TAMANIO_CELDA, 120, FRUTA_BANANO, true};
    
    // Cocodrilos
    estado->num_cocodrilos = 3;
    estado->cocodrilos[0] = (Cocodrilo){6 * TAMANIO_CELDA, 9 * TAMANIO_CELDA, 0, true};
    estado->cocodrilos[1] = (Cocodrilo){10 * TAMANIO_CELDA, 7 * TAMANIO_CELDA, 1, true};
    estado->cocodrilos[2] = (Cocodrilo){14 * TAMANIO_CELDA, 10 * TAMANIO_CELDA, 0, true};
    
    estado->juego_activo = true;
    
    // Inicializar matriz
    inicializar_matriz(&estado->matriz, estado);

    // En inicializar_juego, deja SOLO estos:
    //printf("=== VERIFICACIÓN INICIAL ===\n");
    //printf("Isla 1: y=%.1f, Jugador: y=%.1f\n", estado->islas[1].y, estado->jugador.y);
    //printf("Jugador DEBERÍA estar sobre la isla: %s\n", 
        //(estado->jugador.y < estado->islas[1].y) ? "SÍ" : "NO");

}

// ==================== SISTEMA DE MATRIZ ====================

void coordenadas_a_matriz(float x, float y, int *fila, int *columna) {
    *columna = (int)(x / TAMANIO_CELDA);
    *fila = (int)(y / TAMANIO_CELDA);
    
    //printf("Coordenadas a matriz: (%.1f,%.1f) -> [%d,%d] (TAMANIO_CELDA=%d)\n", 
          // x, y, *fila, *columna, TAMANIO_CELDA);
    
    // Asegurar que esté dentro de los límites
    if (*fila < 0) *fila = 0;
    if (*fila >= MATRIZ_FILAS) *fila = MATRIZ_FILAS - 1;
    if (*columna < 0) *columna = 0;
    if (*columna >= MATRIZ_COLUMNAS) *columna = MATRIZ_COLUMNAS - 1;
    
    //printf("Coordenadas ajustadas: [%d,%d]\n", *fila, *columna);
}

void matriz_a_coordenadas(int fila, int columna, float *x, float *y) {
    *x = columna * TAMANIO_CELDA + TAMANIO_CELDA / 2;
    *y = fila * TAMANIO_CELDA + TAMANIO_CELDA / 2;
}

bool celda_es_solida(int tipo_celda) {
    // TEMPORAL: Casi todo es sólido excepto vacío, agua y lianas
    return (tipo_celda != TIPO_VACIO && 
            tipo_celda != TIPO_AGUA && 
            tipo_celda != TIPO_LIANA);
}

bool celda_es_liana(int tipo_celda) {
    return (tipo_celda == TIPO_LIANA);
}

bool celda_es_mortal(int tipo_celda) {
    return (tipo_celda == TIPO_AGUA || tipo_celda == TIPO_COCODRILO);
}

void inicializar_matriz(MatrizJuego *matriz, EstadoJuego *estado) {
    //printf("=== INICIALIZANDO MATRIZ ===\n");
    //printf("MATRIZ_FILAS=%d, MATRIZ_COLUMNAS=%d, TAMANIO_CELDA=%d\n", 
           //MATRIZ_FILAS, MATRIZ_COLUMNAS, TAMANIO_CELDA);
    
    // Limpiar matriz
    for (int f = 0; f < MATRIZ_FILAS; f++) {
        for (int c = 0; c < MATRIZ_COLUMNAS; c++) {
            matriz->celdas[f][c].tipo = TIPO_VACIO;
            matriz->celdas[f][c].datos_extra = 0;
        }
    }
    
    // Agua en las 2 últimas filas
    for (int f = MATRIZ_FILAS - 2; f < MATRIZ_FILAS; f++) {
        for (int c = 0; c < MATRIZ_COLUMNAS; c++) {
            matriz->celdas[f][c].tipo = TIPO_AGUA;
        }
    }
    
    // Plataforma superior (fila 1)
    for (int c = 0; c < MATRIZ_COLUMNAS; c++) {
        matriz->celdas[1][c].tipo = TIPO_PLATAFORMA;
    }
    
    // Padre en plataforma superior
    int f_padre, c_padre;
    coordenadas_a_matriz(estado->padre.x, estado->padre.y, &f_padre, &c_padre);
    if (f_padre >= 0 && f_padre < MATRIZ_FILAS && c_padre >= 0 && c_padre < MATRIZ_COLUMNAS) {
        matriz->celdas[f_padre][c_padre].tipo = TIPO_PADRE;
    }
    
    // Islas - CON DEBUG CRÍTICO
    //printf("=== COLOCANDO ISLAS EN MATRIZ ===\n");
    for (int i = 0; i < estado->num_islas; i++) {
        int fila, col_inicio, col_fin;
        coordenadas_a_matriz(estado->islas[i].x, estado->islas[i].y, &fila, &col_inicio);
        coordenadas_a_matriz(estado->islas[i].x + estado->islas[i].ancho, estado->islas[i].y, &fila, &col_fin);
        
        //printf("Isla %d: coord(%.1f,%.1f) ancho=%.1f -> fila=%d, columnas=%d-%d\n",
               //i, estado->islas[i].x, estado->islas[i].y, estado->islas[i].ancho,
               //fila, col_inicio, col_fin);
        
        for (int c = col_inicio; c <= col_fin && c < MATRIZ_COLUMNAS; c++) {
            if (c >= 0 && fila >= 0 && fila < MATRIZ_FILAS) {
                matriz->celdas[fila][c].tipo = TIPO_PLATAFORMA;
                //printf("  - Plataforma en [%d,%d]\n", fila, c);
            }
        }
    }
    
    // Lianas
    for (int i = 0; i < estado->num_lianas; i++) {
        int fila_inicio, col, fila_fin;
        coordenadas_a_matriz(estado->lianas[i].x, estado->lianas[i].y_inicio, &fila_inicio, &col);
        coordenadas_a_matriz(estado->lianas[i].x, estado->lianas[i].y_fin, &fila_fin, &col);
        
        for (int f = fila_inicio; f <= fila_fin && f < MATRIZ_FILAS; f++) {
            if (f >= 0 && col >= 0 && col < MATRIZ_COLUMNAS) {
                matriz->celdas[f][col].tipo = TIPO_LIANA;
            }
        }
    }
    
    // Frutas
    for (int i = 0; i < estado->num_frutas; i++) {
        if (estado->frutas[i].activo) {
            int fila, col;
            coordenadas_a_matriz(estado->frutas[i].x, estado->frutas[i].y, &fila, &col);
            if (fila >= 0 && fila < MATRIZ_FILAS && col >= 0 && col < MATRIZ_COLUMNAS) {
                matriz->celdas[fila][col].tipo = TIPO_FRUTA;
                matriz->celdas[fila][col].datos_extra = estado->frutas[i].tipo;
            }
        }
    }
    
    // Cocodrilos
    for (int i = 0; i < estado->num_cocodrilos; i++) {
        if (estado->cocodrilos[i].activo) {
            int fila, col;
            coordenadas_a_matriz(estado->cocodrilos[i].x, estado->cocodrilos[i].y, &fila, &col);
            if (fila >= 0 && fila < MATRIZ_FILAS && col >= 0 && col < MATRIZ_COLUMNAS) {
                matriz->celdas[fila][col].tipo = TIPO_COCODRILO;
                matriz->celdas[fila][col].datos_extra = estado->cocodrilos[i].tipo;
            }
        }
    }
    
    // Jugador - DECLARAR fila_jug y col_jug UNA SOLA VEZ
    int fila_jug, col_jug;
    coordenadas_a_matriz(estado->jugador.x, estado->jugador.y, &fila_jug, &col_jug);
    if (fila_jug >= 0 && fila_jug < MATRIZ_FILAS && col_jug >= 0 && col_jug < MATRIZ_COLUMNAS) {
        matriz->celdas[fila_jug][col_jug].tipo = TIPO_JUGADOR;
    }

    //printf("=== MATRIZ INICIALIZADA ===\n");
    
    // DEBUG: Mostrar estado de la matriz alrededor de donde debería estar el jugador
    //printf("ESTADO DE LA MATRIZ alrededor de jugador:\n");
    //printf("Jugador en matriz: [%d,%d]\n", fila_jug, col_jug); // <-- USA LAS VARIABLES YA DECLARADAS
    
    for (int f = fila_jug - 1; f <= fila_jug + 2; f++) {
        if (f < 0 || f >= MATRIZ_FILAS) continue;
        //printf("Fila %2d: ", f);
        for (int c = col_jug - 3; c <= col_jug + 3; c++) {
            if (c < 0 || c >= MATRIZ_COLUMNAS) {
                //printf("X");
            } else {
                //printf("%d", matriz->celdas[f][c].tipo);
            }
        }
        //printf("\n");
    }
} // <--- LLAVE DE CIERRE CORRECTA


void actualizar_matriz_desde_estado(EstadoJuego *estado) {
    MatrizJuego *matriz = &estado->matriz;
    
    // Limpiar posición anterior del jugador
    for (int f = 0; f < MATRIZ_FILAS; f++) {
        for (int c = 0; c < MATRIZ_COLUMNAS; c++) {
            if (matriz->celdas[f][c].tipo == TIPO_JUGADOR) {
                matriz->celdas[f][c].tipo = TIPO_VACIO;
            }
        }
    }
    
    // Actualizar posición del jugador
    int f_jug, c_jug;
    coordenadas_a_matriz(estado->jugador.x, estado->jugador.y, &f_jug, &c_jug);
    if (f_jug >= 0 && f_jug < MATRIZ_FILAS && c_jug >= 0 && c_jug < MATRIZ_COLUMNAS) {
        matriz->celdas[f_jug][c_jug].tipo = TIPO_JUGADOR;
    }
    
    // Actualizar estado de frutas
    for (int i = 0; i < estado->num_frutas; i++) {
        if (!estado->frutas[i].activo) {
            int f_fruta, c_fruta;
            coordenadas_a_matriz(estado->frutas[i].x, estado->frutas[i].y, &f_fruta, &c_fruta);
            if (f_fruta >= 0 && f_fruta < MATRIZ_FILAS && c_fruta >= 0 && c_fruta < MATRIZ_COLUMNAS) {
                if (matriz->celdas[f_fruta][c_fruta].tipo == TIPO_FRUTA) {
                    matriz->celdas[f_fruta][c_fruta].tipo = TIPO_VACIO;
                }
            }
        }
    }
}

// ==================== COLISIONES POR MATRIZ ====================

bool esta_en_superficie_matriz(Jugador *jugador, MatrizJuego *matriz) {
    //printf("=== VERIFICACIÓN DE SUELO URGENTE ===\n");
    
    // Posición exacta del jugador
    float base_jugador = jugador->y + JUGADOR_HITBOX/2;
    //printf("Jugador: (%.1f, %.1f) base=%.1f\n", jugador->x, jugador->y, base_jugador);
    
    // Convertir a matriz
    int fila_base, col_actual;
    coordenadas_a_matriz(jugador->x, base_jugador, &fila_base, &col_actual);
    //printf("En matriz: [%d,%d]\n", fila_base, col_actual);
    
    // Verificar fila debajo
    int fila_debajo = fila_base + 1;
   // printf("Buscando plataformas en fila: %d\n", fila_debajo);
    
    if (fila_debajo >= MATRIZ_FILAS) {
        //printf("ERROR: Fuera de matriz!\n");
        return false;
    }
    
    // Verificar TODA la fila debajo
    //printf("Fila %d completa: ", fila_debajo);
    for (int c = 0; c < MATRIZ_COLUMNAS; c++) {
       // printf("%d", matriz->celdas[fila_debajo][c].tipo);
    }
    printf("\n");
    
    // Verificar específicamente alrededor del jugador
    bool encontro_plataforma = false;
    for (int col_offset = -2; col_offset <= 2; col_offset++) {
        int col_check = col_actual + col_offset;
        if (col_check < 0 || col_check >= MATRIZ_COLUMNAS) continue;
        
        int tipo = matriz->celdas[fila_debajo][col_check].tipo;
        //printf("Celda [%d,%d]: tipo=%d, solida=%d\n", fila_debajo, col_check, tipo, celda_es_solida(tipo));
        
        if (celda_es_solida(tipo)) {
            //printf("¡PLATAFORMA ENCONTRADA!\n");
            encontro_plataforma = true;
            
            float y_plataforma = fila_debajo * TAMANIO_CELDA;
            float distancia = base_jugador - y_plataforma;
            //printf("Distancia a plataforma: %.1f\n", distancia);
            
            if (distancia >= 0 && distancia < 15.0f) {
                jugador->y = y_plataforma - JUGADOR_HITBOX/2 - 1;
                //printf("AJUSTADO: y=%.1f\n", jugador->y);
                return true;
            }
        }
    }
    
    //printf("RESULTADO: %s\n", encontro_plataforma ? "Plataforma lejana" : "NO HAY PLATAFORMA");
    return false;
}

int liana_mas_cercana_matriz(Jugador *jugador, MatrizJuego *matriz) {
    int fila_actual, col_actual;
    coordenadas_a_matriz(jugador->x, jugador->y, &fila_actual, &col_actual);
    
    //printf("Buscando lianas alrededor de: [%d,%d] pos=(%.1f,%.1f)\n", 
           //fila_actual, col_actual, jugador->x, jugador->y);
    
    // Buscar en un área MUY amplia
    for (int f_offset = -2; f_offset <= 2; f_offset++) {
        for (int col_offset = -3; col_offset <= 3; col_offset++) {
            int fila_check = fila_actual + f_offset;
            int col_check = col_actual + col_offset;
            
            if (fila_check < 0 || fila_check >= MATRIZ_FILAS || 
                col_check < 0 || col_check >= MATRIZ_COLUMNAS) {
                continue;
            }
            
            if (matriz->celdas[fila_check][col_check].tipo == TIPO_LIANA) {
                // Calcular posición exacta de la liana
                float x_liana = col_check * TAMANIO_CELDA + TAMANIO_CELDA/2;
                
                float distancia_horizontal = fabs(jugador->x - x_liana);
                
                //printf("Liana en [%d,%d] - distH=%.1f (límite=%.1f)\n", 
                       //fila_check, col_check, distancia_horizontal, DISTANCIA_AGARRE_LIANA);
                
                // Permitir agarre MUY flexible - solo verificar horizontal
                if (distancia_horizontal <= DISTANCIA_AGARRE_LIANA) {
                    //printf("¡Liana SUFICIENTEMENTE CERCANA! Columna: %d\n", col_check);
                    return col_check;
                } else {
                    //printf("Liana demasiado lejana\n");
                }
            }
        }
    }
    
    //printf("No se encontraron lianas en el área de búsqueda\n");
    return -1;
}

bool esta_sobre_isla(Jugador *jugador, Isla islas[], int num_islas) {
    float base_jugador = jugador->y + JUGADOR_HITBOX/2;
    
    for (int i = 0; i < num_islas; i++) {
        // Verificar si está horizontalmente sobre la isla
        bool sobre_isla_x = (jugador->x >= islas[i].x && 
                            jugador->x <= islas[i].x + islas[i].ancho);
        
        // Verificar si está justo encima de la isla (con un margen)
        bool sobre_isla_y = (base_jugador >= islas[i].y - 5 && 
                            base_jugador <= islas[i].y + 5);
        
        if (sobre_isla_x && sobre_isla_y) {
            //printf("DEBUG: ¡En isla %d! Ajustando posición...\n", i);
            // Ajustar para estar exactamente sobre la isla
            jugador->y = islas[i].y - JUGADOR_HITBOX/2 - 1;
            return true;
        }
    }
    return false;
}

// Función auxiliar para calcular distancia (si no quieres usar raymath.h)
float calcular_distancia(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

void verificar_colisiones_matriz(EstadoJuego *estado) {
    Jugador *j = &estado->jugador;
    MatrizJuego *matriz = &estado->matriz;
    
    // Verificar área del jugador (no solo punto central)
    for (int f_offset = -1; f_offset <= 1; f_offset++) {
        for (int c_offset = -1; c_offset <= 1; c_offset++) {
            float check_x = j->x + (c_offset * (JUGADOR_HITBOX/3));
            float check_y = j->y + (f_offset * (JUGADOR_HITBOX/3));
            
            int fila_check, col_check;
            coordenadas_a_matriz(check_x, check_y, &fila_check, &col_check);
            
            if (fila_check < 0 || fila_check >= MATRIZ_FILAS || 
                col_check < 0 || col_check >= MATRIZ_COLUMNAS) {
                continue;
            }
            
            int tipo_celda = matriz->celdas[fila_check][col_check].tipo;
            
            // Celda mortal
            if (celda_es_mortal(tipo_celda)) {
                //printf("¡Colisión mortal! Tipo: %d en fila:%d,col:%d\n", 
                       //tipo_celda, fila_check, col_check);
                
                estado->jugador.vidas--;
                if (estado->jugador.vidas > 0) {
                    //printf("¡Perdiste una vida! Vidas restantes: %d\n", estado->jugador.vidas);
                    // Respawn en isla segura
                    j->x = estado->islas[1].x + estado->islas[1].ancho/2;
                    j->y = estado->islas[1].y - 10;
                    j->velocidad_x = 0;
                    j->velocidad_y = 0;
                    j->estado = ESTADO_SUELO;
                    actualizar_matriz_desde_estado(estado);
                } else {
                    estado->juego_activo = false;
                    printf("¡Game Over Final!\n");
                }
                return;
            }
            
            // Fruta - solo si está suficientemente cerca
            if (tipo_celda == TIPO_FRUTA) {
                // Buscar la fruta correspondiente
                for (int i = 0; i < estado->num_frutas; i++) {
                    if (!estado->frutas[i].activo) continue;
                    
                    int f_fruta, c_fruta;
                    coordenadas_a_matriz(estado->frutas[i].x, estado->frutas[i].y, &f_fruta, &c_fruta);
                    
                    if (f_fruta == fila_check && c_fruta == col_check) {
                        // Calcular distancia sin Vector2Distance
                        float dx = j->x - estado->frutas[i].x;
                        float dy = j->y - estado->frutas[i].y;
                        float distancia = sqrtf(dx * dx + dy * dy);
                        
                        if (distancia < (JUGADOR_HITBOX + FRUTA_HITBOX) / 2) {
                            j->puntuacion += estado->frutas[i].puntos;
                            estado->frutas[i].activo = false;
                            matriz->celdas[fila_check][col_check].tipo = TIPO_VACIO;
                            //printf("¡Fruta tomada! +%d puntos. Total: %d\n", 
                                   //estado->frutas[i].puntos, j->puntuacion);
                        }
                        break;
                    }
                }
            }
            
            // Victoria
            if (tipo_celda == TIPO_PADRE) {
                // Calcular distancia sin Vector2Distance
                float dx = j->x - estado->padre.x;
                float dy = j->y - estado->padre.y;
                float distancia = sqrtf(dx * dx + dy * dy);
                
                if (distancia < (JUGADOR_HITBOX + 20) / 2) {
                    estado->juego_activo = false;
                    //printf("¡Victoria! Rescataste a Donkey Kong. Puntuación final: %d\n", j->puntuacion);
                    return;
                }
            }
        }
    }
}

// ==================== ACTUALIZACIÓN DESDE SERVIDOR ====================

void actualizar_posicion_desde_matriz(EstadoJuego *estado) {
    MatrizJuego *matriz = &estado->matriz;
    
    // Buscar al jugador en la matriz
    for (int f = 0; f < MATRIZ_FILAS; f++) {
        for (int c = 0; c < MATRIZ_COLUMNAS; c++) {
            if (matriz->celdas[f][c].tipo == TIPO_JUGADOR) {
                // Convertir coordenadas de matriz a píxeles
                float x = c * TAMANIO_CELDA + TAMANIO_CELDA / 2;
                float y = f * TAMANIO_CELDA + TAMANIO_CELDA / 2;
                
                // Actualizar posición del jugador
                estado->jugador.x = x;
                estado->jugador.y = y;
                
                printf("Posición actualizada desde matriz: [%d][%d] -> (%.1f, %.1f)\n", 
                       f, c, x, y);
                return;
            }
        }
    }
    
    printf("ADVERTENCIA: Jugador no encontrado en matriz\n");
}

// ==================== FUNCIONES OBSOLETAS (ELIMINAR) ====================

// ESTAS FUNCIONES YA NO SE USAN - SE MANTIENEN TEMPORALMENTE PARA COMPILACIÓN
bool esta_en_superficie(Jugador *jugador, EstadoJuego *estado) {
    return esta_en_superficie_matriz(jugador, &estado->matriz);
}

int liana_mas_cercana(Jugador *jugador, Liana lianas[], int num_lianas) {
    return liana_mas_cercana_matriz(jugador, &((EstadoJuego*)0)->matriz); // Placeholder
}

bool esta_en_isla(Jugador *jugador, Isla islas[], int num_islas) {
    return false; // Obsoleta
}

int encontrar_isla_para_respawn(Isla islas[], int num_islas) {
    return 0; // Obsoleta
}

void verificar_colisiones(EstadoJuego *estado) {
    verificar_colisiones_matriz(estado); // Redirigir a nueva función
}