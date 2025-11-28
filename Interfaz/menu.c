#include "menu.h"
#include "graficos.h"
#include "conexion.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

// Variables para control de actualizacion de lista de partidas
static double ultimo_refresh_lista = 0;
static const double INTERVALO_REFRESH = 3.0; // Actualiza cada 3 segundos

// ==================== ACTUALIZACION DE MENU PRINCIPAL ====================

void actualizar_menu_principal(FrameInputs* inputs, EstadoMenu* estado_menu, int* seleccion_actual) {
    // Verificar si se presiono ENTER
    if (inputs->enter_pressed) {
        if (*seleccion_actual == 0) {
            // Opcion JUGADOR - Crear nueva partida
            char game_id[50];
            // Generar ID unico usando timestamp
            snprintf(game_id, sizeof(game_id), "partida_%lld", (long long)time(NULL));
            
            // Intentar crear partida en servidor
            if (servidor_conectado && crear_nueva_partida(game_id)) {
                strcpy(partida_seleccionada_global, game_id);
                *estado_menu = MENU_CREATING_GAME;
                printf("Creando partida: %s - Esperando confirmacion...\n", game_id);
            } else {
                printf("No se pudo crear partida - servidor no conectado\n");
            }
        } else if (*seleccion_actual == 1) {
            // Opcion ESPECTADOR - Ir a seleccion de partida
            *estado_menu = MENU_SELECT_GAME;
            if (servidor_conectado) {
                solicitar_lista_partidas();
                ultimo_refresh_lista = GetTime();
            }
            *seleccion_actual = 0; // Resetear seleccion para el siguiente menu
        }
    }
    
    // Navegacion del menu con flechas
    if (inputs->seleccion_menu != 0) {
        *seleccion_actual += inputs->seleccion_menu;
        // Mantener seleccion dentro de limites
        if (*seleccion_actual < 0) *seleccion_actual = 1;
        if (*seleccion_actual > 1) *seleccion_actual = 0;
    }
}

// ==================== ACTUALIZACION DE SELECCION DE PARTIDA ====================

void actualizar_seleccion_partida(FrameInputs* inputs, EstadoMenu* estado_menu, int* seleccion_actual) {
    // Volver al menu principal con ESC
    if (inputs->escape_pressed) {
        *estado_menu = MENU_MAIN;
        *seleccion_actual = 0;
        cantidad_partidas = 0; // Limpiar lista de partidas
        return;
    }
    
    // Actualizar lista automaticamente cada intervalo definido
    double tiempo_actual = GetTime();
    if (tiempo_actual - ultimo_refresh_lista >= INTERVALO_REFRESH) {
        if (servidor_conectado) {
            solicitar_lista_partidas();
            ultimo_refresh_lista = tiempo_actual;
            printf("Actualizando lista de partidas...\n");
        }
    }
    
    // Procesar respuesta del servidor (lista de partidas)
    procesar_respuesta_servidor(NULL);
    
    // Navegacion en lista de partidas
    if (inputs->seleccion_menu != 0 && cantidad_partidas > 0) {
        *seleccion_actual += inputs->seleccion_menu;
        // Circular por la lista
        if (*seleccion_actual < 0) *seleccion_actual = cantidad_partidas - 1;
        if (*seleccion_actual >= cantidad_partidas) *seleccion_actual = 0;
    }
    
    // Seleccionar partida con ENTER
    if (inputs->enter_pressed && cantidad_partidas > 0) {
        InfoPartida* partida_seleccionada = &partidas_disponibles[*seleccion_actual];
        
        // Solo unirse a partidas activas
        if (partida_seleccionada->active) {
            if (unirse_partida_espectador(partida_seleccionada->game_id)) {
                strcpy(partida_seleccionada_global, partida_seleccionada->game_id);
                *estado_menu = MENU_JOINING_GAME;
                printf("Uniendose como espectador a: %s\n", partida_seleccionada->game_id);
            }
        } else {
            printf("No se puede observar una partida inactiva\n");
        }
    }
    
    // Refresh manual con tecla R
    if (IsKeyPressed(KEY_R) && servidor_conectado) {
        solicitar_lista_partidas();
        ultimo_refresh_lista = tiempo_actual;
        printf("Lista actualizada manualmente\n");
    }
}

