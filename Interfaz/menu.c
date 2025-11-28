#include "menu.h"
#include "graficos.h"
#include "conexion.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// ==================== ACTUALIZACIÓN DE MENÚ PRINCIPAL ====================

void actualizar_menu_principal(FrameInputs* inputs, EstadoMenu* estado_menu, int* seleccion_actual) {
    if (inputs->enter_pressed) {
        if (*seleccion_actual == 0) {
            // JUGADOR - Crear nueva partida con handshake
            char game_id[50];
            snprintf(game_id, sizeof(game_id), "partida_%lld", (long long)time(NULL));
            
            if (servidor_conectado && crear_nueva_partida(game_id)) {
                strcpy(partida_seleccionada_global, game_id);
                *estado_menu = MENU_CREATING_GAME;  // ← NUEVO estado de espera
                printf("⏳ Creando partida: %s - Esperando confirmación...\n", game_id);
            }
        } else if (*seleccion_actual == 1) {
            // ESPECTADOR - Solicitar lista
            *estado_menu = MENU_SELECT_GAME;
            if (servidor_conectado) {
                solicitar_lista_partidas();
            }
        }
    }
    
    // Navegación del menú
    if (inputs->seleccion_menu != 0) {
        *seleccion_actual += inputs->seleccion_menu;
        if (*seleccion_actual < 0) *seleccion_actual = 1;
        if (*seleccion_actual > 1) *seleccion_actual = 0;
    }
}

// ==================== ACTUALIZACIÓN DE SELECCIÓN DE PARTIDA ====================

void actualizar_seleccion_partida(FrameInputs* inputs, EstadoMenu* estado_menu, int* seleccion_actual) {
    if (inputs->escape_pressed) {
        *estado_menu = MENU_MAIN;
        return;
    }
    
    // Procesar respuesta del servidor (lista de partidas)
    if (cantidad_partidas == 0) {
        procesar_respuesta_servidor(NULL); // Intentar obtener lista
    }
    
    // Navegación y selección...
    if (inputs->enter_pressed && cantidad_partidas > 0) {
        if (unirse_partida_espectador(partidas_disponibles[*seleccion_actual].game_id)) {
            strcpy(partida_seleccionada_global, partidas_disponibles[*seleccion_actual].game_id);
            *estado_menu = MENU_JOINING_GAME;  // ← NUEVO estado de espera
            printf("⏳ Uniéndose como espectador...\n");
        }
    }
}

// ==================== ACTUALIZACIÓN MODO JUGADOR ====================

void actualizar_modo_jugador(FrameInputs* inputs, EstadoMenu* estado_menu, EstadoJuego* estado_juego) {
    // Enviar inputs al servidor
    if (servidor_conectado && inputs->num_inputs > 0) {
        for (int i = 0; i < inputs->num_inputs; i++) {
            const char* input_str = inputs->inputs[i];
            
            // ✅ SIMPLIFICADO: El string ya viene con _PRESSED/_RELEASED
            if (strstr(input_str, "_RELEASED") != NULL) {
                // Extraer solo la parte del key (sin _RELEASED)
                char key[20];
                strncpy(key, input_str, strlen(input_str) - 9);
                key[strlen(input_str) - 9] = '\0';
                
                enviar_input_al_servidor(
                    CLIENT_PLAYER, 
                    partida_seleccionada_global,
                    "KEY_RELEASED",  // ← tipo fijo
                    key              // ← ej: "LEFT", "RIGHT", etc.
                );
            } 
            else if (strstr(input_str, "_PRESSED") != NULL) {
                // Extraer solo la parte del key (sin _PRESSED)
                char key[20];
                strncpy(key, input_str, strlen(input_str) - 8);
                key[strlen(input_str) - 8] = '\0';
                
                enviar_input_al_servidor(
                    CLIENT_PLAYER, 
                    partida_seleccionada_global,
                    "KEY_PRESSED",   // ← tipo fijo  
                    key              // ← ej: "LEFT", "RIGHT", etc.
                );
            }
        }
    }
    
    // Procesar respuesta del servidor
    if (servidor_conectado) {
        procesar_respuesta_servidor(estado_juego);
    }
    
    // Volver al menú si se presiona ESC
    if (inputs->escape_pressed) {
        salir_partida_jugador(partida_seleccionada_global);
        set_partida_activa(false);
        *estado_menu = MENU_MAIN;
        printf("=== VOLVIENDO AL MENÚ DESDE JUEGO ===\n");
    }
}

// ==================== ACTUALIZACIÓN MODO ESPECTADOR ====================

