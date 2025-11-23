#include "game.h"
#include <stdio.h>
#include <string.h>

// ==================== INICIALIZACIÓN ====================

void inicializar_estado_juego(EstadoJuego *estado) {
    printf("=== INICIALIZANDO ESTADO DEL JUEGO ===\n");
    
    // Limpiar estructura
    memset(estado, 0, sizeof(EstadoJuego));
    
    // Inicializar mapa estático
    inicializar_mapa_estatico(estado);
    
    // Estado inicial del jugador
    estado->jugador = (Jugador){
        .x = 100.0f,
        .y = 500.0f, 
        .vidas = 3,
        .puntuacion = 0,
        .activo = true,
        .estado = ESTADO_SUELO,
        .liana_actual = -1,
        .en_suelo = true
    };
    
    estado->juego_activo = true;
    
    printf("Estado del juego inicializado\n");
}

void inicializar_mapa_estatico(EstadoJuego *estado) {
    printf("=== INICIALIZANDO MAPA ESTÁTICO ===\n");
    
    // PADRE - Donkey Kong
    estado->padre = (Padre){400.0f, 50.0f, true};
    
    // PLATAFORMAS/ISLAS (colisiones)
    estado->num_plataformas = 5;
    estado->plataformas[0] = (Plataforma){0.0f, 550.0f, 200.0f};    // Inferior izquierda
    estado->plataformas[1] = (Plataforma){300.0f, 500.0f, 90.0f};   // Inferior centro
    estado->plataformas[2] = (Plataforma){130.0f, 220.0f, 100.0f};  // Media izquierda
    estado->plataformas[3] = (Plataforma){600.0f, 280.0f, 180.0f};  // Media derecha
    estado->plataformas[4] = (Plataforma){460.0f, 80.0f, 200.0f};   // Superior derecha
    
    // LIANAS
    estado->num_lianas = 5;
    estado->lianas[0] = (Liana){100.0f, 80.0f, 480.0f};
    estado->lianas[1] = (Liana){200.0f, 220.0f, 490.0f};
    estado->lianas[2] = (Liana){320.0f, 80.0f, 410.0f};
    estado->lianas[3] = (Liana){520.0f, 120.0f, 400.0f};
    estado->lianas[4] = (Liana){680.0f, 50.0f, 400.0f};
    
    // COCODRILOS INICIALES
    estado->num_cocodrilos = 3;
    estado->cocodrilos[0] = (Cocodrilo){460.0f, 65.0f, ENEMY_RED_CROCODILE, true};
    estado->cocodrilos[1] = (Cocodrilo){500.0f, 65.0f, ENEMY_BLUE_CROCODILE, true};
    estado->cocodrilos[2] = (Cocodrilo){540.0f, 65.0f, ENEMY_RED_CROCODILE, true};
    
    // FRUTAS INICIALES
    estado->num_frutas = 4;
    estado->frutas[0] = (Fruta){150.0f, 200.0f, 100, FRUIT_BANANA, true};
    estado->frutas[1] = (Fruta){350.0f, 250.0f, 150, FRUIT_APPLE, true};
    estado->frutas[2] = (Fruta){550.0f, 150.0f, 120, FRUIT_PEAR, true};
    estado->frutas[3] = (Fruta){650.0f, 200.0f, 130, FRUIT_ORANGE, true};
    
    printf("Mapa estático inicializado:\n");
    printf("- %d plataformas\n", estado->num_plataformas);
    printf("- %d lianas\n", estado->num_lianas); 
    printf("- %d cocodrilos\n", estado->num_cocodrilos);
    printf("- %d frutas\n", estado->num_frutas);
}

// ==================== CONVERSIONES ESTADO-JSON ====================

EstadoPlayerJSON estado_jugador_a_json(EstadoJugador estado) {
    switch (estado) {
        case ESTADO_SUELO: return PLAYER_STANDING;
        case ESTADO_AGARRADO_LIANA: return PLAYER_CLIMBING;
        case ESTADO_SALTANDO: return PLAYER_JUMPING;
        case ESTADO_CAYENDO: return PLAYER_FALLING;
        default: return PLAYER_STANDING;
    }
}

EstadoJugador estado_jugador_desde_json(EstadoPlayerJSON estado_json) {
    switch (estado_json) {
        case PLAYER_STANDING: return ESTADO_SUELO;
        case PLAYER_MOVING_LEFT: return ESTADO_SUELO; // Aproximación
        case PLAYER_MOVING_RIGHT: return ESTADO_SUELO; // Aproximación  
        case PLAYER_CLIMBING: return ESTADO_AGARRADO_LIANA;
        case PLAYER_JUMPING: return ESTADO_SALTANDO;
        case PLAYER_FALLING: return ESTADO_CAYENDO;
        default: return ESTADO_SUELO;
    }
}

// ==================== UTILIDADES ====================

void limpiar_estado_juego(EstadoJuego *estado) {
    // Por ahora solo resetear
    resetear_estado_juego(estado);
}

void resetear_estado_juego(EstadoJuego *estado) {
    printf("=== RESETEANDO ESTADO DEL JUEGO ===\n");
    
    // Guardar configuración estática
    Padre padre_backup = estado->padre;
    int num_plataformas = estado->num_plataformas;
    Plataforma plataformas_backup[15];
    memcpy(plataformas_backup, estado->plataformas, sizeof(Plataforma) * num_plataformas);
    
    int num_lianas = estado->num_lianas;
    Liana lianas_backup[20];
    memcpy(lianas_backup, estado->lianas, sizeof(Liana) * num_lianas);
    
    // Limpiar todo
    memset(estado, 0, sizeof(EstadoJuego));
    
    // Restaurar estático
    estado->padre = padre_backup;
    estado->num_plataformas = num_plataformas;
    memcpy(estado->plataformas, plataformas_backup, sizeof(Plataforma) * num_plataformas);
    estado->num_lianas = num_lianas;
    memcpy(estado->lianas, lianas_backup, sizeof(Liana) * num_lianas);
    
    // Reiniciar jugador
    estado->jugador = (Jugador){
        .x = 100.0f,
        .y = 500.0f,
        .vidas = 3,
        .puntuacion = 0, 
        .activo = true,
        .estado = ESTADO_SUELO,
        .liana_actual = -1,
        .en_suelo = true
    };
    
    estado->juego_activo = true;
    
    printf("Estado del juego reseteado\n");
}