// ==================== ACTUALIZACION MODO JUGADOR ====================

void actualizar_modo_jugador(FrameInputs* inputs, EstadoMenu* estado_menu, EstadoJuego* estado_juego) {
    // Verificar que la conexion sigue activa
    if (!servidor_conectado) {
        printf("Servidor desconectado, volviendo al menu\n");
        *estado_menu = MENU_MAIN;
        return;
    }
    
    static int error_count = 0;
    
    // Procesar respuesta del servidor PRIMERO
    if (!procesar_respuesta_servidor(estado_juego)) {
        error_count++;
        // Verificar si hay demasiados errores consecutivos
        if (error_count > 10) {
            printf("Demasiados errores, verificando conexion...\n");
            if (!hay_datos_disponibles()) {
                printf("Conexion perdida, volviendo al menu\n");
                *estado_menu = MENU_MAIN;
                error_count = 0;
                return;
            }
        }
    } else {
        error_count = 0; // Resetear contador si procesamiento fue exitoso
    }
    
    // VERIFICAR GAME OVER (vidas = 0)
    if (estado_juego->jugador.vidas <= 0) {
        printf("GAME OVER - Sin vidas\n");
        salir_partida_jugador(partida_seleccionada_global);
        set_partida_activa(false);
        *estado_menu = MENU_MAIN;
        error_count = 0;
        return;
    }
    
    // VERIFICAR VICTORIA (llegar donde el padre)
    float distancia_al_padre = sqrt(
        pow(estado_juego->jugador.x - estado_juego->padre.x, 2) +
        pow(estado_juego->jugador.y - estado_juego->padre.y, 2)
    );
    
    const float DISTANCIA_VICTORIA = 30.0f; // Radio de victoria
    
    if (distancia_al_padre <= DISTANCIA_VICTORIA && estado_juego->padre.activo) {
        printf("VICTORIA - Llegaste donde tu padre\n");
        salir_partida_jugador(partida_seleccionada_global);
        set_partida_activa(false);
        *estado_menu = MENU_MAIN;
        error_count = 0;
        return;
    }
    
    // Enviar inputs al servidor
    if (inputs->num_inputs > 0) {
        for (int i = 0; i < inputs->num_inputs; i++) {
            const char* input_str = inputs->inputs[i];
            
            // Procesar tecla liberada
            if (strstr(input_str, "_RELEASED") != NULL) {
                char key[20];
                strncpy(key, input_str, strlen(input_str) - 9);
                key[strlen(input_str) - 9] = '\0';
                
                if (!enviar_input_al_servidor(CLIENT_PLAYER, partida_seleccionada_global,
                                             "KEY_RELEASED", key)) {
                    printf("Error enviando input\n");
                    *estado_menu = MENU_MAIN;
                    error_count = 0;
                    return;
                }
            } 
            // Procesar tecla presionada
            else if (strstr(input_str, "_PRESSED") != NULL) {
                char key[20];
                strncpy(key, input_str, strlen(input_str) - 8);
                key[strlen(input_str) - 8] = '\0';
                
                if (!enviar_input_al_servidor(CLIENT_PLAYER, partida_seleccionada_global,
                                             "KEY_PRESSED", key)) {
                    printf("Error enviando input\n");
                    *estado_menu = MENU_MAIN;
                    error_count = 0;
                    return;
                }
            }
        }
    }
    
    // Volver al menu con ESC
    if (inputs->escape_pressed) {
        printf("Saliendo de partida por ESC...\n");
        salir_partida_jugador(partida_seleccionada_global);
        set_partida_activa(false);
        *estado_menu = MENU_MAIN;
        error_count = 0;
    }
}

// ==================== ACTUALIZACION MODO ESPECTADOR ====================

