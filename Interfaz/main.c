#include "graficos.h"
#include "controles.h"
#include "conexion.h"
#include "menu.h"
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// Estados transitorios para animaciones
typedef enum {
    TRANSITION_NONE,
    TRANSITION_GAME_OVER,
    TRANSITION_VICTORIA
} EstadoTransicion;

void inicializar_estado_default(EstadoJuego *estado) {
    printf("Inicializando estado por defecto\n");
    memset(estado, 0, sizeof(EstadoJuego));
    
    // Jugador
    estado->jugador.x = 100.0f;
    estado->jugador.y = 500.0f;
    estado->jugador.vidas = 3;
    estado->jugador.activo = true;
    estado->juego_activo = true;
    
    // ==================== INICIALIZAR LIANAS ====================
    estado->num_lianas = 9;
    
    estado->lianas[0].x = 40;
    estado->lianas[0].y_inicio = 80;
    estado->lianas[0].y_fin = 480;
    
    estado->lianas[1].x = 110;
    estado->lianas[1].y_inicio = 80;
    estado->lianas[1].y_fin = 470;
    
    estado->lianas[2].x = 200;
    estado->lianas[2].y_inicio = 220;
    estado->lianas[2].y_fin = 490;
    
    estado->lianas[3].x = 320;
    estado->lianas[3].y_inicio = 80;
    estado->lianas[3].y_fin = 410;
    
    estado->lianas[4].x = 450;
    estado->lianas[4].y_inicio = 80;
    estado->lianas[4].y_fin = 300;
    
    estado->lianas[5].x = 520;
    estado->lianas[5].y_inicio = 120;
    estado->lianas[5].y_fin = 400;
    
    estado->lianas[6].x = 580;
    estado->lianas[6].y_inicio = 130;
    estado->lianas[6].y_fin = 350;
    
    estado->lianas[7].x = 680;
    estado->lianas[7].y_inicio = 50;
    estado->lianas[7].y_fin = 400;
    
    estado->lianas[8].x = 750;
    estado->lianas[8].y_inicio = 50;
    estado->lianas[8].y_fin = 400;
    
    // ==================== INICIALIZAR PLATAFORMAS ====================
    estado->num_plataformas = 10;
    
    // NIVEL INFERIOR
    estado->plataformas[0].x = 0;
    estado->plataformas[0].y = 550;
    estado->plataformas[0].ancho = 200;
    
    estado->plataformas[1].x = 420;
    estado->plataformas[1].y = 520;
    estado->plataformas[1].ancho = 80;
    
    estado->plataformas[2].x = 550;
    estado->plataformas[2].y = 500;
    estado->plataformas[2].ancho = 80;
    
    estado->plataformas[3].x = 300;
    estado->plataformas[3].y = 500;
    estado->plataformas[3].ancho = 90;
    
    estado->plataformas[4].x = 670;
    estado->plataformas[4].y = 460;
    estado->plataformas[4].ancho = 100;
    
    // NIVEL MEDIO
    estado->plataformas[5].x = 130;
    estado->plataformas[5].y = 220;
    estado->plataformas[5].ancho = 100;
    
    estado->plataformas[6].x = 130;
    estado->plataformas[6].y = 340;
    estado->plataformas[6].ancho = 150;
    
    estado->plataformas[7].x = 600;
    estado->plataformas[7].y = 280;
    estado->plataformas[7].ancho = 180;
    
    // NIVEL SUPERIOR
    estado->plataformas[8].x = 460;
    estado->plataformas[8].y = 80;
    estado->plataformas[8].ancho = 200;
    
    estado->plataformas[9].x = 0;
    estado->plataformas[9].y = 60;
    estado->plataformas[9].ancho = 460;
    
    // ==================== INICIALIZAR PADRE ====================
    estado->padre.x = (40 / 2) * 20 + 20 / 2;
    estado->padre.y = 2 * 20 + 20 / 2;
    estado->padre.activo = true;
    
    printf("✅ Estado inicializado:\n");
    printf("   - Lianas: %d\n", estado->num_lianas);
    printf("   - Plataformas: %d\n", estado->num_plataformas);
    printf("   - Padre en: (%.1f, %.1f)\n", estado->padre.x, estado->padre.y);
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
        printf("⚠️ Modo local activado (sin servidor)\n");
        printf("⚠️ No se podrá jugar sin servidor\n");
    } else {
        printf("✅ Conectado al servidor\n");
    }

    // Variables para estado de la aplicación
    EstadoMenu estado_menu = MENU_MAIN;
    int seleccion_actual = 0;
    
    // Variables para transiciones
    EstadoTransicion transicion_actual = TRANSITION_NONE;
    double tiempo_transicion = 0;
    const double DURACION_TRANSICION = 3.0; // 3 segundos
    
    // Variables previas de juego
    int vidas_previas = 3;
    float distancia_previa_padre = 1000.0f;
    
    // Bucle principal del juego
    while (!WindowShouldClose()) {
        double tiempo_actual = GetTime();
        
        // 1. DETECTAR INPUTS (siempre)
        detectar_inputs_frame(&inputs_frame);
        
        // 2. VERIFICAR CONDICIONES DE FIN DE JUEGO (solo en MENU_PLAYING)
        if (estado_menu == MENU_PLAYING && transicion_actual == TRANSITION_NONE) {
            // Verificar GAME OVER
            if (estado_juego.jugador.vidas <= 0 && vidas_previas > 0) {
                printf("💀 GAME OVER - Sin vidas\n");
                transicion_actual = TRANSITION_GAME_OVER;
                tiempo_transicion = tiempo_actual;
            }
            
            // Verificar VICTORIA
            float distancia_al_padre = sqrt(
                pow(estado_juego.jugador.x - estado_juego.padre.x, 2) +
                pow(estado_juego.jugador.y - estado_juego.padre.y, 2)
            );
            
            const float DISTANCIA_VICTORIA = 30.0f;
            
            if (distancia_al_padre <= DISTANCIA_VICTORIA && 
                distancia_previa_padre > DISTANCIA_VICTORIA &&
                estado_juego.padre.activo) {
                printf("🎉 ¡VICTORIA! - Llegaste donde tu padre\n");
                transicion_actual = TRANSITION_VICTORIA;
                tiempo_transicion = tiempo_actual;
            }
            
            vidas_previas = estado_juego.jugador.vidas;
            distancia_previa_padre = distancia_al_padre;
        }
        
        // 3. MANEJAR TRANSICIONES
        if (transicion_actual != TRANSITION_NONE) {
            double tiempo_transcurrido = tiempo_actual - tiempo_transicion;
            
            if (tiempo_transcurrido >= DURACION_TRANSICION) {
                // Fin de transición - volver al menú
                salir_partida_jugador(partida_seleccionada_global);
                set_partida_activa(false);
                estado_menu = MENU_MAIN;
                transicion_actual = TRANSITION_NONE;
                seleccion_actual = 0;
                vidas_previas = 3;
                distancia_previa_padre = 1000.0f;
                printf("🔙 Volviendo al menú principal\n");
            }
        }
        
        // 4. MÁQUINA DE ESTADOS - ACTUALIZACIÓN (solo si no hay transición)
        if (transicion_actual == TRANSITION_NONE) {
            switch (estado_menu) {
                case MENU_MAIN:
                    actualizar_menu_principal(&inputs_frame, &estado_menu, &seleccion_actual);
                    break;
                    
                case MENU_SELECT_GAME:
                    actualizar_seleccion_partida(&inputs_frame, &estado_menu, &seleccion_actual);
                    break;
                    
                case MENU_CREATING_GAME:
                case MENU_JOINING_GAME:
                    actualizar_estado_espera(&inputs_frame, &estado_menu, &estado_juego);
                    break;
                    
                case MENU_PLAYING:
                    actualizar_modo_jugador(&inputs_frame, &estado_menu, &estado_juego);
                    break;
                    
                case MENU_SPECTATING:
                    actualizar_modo_espectador(&inputs_frame, &estado_menu, &estado_juego);
                    break;
            }
        }
        
        // 5. MÁQUINA DE ESTADOS - RENDERIZADO
        BeginDrawing();
        
        // Renderizar según transición o estado
        if (transicion_actual != TRANSITION_NONE) {
            // Mostrar pantalla de transición sobre el juego
            dibujar_escena_completa(&estado_juego, &sprites_global);
            
            if (transicion_actual == TRANSITION_GAME_OVER) {
                dibujar_pantalla_game_over();
            } else if (transicion_actual == TRANSITION_VICTORIA) {
                dibujar_pantalla_victoria();
            }
        } else {
            // Renderizado normal según estado
            switch (estado_menu) {
                case MENU_MAIN:
                    ClearBackground(BLACK);
                    dibujar_interfaz_menu(estado_menu, seleccion_actual, 
                                         partidas_disponibles, cantidad_partidas, 
                                         partida_seleccionada_global);
                    break;
                    
                case MENU_SELECT_GAME:
                    ClearBackground(BLACK);
                    dibujar_interfaz_menu(estado_menu, seleccion_actual, 
                                         partidas_disponibles, cantidad_partidas, 
                                         partida_seleccionada_global);
                    break;
                    
                case MENU_CREATING_GAME:
                case MENU_JOINING_GAME:
                    ClearBackground(BLACK);
                    dibujar_estado_espera(estado_menu);
                    break;
                    
                case MENU_PLAYING:
                    dibujar_escena_completa(&estado_juego, &sprites_global);
                    dibujar_hud_jugador(partida_seleccionada_global);
                    break;
                    
                case MENU_SPECTATING:
                    dibujar_escena_completa(&estado_juego, &sprites_global);
                    dibujar_hud_espectador(partida_seleccionada_global);
                    break;
            }
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
        set_partida_activa(false);
        desconectar_servidor();
    }
    cerrar_graficos();
    
    printf("Cliente cerrado correctamente\n");
    return 0;
}