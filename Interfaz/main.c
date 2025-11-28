#include "graficos.h"
#include "controles.h"
#include "conexion.h"
#include "menu.h"
#include "config.h"
#include <stdio.h>
#include <string.h>

void inicializar_estado_default(EstadoJuego *estado) {
    printf("Inicializando estado por defecto\n");
    memset(estado, 0, sizeof(EstadoJuego));
    estado->jugador.x = 100.0f;
    estado->jugador.y = 500.0f;
    estado->jugador.vidas = 3;
    estado->jugador.activo = true;
    estado->juego_activo = true;
}

int main(void) {
    EstadoJuego estado_juego;
    FrameInputs inputs_frame;
    
    // Inicialización
    inicializar_graficos();
    inicializar_estado_default(&estado_juego);
    
    printf("Cliente Donkey Kong Jr Iniciado\n");
    printf("   - Pantalla: %dx%d\n", SCREEN_WIDTH, SCREEN_HEIGHT);
    printf("   - FPS: %d\n", FPS);
    
    // Conexión al servidor
    if(!conectar_servidor("127.0.0.1")) {
        printf("Modo local activado (sin servidor)\n");
    } else {
        printf("Conectado al servidor\n");
    }

    // Variables para estado de la aplicación
    EstadoMenu estado_menu = MENU_MAIN;
    int seleccion_actual = 0;
    
    // Bucle principal del juego
    while (!WindowShouldClose()) {
        // 1. DETECTAR INPUTS (siempre)
        detectar_inputs_frame(&inputs_frame);
        
        // 2. MAQUINA DE ESTADOS - ACTUALIZACIÓN
        switch (estado_menu) {
            case MENU_MAIN:
                actualizar_menu_principal(&inputs_frame, &estado_menu, &seleccion_actual);
                break;
                
            case MENU_SELECT_GAME:
                actualizar_seleccion_partida(&inputs_frame, &estado_menu, &seleccion_actual);
                break;
                
            case MENU_CREATING_GAME:    // ← NUEVO ESTADO
            case MENU_JOINING_GAME:     // ← NUEVO ESTADO
                actualizar_estado_espera(&inputs_frame, &estado_menu, &estado_juego);
                break;
                
            case MENU_PLAYING:
                actualizar_modo_jugador(&inputs_frame, &estado_menu, &estado_juego);
                break;
                
            case MENU_SPECTATING:
                actualizar_modo_espectador(&inputs_frame, &estado_menu, &estado_juego);
                break;
        }
        
        // 3. MAQUINA DE ESTADOS - RENDERIZADO
        BeginDrawing();
        
        switch (estado_menu) {
            case MENU_MAIN:
            case MENU_SELECT_GAME:
            case MENU_CREATING_GAME:    // ← NUEVO: mostrar menú durante espera
            case MENU_JOINING_GAME:     // ← NUEVO: mostrar menú durante espera
                // SOLO MENÚ - fondo negro limpio
                ClearBackground(BLACK);
                dibujar_interfaz_menu(estado_menu, seleccion_actual, partidas_disponibles, cantidad_partidas, partida_seleccionada_global);
                break;
                
            case MENU_PLAYING:
            case MENU_SPECTATING:
                // SOLO JUEGO - escena completa
                dibujar_escena_completa(&estado_juego, &sprites_global);
                // HUD se dibuja dentro de dibujar_escena_completa
                break;
        }
        
        EndDrawing();
    }
    
    // Limpieza
    if (servidor_conectado) {
        if (estado_menu == MENU_SPECTATING) {
            salir_partida_espectador(partida_seleccionada_global);
        } else if (estado_menu == MENU_PLAYING) {
            salir_partida_jugador(partida_seleccionada_global);
        }
        set_partida_activa(false); // ← AGREGAR: asegurar desactivación
        desconectar_servidor();
    }
    cerrar_graficos();
    
    printf("Cliente cerrado correctamente\n");
    return 0;
}