void actualizar_modo_espectador(FrameInputs* inputs, EstadoMenu* estado_menu, EstadoJuego* estado_juego) {
    // Procesar actualizaciones del servidor si esta conectado
    if (servidor_conectado) {
        procesar_respuesta_servidor(estado_juego);
    }
    
    // Volver al menu con ESC
    if (inputs->escape_pressed) {
        salir_partida_espectador(partida_seleccionada_global);
        set_partida_activa(false);
        *estado_menu = MENU_MAIN;
        printf("VOLVIENDO AL MENU DESDE ESPECTADOR\n");
    }
}

// ==================== ACTUALIZACION ESTADO ESPERA ====================

void actualizar_estado_espera(FrameInputs* inputs, EstadoMenu* estado_menu, EstadoJuego* estado_juego) {
    static bool start_game_enviado = false;
    static int intentos = 0;
    static long ultimo_intento = 0;
    static long tiempo_inicio = 0;
    
    long ahora = GetTime() * 1000;
    
    // Inicializar tiempo de inicio
    if (tiempo_inicio == 0) tiempo_inicio = ahora;
    
    // Procesar respuesta del servidor
    if (procesar_respuesta_servidor(estado_juego)) {
        printf("Respuesta del servidor procesada en estado espera\n");
        
        // Si se esta creando partida y aun no se envio START_GAME
        if (*estado_menu == MENU_CREATING_GAME && !start_game_enviado) {
            printf("GAME_CREATED recibido, enviando START_GAME...\n");
            if (confirmar_inicio_partida(partida_seleccionada_global)) {
                start_game_enviado = true;
                intentos = 0;
                printf("START_GAME enviado, esperando GAME_STARTED...\n");
            }
        }
        // Si se esta uniendo como espectador
        else if (*estado_menu == MENU_JOINING_GAME) {
            *estado_menu = MENU_SPECTATING;
            tiempo_inicio = 0;
            printf("Unido como ESPECTADOR\n");
        }
    }
    
    // Reintentar envio de START_GAME si es necesario
    if (*estado_menu == MENU_CREATING_GAME && start_game_enviado && 
        !esta_en_partida_activa() && intentos < 3) {
        
        if (ahora - ultimo_intento > 1000) {
            printf("Reintentando START_GAME (intento %d)...\n", intentos + 1);
            confirmar_inicio_partida(partida_seleccionada_global);
            intentos++;
            ultimo_intento = ahora;
        }
    }
    
    // Transicion a juego cuando la partida esta activa
    if (start_game_enviado && esta_en_partida_activa()) {
        *estado_menu = MENU_PLAYING;
        start_game_enviado = false;
        intentos = 0;
        tiempo_inicio = 0;
        printf("Partida INICIADA como JUGADOR\n");
    }
    
    // Timeout de 8 segundos
    if (ahora - tiempo_inicio > 8000) {
        printf("Timeout esperando inicio de partida\n");
        *estado_menu = MENU_MAIN;
        set_partida_activa(false);
        start_game_enviado = false;
        intentos = 0;
        tiempo_inicio = 0;
    }
    
    // Cancelar con ESC
    if (inputs->escape_pressed) {
        *estado_menu = MENU_MAIN;
        set_partida_activa(false);
        start_game_enviado = false;
        intentos = 0;
        tiempo_inicio = 0;
        printf("Cancelando conexion...\n");
    }
}

// ==================== RENDERIZADO DE MENUS ====================

void dibujar_menu_principal(int seleccion) {
    int centerX = GetScreenWidth() / 2;
    int centerY = GetScreenHeight() / 2;
    
    // Fondo semitransparente
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0, 0, 0, 200});
    
    // Titulo
    DrawText("DONKEY KONG JR", centerX - 180, centerY - 150, 40, YELLOW);
    
    // Estado de conexion
    const char* estado_servidor = servidor_conectado ? "CONECTADO" : "DESCONECTADO";
    Color color_servidor = servidor_conectado ? GREEN : RED;
    DrawText(TextFormat("Servidor: %s", estado_servidor), centerX - 100, centerY - 90, 20, color_servidor);
    
    // Opciones del menu
    const char* opciones[] = {"JUGAR COMO JUGADOR", "OBSERVAR PARTIDA"};
    for (int i = 0; i < 2; i++) {
        Color color = (i == seleccion) ? GREEN : WHITE;
        DrawText(opciones[i], centerX - 140, centerY - 30 + i * 60, 30, color);
        
        // Indicador de seleccion
        if (i == seleccion) {
            DrawText(">", centerX - 170, centerY - 30 + i * 60, 30, GREEN);
        }
    }
    
    // Instrucciones
    DrawText("NAVEGAR | ENTER Seleccionar", centerX - 180, centerY + 120, 20, LIGHTGRAY);
}

