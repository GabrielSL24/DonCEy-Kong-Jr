#include "C:/raylib/raylib/src/raylib.h"

#include <stdio.h>
#include <stdbool.h>
//#include "raylib.h" 

#include "game.h"
#include "config.h"
#include "graficos.h"
#include "controles.h"
#include "conexion.h"

int main(void) {
    EstadoJuego estado;
    Controles controles = {0};
    
    inicializar_graficos();
    inicializar_juego(&estado);
    
    // ✅ DEBUG: Estado inicial del juego
    printf("🎮 Estado INICIAL del juego:\n");
    printf("   - Posición: (%.1f, %.1f)\n", estado.jugador.x, estado.jugador.y);
    printf("   - Vidas: %d\n", estado.jugador.vidas);
    printf("   - Puntos: %d\n", estado.jugador.puntuacion);
    printf("   - Juego activo: %s\n", estado.juego_activo ? "SÍ" : "NO");
    
    if(!conectar_servidor("127.0.0.1")) {
        printf("Modo local activado.\n");
    } else {
        printf("✅ Conectado al servidor\n");
    }
    
    while (!WindowShouldClose()) {
        actualizar_controles(&controles);
        
        if (servidor_conectado && estado.juego_activo) {
            // 1. MOVIMIENTO LOCAL
            aplicar_movimiento(&estado, &controles);
            verificar_colisiones_matriz(&estado);
            actualizar_matriz_desde_estado(&estado);
            
            // 2. SINCRONIZAR con servidor
            static int frame_count = 0;
            if (frame_count % 2 == 0) {
                printf("🔄 Frame %d - Sincronizando con servidor...\n", frame_count);
                printf("   - Estado actual: vidas=%d, puntos=%d\n", 
                       estado.jugador.vidas, estado.jugador.puntuacion);
                
                // Enviar estado actual
                if (enviar_estado_actual_al_servidor(estado.jugador.x, estado.jugador.y,
                                                    estado.jugador.vidas, estado.jugador.puntuacion)) {
                    printf("📤 Estado enviado al servidor\n");
                    
                    // Recibir consecuencias
                    int vidas_serv, puntos_serv;
                    bool activo_serv;
                    if (recibir_consecuencias_del_servidor(&vidas_serv, &puntos_serv, &activo_serv)) {
                        printf("📥 Consecuencias recibidas del servidor\n");
                        
                        // Aplicar consecuencias
                        estado.jugador.vidas = vidas_serv;
                        estado.jugador.puntuacion = puntos_serv;
                        estado.juego_activo = activo_serv;
                        
                        printf("🔄 Estado actualizado:\n");
                        printf("   - Vidas: %d → %d\n", estado.jugador.vidas, vidas_serv);
                        printf("   - Puntos: %d → %d\n", estado.jugador.puntuacion, puntos_serv);
                        printf("   - Activo: %s\n", estado.juego_activo ? "SÍ" : "NO");
                    } else {
                        printf("❌ No se pudieron recibir consecuencias\n");
                    }
                } else {
                    printf("❌ No se pudo enviar estado al servidor\n");
                }
            }
            frame_count++;
            
        } else if (!servidor_conectado) {
            // MODO LOCAL
            aplicar_movimiento(&estado, &controles);
            verificar_colisiones_matriz(&estado);
            actualizar_matriz_desde_estado(&estado);
        }
        
        dibujar_escena(&estado);
        
        if (!estado.juego_activo) {
            printf("💀 JUEGO TERMINADO - Mostrando pantalla final\n");
            BeginDrawing();
            ClearBackground(BLACK);
            DrawText("JUEGO TERMINADO", 100, 200, 40, RED);
            DrawText("Presiona ESC para salir", 120, 250, 20, WHITE);
            EndDrawing();
        }
    }
    
    if (servidor_conectado) {
        desconectar_servidor();
    }

    cerrar_graficos();
    return 0;
}