void actualizar_modo_espectador(FrameInputs* inputs, EstadoMenu* estado_menu, EstadoJuego* estado_juego) {
    // CAMBIO: usar procesar_respuesta_servidor en lugar de recibir_estado_actualizado
    if (servidor_conectado) {
        procesar_respuesta_servidor(estado_juego); // ← CAMBIADO
    }
    
    // Volver al menú si se presiona ESC
    if (inputs->escape_pressed) {
        salir_partida_espectador(partida_seleccionada_global);
        set_partida_activa(false); // ← AGREGAR
        *estado_menu = MENU_MAIN;
        printf("=== VOLVIENDO AL MENÚ DESDE ESPECTADOR ===\n");
    }
}

void actualizar_estado_espera(FrameInputs* inputs, EstadoMenu* estado_menu, EstadoJuego* estado_juego) {
    // Intentar procesar respuesta del servidor
    if (procesar_respuesta_servidor(estado_juego)) {
        // Si recibimos confirmación, cambiar al estado de juego
        if (*estado_menu == MENU_CREATING_GAME) {
            *estado_menu = MENU_PLAYING;
            printf("✅ ¡Partida como JUGADOR iniciada!\n");
        } else if (*estado_menu == MENU_JOINING_GAME) {
            *estado_menu = MENU_SPECTATING;
            printf("✅ ¡Partida como ESPECTADOR iniciada!\n");
        }
    }
    
    // Permitir cancelar con ESC
    if (inputs->escape_pressed) {
        *estado_menu = MENU_MAIN;
        set_partida_activa(false); // ← AGREGAR
        printf("❌ Cancelando conexión...\n");
    }
}
// ==================== RENDERIZADO DE MENÚS ====================

void dibujar_menu_principal(int seleccion) {
    int centerX = GetScreenWidth() / 2;
    int centerY = GetScreenHeight() / 2;
    
    // Fondo semitransparente
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0, 0, 0, 200});
    
    // Título
    DrawText("DONKEY KONG JR", centerX - 150, centerY - 150, 40, YELLOW);
    
    // Opciones
    const char* opciones[] = {"JUGAR COMO JUGADOR", "OBSERVAR PARTIDA"};
    for (int i = 0; i < 2; i++) {
        Color color = (i == seleccion) ? GREEN : WHITE;
        DrawText(opciones[i], centerX - 120, centerY - 50 + i * 60, 30, color);
        
        if (i == seleccion) {
            DrawText(">", centerX - 150, centerY - 50 + i * 60, 30, GREEN);
        }
    }
    
    // Instrucciones
    DrawText("Usa ↑↓ para navegar, ENTER para seleccionar", centerX - 200, centerY + 100, 20, LIGHTGRAY);
}

void dibujar_seleccion_partida(int seleccion, InfoPartida partidas[], int count) {
    int centerX = GetScreenWidth() / 2;
    int centerY = GetScreenHeight() / 2;
    
    // Fondo semitransparente
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0, 0, 0, 200});
    
    // Título
    DrawText("SELECCIONAR PARTIDA", centerX - 150, centerY - 200, 30, YELLOW);
    
    if (count == 0) {
        DrawText("No hay partidas disponibles", centerX - 120, centerY, 25, RED);
        DrawText("Presiona ESC para volver", centerX - 100, centerY + 50, 20, LIGHTGRAY);
        return;
    }
    
    // Lista de partidas
    for (int i = 0; i < count; i++) {
        int y = centerY - 100 + i * 60;
        Color color = (i == seleccion) ? GREEN : WHITE;
        
        char info[100];
        snprintf(info, sizeof(info), "%s - Jugadores: %d - Espectadores: %d", 
                partidas[i].game_id, partidas[i].player_count, partidas[i].spectators);
        
        DrawText(info, centerX - 200, y, 20, color);
        
        if (i == seleccion) {
            DrawText(">", centerX - 230, y, 25, GREEN);
        }
    }
    
    // Instrucciones
    DrawText("Usa ↑↓ para navegar, ENTER para observar, ESC para volver", 
             centerX - 250, centerY + 150, 20, LIGHTGRAY);
}

void dibujar_hud_espectador(const char* partida_actual) {
    DrawRectangle(10, 10, 300, 60, (Color){0, 0, 0, 128});
    DrawText("MODO ESPECTADOR", 20, 15, 20, YELLOW);
    DrawText(TextFormat("Partida: %s", partida_actual), 20, 40, 15, WHITE);
    DrawText("ESC para salir", GetScreenWidth() - 150, 15, 20, LIGHTGRAY);
}

void dibujar_hud_jugador(const char* partida_actual) {
    DrawRectangle(10, 10, 250, 40, (Color){0, 0, 0, 128});
    DrawText("MODO JUGADOR", 20, 15, 20, GREEN);
    DrawText(TextFormat("Partida: %s", partida_actual), 20, 35, 15, WHITE);
    DrawText("ESC para menú", GetScreenWidth() - 150, 15, 20, LIGHTGRAY);
}