void dibujar_seleccion_partida(int seleccion, InfoPartida partidas[], int count) {
    int centerX = GetScreenWidth() / 2;
    int centerY = GetScreenHeight() / 2;
    
    // Fondo
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0, 0, 0, 200});
    
    // Titulo
    DrawText("SELECCIONAR PARTIDA", centerX - 180, 50, 35, YELLOW);
    
    // Tiempo hasta proxima actualizacion
    double tiempo_restante = INTERVALO_REFRESH - (GetTime() - ultimo_refresh_lista);
    DrawText(TextFormat("Actualizacion en: %.1fs", tiempo_restante), 
             GetScreenWidth() - 250, 20, 18, LIGHTGRAY);
    
    // Mensaje si no hay partidas
    if (count == 0) {
        DrawText("No hay partidas disponibles", centerX - 150, centerY, 25, RED);
        DrawText("Esperando partidas...", centerX - 100, centerY + 40, 20, ORANGE);
        DrawText("ESC Volver | R Actualizar", centerX - 130, GetScreenHeight() - 60, 20, LIGHTGRAY);
        return;
    }
    
    // Encabezados de tabla
    int startY = 120;
    DrawText("ID PARTIDA", 50, startY, 20, GRAY);
    DrawText("JUGADORES", 300, startY, 20, GRAY);
    DrawText("ESPECTADORES", 450, startY, 20, GRAY);
    DrawText("ESTADO", 640, startY, 20, GRAY);
    DrawLine(30, startY + 30, GetScreenWidth() - 30, startY + 30, GRAY);
    
    // Lista de partidas
    for (int i = 0; i < count && i < 8; i++) { // Maximo 8 partidas visibles
        int y = startY + 50 + i * 45;
        Color color = (i == seleccion) ? GREEN : WHITE;
        Color color_fondo = (i == seleccion) ? (Color){0, 100, 0, 50} : (Color){0, 0, 0, 0};
        
        // Fondo de seleccion
        if (i == seleccion) {
            DrawRectangle(30, y - 5, GetScreenWidth() - 60, 40, color_fondo);
        }
        
        // Indicador de seleccion
        if (i == seleccion) {
            DrawText(">", 20, y, 25, GREEN);
        }
        
        // Acortar ID si es muy largo
        char id_corto[30];
        if (strlen(partidas[i].game_id) > 25) {
            strncpy(id_corto, partidas[i].game_id, 22);
            id_corto[22] = '.';
            id_corto[23] = '.';
            id_corto[24] = '.';
            id_corto[25] = '\0';
        } else {
            strcpy(id_corto, partidas[i].game_id);
        }
        
        // Dibujar informacion de partida
        DrawText(id_corto, 50, y, 20, color);
        DrawText(TextFormat("%d", partidas[i].player_count), 330, y, 20, color);
        DrawText(TextFormat("%d", partidas[i].spectators), 510, y, 20, color);
        
        // Estado con color
        const char* estado_str = partidas[i].active ? "ACTIVA" : "INACTIVA";
        Color estado_color = partidas[i].active ? GREEN : RED;
        DrawText(estado_str, 650, y, 20, estado_color);
    }
    
    // Indicar si hay mas partidas no mostradas
    if (count > 8) {
        DrawText(TextFormat("... y %d mas", count - 8), centerX - 50, startY + 410, 18, GRAY);
    }
    
    // Instrucciones
    DrawRectangle(0, GetScreenHeight() - 80, GetScreenWidth(), 80, (Color){0, 0, 0, 180});
    DrawText("NAVEGAR | ENTER Observar | R Actualizar | ESC Volver", 
             centerX - 280, GetScreenHeight() - 50, 20, LIGHTGRAY);
}

