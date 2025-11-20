#include "graficos.h"
#include "mapa.h"
#include "controles.h"
#include "game.h"
#include "conexion.h"
#include <stdio.h>

int main(void) {
    EstadoJuego estado;
    Controles ctrl = {0};
    
    // Inicialización gráfica y del juego
    inicializar_graficos();
    cargar_mapa(&mapa_global);
    inicializar_juego(&estado);
    configurar_mapa_completo(&mapa_global, &estado);
    
    // DEBUG: Estado inicial
    printf("🎮 Estado INICIAL del juego:\n");
    printf("   - Posición: (%.1f, %.1f)\n", estado.jugador.x, estado.jugador.y);
    printf("   - Vidas: %d\n", estado.jugador.vidas);
    printf("   - Puntos: %d\n", estado.jugador.puntuacion);
    printf("   - Juego activo: %s\n", estado.juego_activo ? "SÍ" : "NO");
    
    // Conexión al servidor
    if(!conectar_servidor("127.0.0.1")) {
        printf("Modo local activado.\n");
    } else {
        printf("✅ Conectado al servidor\n");
    }
    
    // Bucle principal del juego
    while (!WindowShouldClose()) {
        actualizar_controles(&ctrl);
        
        if (servidor_conectado && estado.juego_activo) {
            // MODO CON SERVIDOR
            aplicar_movimiento(&estado, &ctrl);
            verificar_colisiones_matriz(&estado);
            actualizar_matriz_desde_estado(&estado);
            
            // Sincronizar con servidor cada ciertos frames
            static int frame_count = 0;
            if (frame_count % 10 == 0) { // Cada 2 frames
                printf("🔄 Frame %d - Sincronizando con servidor...\n", frame_count);
                
                // Convertir coordenadas a matriz antes de enviar
                int matriz_x, matriz_y;
                coordenadas_a_matriz(estado.jugador.x, estado.jugador.y, &matriz_x, &matriz_y);

                printf("📍 Enviando coordenadas MATRIZ: [%d, %d]\n", matriz_x, matriz_y);

                if (enviar_estado_actual_al_servidor(matriz_x, matriz_y, estado.jugador.vidas, estado.jugador.puntuacion)) {
                    printf("📤 Estado enviado al servidor\n");

                    //Recibir consecuencias
                    int vidas_serv, puntos_serv;
                    bool activo_serv;

                    if (recibir_consecuencias_del_servidor(&vidas_serv, &puntos_serv, &activo_serv)) {
                        printf("📥 Consecuencias recibidas del servidor\n");

                        //Aplcar consecuencias
                        estado.jugador.vidas = vidas_serv;
                        estado.jugador.puntuacion = puntos_serv;
                        estado.juego_activo = activo_serv;

                        printf("🔄 Estado actualizado:\n");
                        printf("   - Vidas: %d\n", estado.jugador.vidas);
                        printf("   - Puntos: %d\n", estado.jugador.puntuacion);
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
            // MODO LOCAL (sin servidor)
            aplicar_movimiento(&estado, &ctrl);
            verificar_colisiones_matriz(&estado);
            actualizar_matriz_desde_estado(&estado);
        }
        
        // Dibujar escena completa con sprites
        dibujar_escena_completa(&estado, &mapa_global, &sprites_global);
        
        // Pantalla de juego terminado
        if (!estado.juego_activo) {
            printf("💀 JUEGO TERMINADO - Mostrando pantalla final\n");
            BeginDrawing();
            ClearBackground(BLACK);
            DrawText("JUEGO TERMINADO", 100, 200, 40, RED);
            DrawText("Presiona ESC para salir", 120, 250, 20, WHITE);
            EndDrawing();
        }
    }
    
    // Limpieza
    if (servidor_conectado) {
        desconectar_servidor();
    }
    descargar_mapa(&mapa_global);
    cerrar_graficos();
    
    return 0;
}