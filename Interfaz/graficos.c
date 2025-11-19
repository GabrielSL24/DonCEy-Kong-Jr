#include "raylib.h"
#include "graficos.h"
#include "config.h"
#include "game.h"


void inicializar_graficos(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "DonCEy Kong Jr");
    SetTargetFPS(FPS);
}

void cerrar_graficos(void) {
    CloseWindow();
}

void dibujar_jugador(Jugador *j) {
    if (j->activo) {
        // CAMBIAR: Usar JUGADOR_HITBOX para ser consistente con la lógica
        DrawRectangle(j->x - JUGADOR_HITBOX/2, j->y - JUGADOR_HITBOX/2, 
                     JUGADOR_HITBOX, JUGADOR_HITBOX, COLOR_JUGADOR);
    }
}

void dibujar_cocodrilo(Cocodrilo *c) {
    if (c->activo) {
        Color color = (c->tipo == 0) ? COLOR_COCODRILO_ROJO : COLOR_COCODRILO_AZUL;
        // CAMBIAR: Usar COCODRILO_HITBOX
        DrawRectangle(c->x - COCODRILO_HITBOX/2, c->y - COCODRILO_HITBOX/2,
                     COCODRILO_HITBOX, COCODRILO_HITBOX, color);
    }
}

void dibujar_fruta(Fruta *f) {
    if (f->activo) {
        Color color_fruta;
        
        switch (f->tipo) {
            case FRUTA_NARANJA:
                color_fruta = COLOR_NARANJA;
                break;
            case FRUTA_CELESTE:
                color_fruta = COLOR_FRUTA_CELESTE;
                break;
            case FRUTA_ROJA:
                color_fruta = COLOR_FRUTA_ROJA;
                break;
            case FRUTA_BANANO:
                color_fruta = COLOR_BANANO;
                break;
            default:
                color_fruta = GREEN;
        }
        
        // CAMBIAR: Usar FRUTA_HITBOX
        DrawRectangle(f->x - FRUTA_HITBOX/2, f->y - FRUTA_HITBOX/2,
                     FRUTA_HITBOX, FRUTA_HITBOX, color_fruta);
        
        // Opcional: pequeño detalle para diferenciar
        DrawRectangle(f->x - FRUTA_HITBOX/4, f->y - FRUTA_HITBOX/4,
                     FRUTA_HITBOX/2, FRUTA_HITBOX/2, (Color){color_fruta.r/2, color_fruta.g/2, color_fruta.b/2, 255});
    }
}

void dibujar_lianas(Liana lianas[], int count) {
    for (int i = 0; i < count; i++) {
        DrawRectangle(lianas[i].x - LIANA_WIDTH/2, lianas[i].y_inicio,
                     LIANA_WIDTH, lianas[i].y_fin - lianas[i].y_inicio, COLOR_LIANA);
    }
}


void dibujar_ui(EstadoJuego *estado) {
    DrawText(TextFormat("Vidas: %d", estado->jugador.vidas), 10, 10, 20, DARKGRAY);
    DrawText(TextFormat("Puntos: %d", estado->jugador.puntuacion), 10, 40, 20, DARKGRAY);
    
    if (!estado->juego_activo) {
        DrawText("GAME OVER", SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 - 20, 40, RED);
    }
}
void dibujar_agua(void) {
    DrawRectangle(0, SCREEN_HEIGHT - AGUA_HEIGHT, SCREEN_WIDTH, AGUA_HEIGHT, COLOR_AGUA);
}

void dibujar_plataforma_superior(Plataforma *p) {
    DrawRectangle(p->x, p->y, p->ancho, PLATAFORMA_HEIGHT, COLOR_PLATAFORMA);
}

void dibujar_padre(Padre *p) {
    if (p->activo) {
        DrawRectangle(p->x - JUGADOR_SIZE/2, p->y - JUGADOR_SIZE/2, 
                     JUGADOR_SIZE, JUGADOR_SIZE, COLOR_PADRE);
    }
}