void dibujar_hud_espectador(const char* partida_actual) {
    // Panel informativo
    DrawRectangle(10, 10, 320, 70, (Color){0, 0, 0, 160});
    DrawText("MODO ESPECTADOR", 20, 15, 22, YELLOW);
    
    // Acortar ID si es muy largo
    char id_display[35];
    if (strlen(partida_actual) > 30) {
        strncpy(id_display, partida_actual, 27);
        id_display[27] = '.';
        id_display[28] = '.';
        id_display[29] = '.';
        id_display[30] = '\0';
    } else {
        strcpy(id_display, partida_actual);
    }
    
    DrawText(TextFormat("Partida: %s", id_display), 20, 45, 16, WHITE);
    DrawText("ESC para salir", GetScreenWidth() - 160, 20, 20, LIGHTGRAY);
}

void dibujar_hud_jugador(const char* partida_actual) {
    // Panel informativo
    DrawRectangle(10, 10, 280, 50, (Color){0, 0, 0, 160});
    DrawText("MODO JUGADOR", 20, 15, 22, GREEN);
    DrawText("ESC Menu", GetScreenWidth() - 120, 20, 20, LIGHTGRAY);
}

void dibujar_pantalla_game_over(void) {
    int centerX = GetScreenWidth() / 2;
    int centerY = GetScreenHeight() / 2;
    
    // Overlay oscuro
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0, 0, 0, 200});
    
    // Titulo GAME OVER con efecto de sombra
    DrawText("GAME OVER", centerX - 180, centerY - 80, 60, RED);
    DrawText("GAME OVER", centerX - 182, centerY - 82, 60, DARKGRAY);
    
    // Mensaje
    DrawText("Te quedaste sin vidas", centerX - 130, centerY + 20, 25, WHITE);
    DrawText("Volviendo al menu...", centerX - 120, centerY + 60, 22, LIGHTGRAY);
}

void dibujar_pantalla_victoria(void) {
    int centerX = GetScreenWidth() / 2;
    int centerY = GetScreenHeight() / 2;
    
    // Overlay dorado
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){255, 215, 0, 100});
    
    // Titulo VICTORIA con efecto de sombra
    DrawText("VICTORIA", centerX - 160, centerY - 80, 60, GOLD);
    DrawText("VICTORIA", centerX - 162, centerY - 82, 60, ORANGE);
    
    // Mensaje
    DrawText("Rescataste a tu padre", centerX - 150, centerY + 20, 28, WHITE);
    DrawText("Volviendo al menu...", centerX - 120, centerY + 60, 22, LIGHTGRAY);
}

void dibujar_estado_espera(EstadoMenu estado) {
    int centerX = GetScreenWidth() / 2;
    int centerY = GetScreenHeight() / 2;
    
    // Fondo
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0, 0, 0, 200});
    
    // Mensaje segun estado
    const char* mensaje;
    if (estado == MENU_CREATING_GAME) {
        mensaje = "Creando partida...";
    } else {
        mensaje = "Uniendose a partida...";
    }
    
    DrawText(mensaje, centerX - 140, centerY - 20, 30, YELLOW);
    
    // Animacion de puntos para indicar actividad
    static int dots = 0;
    static double last_time = 0;
    double current_time = GetTime();
    
    if (current_time - last_time > 0.5) {
        dots = (dots + 1) % 4;
        last_time = current_time;
    }
    
    char dots_str[5] = "";
    for (int i = 0; i < dots; i++) {
        strcat(dots_str, ".");
    }
    DrawText(dots_str, centerX + 140, centerY - 20, 30, YELLOW);
    
    // Instruccion
    DrawText("ESC para cancelar", centerX - 100, centerY + 40, 20, LIGHTGRAY);
}