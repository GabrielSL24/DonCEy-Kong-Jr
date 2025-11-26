#include "graficos.h"
#include "controles.h"
#include "game.h"
#include "conexion.h"
#include <stdio.h>

int main(void) {
    EstadoJuego estado;
    FrameInputs inputs_frame;
    
    // Inicialización
    inicializar_graficos();
    inicializar_estado_juego(&estado);
    
    printf("Cliente Donkey Kong Jr Iniciado\n");
    printf("   - Pantalla: %dx%d\n", SCREEN_WIDTH, SCREEN_HEIGHT);
    printf("   - FPS: %d\n", FPS);
    
    // Conexión al servidor
    if(!conectar_servidor("127.0.0.1")) {
        printf("Modo local activado (sin servidor)\n");
    } else {
        printf("Conectado al servidor\n");
    }

    // Variables para control de tiempo (fuera del loop)
    static float ultimo_envio = 0.0f;
    static bool primer_input_enviado = false;
    static float tiempo_inicio = 0.0f;
    
    // Bucle principal del juego
    while (!WindowShouldClose()) {

        float tiempo_actual = GetTime();

        // 1. DETECTAR INPUTS
        detectar_inputs_frame(&inputs_frame);
        
        // 2. ENVIAR INPUTS AL SERVIDOR
        if (servidor_conectado) {
            
            // Inicializar tiempo_inicio en el primer frame
            if (tiempo_inicio == 0.0f) {
                tiempo_inicio = GetTime();
            }

            // Enviar inputs cada 100ms (10 FPS) para no saturar
            if (inputs_frame.num_inputs > 0 && (tiempo_actual - ultimo_envio > 0.1f)) {
                for (int i = 0; i < inputs_frame.num_inputs; i++) {
                    printf("Enviando input: %s\n", inputs_frame.inputs[i]);
                    enviar_input_al_servidor(
                        CLIENT_PLAYER, 
                        1, 
                        "partida_default",
                        "KEY_PRESSED",
                        inputs_frame.inputs[i]
                    );
                }
                primer_input_enviado = true;
                ultimo_envio = tiempo_actual;
            }

            // Input automático después de 2 segundos si no se ha enviado nada 
            if (!primer_input_enviado && (tiempo_actual - tiempo_inicio > 2.0f)) {
                printf("Enviando input inicial de prueba...\n");
                enviar_input_al_servidor(
                    CLIENT_PLAYER, 
                    1, 
                    "partida_default",
                    "KEY_PRESSED", 
                    "RIGHT"
                );
                primer_input_enviado = true;
                ultimo_envio = tiempo_actual;
            }
        } else {
            // MODO LOCAL: Mostrar inputs detectados (debug)
            if (inputs_frame.num_inputs > 0) {
                printf("Modo local - Inputs ignorados: ");
                for (int i = 0; i < inputs_frame.num_inputs; i++) {
                    printf("%s ", inputs_frame.inputs[i]);
                }
                printf("\n");
            }
        }
        
        // 3. RECIBIR ESTADO ACTUALIZADO DEL SERVIDOR
        if (servidor_conectado) {
            if (recibir_estado_actualizado(&estado)) {
                printf("Estado recibido del servidor - Pos: (%.1f, %.1f)\n", 
                       estado.jugador.x, estado.jugador.y);
            }
        } else {
            // MODO LOCAL: Simular estado estático (solo para prueba)
            static bool mostrado = false;
            if (!mostrado) {
                printf("Ejecutando en modo local - Esperando servidor...\n");
                mostrado = true;
            }
        }
        
        // 4. RENDERIZAR ESCENA
        dibujar_escena_completa(&estado, &sprites_global);
        
        // 5. MANEJAR ESTADO DEL JUEGO
        if (!estado.juego_activo && servidor_conectado) {
            // Juego terminado por el servidor
            printf("JUEGO TERMINADO - Esperando reinicio...\n");
            
            // Podrías agregar lógica para reiniciar o salir
            if (IsKeyPressed(KEY_ENTER)) {
                printf("Solicitando reinicio al servidor...\n");
                // enviar_input_al_servidor(CLIENT_PLAYER, 1, "partida_default", "RESTART", "");
            }
        }
    }
    
    // Limpieza
    if (servidor_conectado) {
        desconectar_servidor();
    }
    cerrar_graficos();
    
    printf("👋 Cliente cerrado correctamente\n");
    return 0;
}