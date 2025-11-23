#include "graficos.h"
#include "mapa.h"
#include "controles.h"
#include "game.h"
#include "conexion.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    EstadoJuego estado;
    Controles ctrl = {0};
    
    // Variable para la IP del servidor
    char ip_servidor[16] = "127.0.0.1"; // Valor por defecto (localhost)
    
    // Verificar si se proporcionó IP como argumento
    if (argc >= 2) {
        strncpy(ip_servidor, argv[1], sizeof(ip_servidor) - 1);
        ip_servidor[sizeof(ip_servidor) - 1] = '\0'; // Asegurar terminación nula
        printf("🎯 IP del servidor proporcionada: %s\n", ip_servidor);
    } else {
        printf("ℹ️  Usando localhost (127.0.0.1). Para conectar a otra IP:\n");
        printf("   %s <ip_del_servidor>\n", argv[0]);
    }
    
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
    printf("   - Conectando a: %s\n", ip_servidor);
    
    // Conexión al servidor (CON LA IP PROPORCIONADA)
    if(!conectar_servidor(ip_servidor)) {
        printf("❌ No se pudo conectar al servidor %s. Modo local activado.\n", ip_servidor);
    } else {
        printf("✅ Conectado al servidor %s\n", ip_servidor);
    }
    
    // El resto de tu código se mantiene igual...
    while (!WindowShouldClose()) {
        actualizar_controles(&ctrl);
        
        if (servidor_conectado && estado.juego_activo) {
            // MODO CON SERVIDOR
            aplicar_movimiento(&estado, &ctrl);
            verificar_colisiones_matriz(&estado);
            actualizar_matriz_desde_estado(&estado);
            
            // Sincronizar con servidor cada ciertos frames
            static int frame_count = 0;
            if (frame_count % 10 == 0) {
                printf("🔄 Frame %d - Sincronizando con servidor %s...\n", frame_count, ip_servidor);
                
                // Convertir coordenadas a matriz antes de enviar
                int matriz_x, matriz_y;
                coordenadas_a_matriz(estado.jugador.x, estado.jugador.y, &matriz_x, &matriz_y);

                printf("📍 Enviando coordenadas MATRIZ: [%d, %d] a %s\n", matriz_x, matriz_y, ip_servidor);

                if (enviar_estado_actual_al_servidor(matriz_x, matriz_y, estado.jugador.vidas, estado.jugador.puntuacion)) {
                    printf("📤 Estado enviado al servidor %s\n", ip_servidor);

                    // Recibir consecuencias
                    int vidas_serv, puntos_serv;
                    bool activo_serv;

                    if (recibir_consecuencias_del_servidor(&vidas_serv, &puntos_serv, &activo_serv)) {
                        printf("📥 Consecuencias recibidas del servidor %s\n", ip_servidor);

                        // Aplicar consecuencias
                        estado.jugador.vidas = vidas_serv;
                        estado.jugador.puntuacion = puntos_serv;
                        estado.juego_activo = activo_serv;

                        printf("🔄 Estado actualizado desde %s:\n", ip_servidor);
                        printf("   - Vidas: %d\n", estado.jugador.vidas);
                        printf("   - Puntos: %d\n", estado.jugador.puntuacion);
                        printf("   - Activo: %s\n", estado.juego_activo ? "SÍ" : "NO");

                    } else {
                        printf("❌ No se pudieron recibir consecuencias de %s\n", ip_servidor);
                    }
                } else {
                    printf("❌ No se pudo enviar estado al servidor %s\n", ip_servidor);
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
        printf("🔌 Desconectado del servidor %s\n", ip_servidor);
    }
    descargar_mapa(&mapa_global);
    cerrar_graficos();
    
    return 0;
}