void dibujar_isla(Isla *isla) {
    // PROBLEMA: En la lógica la isla tiene altura 0 (solo posición Y), pero aquí dibujas 20px de altura
    // CAMBIAR: Dibujar solo la superficie (parte verde) donde el jugador puede pararse
    // La parte café es solo decorativa debajo
    
    // Parte superior verde (donde el jugador se para)
    DrawRectangle(isla->x, isla->y - 8, isla->ancho, 8, isla->color_superior);
    // Parte inferior café (decoración)
    DrawRectangle(isla->x, isla->y, isla->ancho, 5, isla->color_inferior);
}

void dibujar_mario(void) {
    // Mario temporal - azul con detalle rojo
    DrawRectangle(SCREEN_WIDTH/2 - 15, 40, 30, 20, COLOR_MARIO_BASE);
    DrawRectangle(SCREEN_WIDTH/2 - 5, 45, 10, 10, COLOR_MARIO_DETALLE);
}

void dibujar_escena(EstadoJuego *estado) {
    BeginDrawing();
    ClearBackground(COLOR_FONDO);
    
    // Dibujar elementos en orden
    dibujar_agua();
    
    // Islas (primero las bases)
    for (int i = 0; i < estado->num_islas; i++) {
        dibujar_isla(&estado->islas[i]);
    }
    
    // Lianas (sobre las islas)
    for (int i = 0; i < estado->num_lianas; i++) {
        DrawRectangle(estado->lianas[i].x - LIANA_WIDTH/2, estado->lianas[i].y_inicio,
                     LIANA_WIDTH, estado->lianas[i].y_fin - estado->lianas[i].y_inicio, COLOR_LIANA);
    }
    
    // Plataforma superior y personajes
    dibujar_plataforma_superior(&estado->plataforma_superior);
    dibujar_mario();
    dibujar_padre(&estado->padre);
    
    // Elementos del juego
    for (int i = 0; i < estado->num_frutas; i++) {
        dibujar_fruta(&estado->frutas[i]);
    }
    
    for (int i = 0; i < estado->num_cocodrilos; i++) {
        dibujar_cocodrilo(&estado->cocodrilos[i]);
    }
    
    dibujar_jugador(&estado->jugador);
    //dibujar_ui(estado);
    
    // DEBUG: Descomenta la siguiente línea para ver hitboxes y grid
    // dibujar_debug(estado);
    
    EndDrawing();
}

void dibujar_debug(EstadoJuego *estado) {
    // Dibujar hitboxes y puntos de colisión (solo para debug)
    #ifdef DEBUG
    // Hitbox del jugador
    DrawRectangleLines(estado->jugador.x - JUGADOR_HITBOX/2, 
                      estado->jugador.y - JUGADOR_HITBOX/2,
                      JUGADOR_HITBOX, JUGADOR_HITBOX, RED);
    
    // Punto central del jugador
    DrawCircle(estado->jugador.x, estado->jugador.y, 2, BLUE);
    
    // Dibujar matriz de colisiones
    for (int f = 0; f < MATRIZ_FILAS; f++) {
        for (int c = 0; c < MATRIZ_COLUMNAS; c++) {
            int x = c * TAMANIO_CELDA;
            int y = f * TAMANIO_CELDA;
            
            if (estado->matriz.celdas[f][c].tipo != TIPO_VACIO) {
                Color grid_color;
                switch (estado->matriz.celdas[f][c].tipo) {
                    case TIPO_PLATAFORMA: grid_color = (Color){255, 255, 0, 80}; break; // Amarillo transparente
                    case TIPO_LIANA: grid_color = (Color){255, 255, 255, 80}; break;    // Blanco transparente
                    case TIPO_AGUA: grid_color = (Color){0, 0, 255, 80}; break;         // Azul transparente
                    case TIPO_COCODRILO: grid_color = (Color){255, 0, 0, 80}; break;    // Rojo transparente
                    case TIPO_FRUTA: grid_color = (Color){0, 255, 0, 80}; break;        // Verde transparente
                    default: grid_color = (Color){128, 128, 128, 50};
                }
                DrawRectangle(x, y, TAMANIO_CELDA, TAMANIO_CELDA, grid_color);
            }
            
            // Líneas de la grid
            DrawRectangleLines(x, y, TAMANIO_CELDA, TAMANIO_CELDA, (Color){50, 50, 50, 50});
        }
    }
    #endif
}