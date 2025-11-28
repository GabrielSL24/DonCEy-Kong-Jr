#include "graficos.h"
#include "config.h"
#include <stdio.h>
#include <string.h>
#include "menu.h"

SistemaSprites sprites_global;

// ==================== INICIALIZACIÓN Y LIMPIEZA ====================

void inicializar_graficos(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Donkey Kong Jr - Cliente");
    SetTargetFPS(FPS);
    cargar_sprites(&sprites_global);
    cargar_fondo(&sprites_global);
    printf("✅ Gráficos inicializados: %dx%d @ %d FPS\n", SCREEN_WIDTH, SCREEN_HEIGHT, FPS);
}

void cargar_sprites(SistemaSprites *sprites) {
    memset(sprites, 0, sizeof(SistemaSprites));
    
    printf("=== CARGANDO SPRITES ===\n");
    
    // JUGADOR - DK Jr
    if (FileExists(SPRITE_DKJ)) {
        sprites->jugador.textura = LoadTexture(SPRITE_DKJ);
        sprites->jugador.cargado = true;
        printf("✓ Jugador cargado: %s\n", SPRITE_DKJ);
    }
    
    // PADRE - Donkey Kong
    if (FileExists(SPRITE_DK)) {
        sprites->padre.textura = LoadTexture(SPRITE_DK);
        sprites->padre.cargado = true;
        printf("✓ Padre cargado: %s\n", SPRITE_DK);
    }
    
    // COCODRILOS
    if (FileExists(SPRITE_COCODRILO_ROJO)) {
        sprites->cocodrilo_rojo.textura = LoadTexture(SPRITE_COCODRILO_ROJO);
        sprites->cocodrilo_rojo.cargado = true;
        printf("✓ Cocodrilo rojo cargado: %s\n", SPRITE_COCODRILO_ROJO);
    }
    
    if (FileExists(SPRITE_COCODRILO_AZUL)) {
        sprites->cocodrilo_azul.textura = LoadTexture(SPRITE_COCODRILO_AZUL);
        sprites->cocodrilo_azul.cargado = true;
        printf("✓ Cocodrilo azul cargado: %s\n", SPRITE_COCODRILO_AZUL);
    }
    
    // FRUTAS - Mapear tipos a sprites disponibles
    if (FileExists(SPRITE_FRUTA_PLATANO)) {
        sprites->fruta_banana.textura = LoadTexture(SPRITE_FRUTA_PLATANO);
        sprites->fruta_banana.cargado = true;
        printf("✓ Fruta banana cargada: %s\n", SPRITE_FRUTA_PLATANO);
    }
    
    if (FileExists(SPRITE_FRUTA_MANZANA)) {
        sprites->fruta_apple.textura = LoadTexture(SPRITE_FRUTA_MANZANA);
        sprites->fruta_apple.cargado = true;
        printf("✓ Fruta apple cargada: %s\n", SPRITE_FRUTA_MANZANA);
    }
    
    // Usar mismos sprites para tipos faltantes (temporal)
    if (FileExists(SPRITE_FRUTA_PERAS)) {
        sprites->fruta_pear.textura = LoadTexture(SPRITE_FRUTA_PERAS);
        sprites->fruta_pear.cargado = true;
        sprites->fruta_orange.textura = LoadTexture(SPRITE_FRUTA_PERAS); // Reutilizar
        sprites->fruta_orange.cargado = true;
        printf("✓ Frutas pear/orange cargadas: %s\n", SPRITE_FRUTA_PERAS);
    }
    
    printf("=== SPRITES CARGADOS ===\n");
}

void descargar_sprites(SistemaSprites *sprites) {
    if (sprites->jugador.cargado) UnloadTexture(sprites->jugador.textura);
    if (sprites->padre.cargado) UnloadTexture(sprites->padre.textura);
    if (sprites->cocodrilo_rojo.cargado) UnloadTexture(sprites->cocodrilo_rojo.textura);
    if (sprites->cocodrilo_azul.cargado) UnloadTexture(sprites->cocodrilo_azul.textura);
    if (sprites->fruta_banana.cargado) UnloadTexture(sprites->fruta_banana.textura);
    if (sprites->fruta_apple.cargado) UnloadTexture(sprites->fruta_apple.textura);
    if (sprites->fruta_pear.cargado) UnloadTexture(sprites->fruta_pear.textura);
    if (sprites->fruta_orange.cargado) UnloadTexture(sprites->fruta_orange.textura);
    
    printf("✅ Sprites descargados\n");
}

void cerrar_graficos(void) {
    descargar_sprites(&sprites_global);
    descargar_fondo(&sprites_global);
    CloseWindow();
    printf("✅ Gráficos cerrados\n");
}

// ==================== FUNCIONES DE DIBUJO DE ENTIDADES ====================

void dibujar_jugador(const Jugador *jugador, const SistemaSprites *sprites) {
    if (!jugador->activo) return;
    
    if (sprites->jugador.cargado) {
        Rectangle dest = {
            jugador->x - SPRITE_JUGADOR_SIZE/2, 
            jugador->y - SPRITE_JUGADOR_SIZE/2,
            SPRITE_JUGADOR_SIZE, 
            SPRITE_JUGADOR_SIZE
        };
        DrawTexturePro(
            sprites->jugador.textura,
            (Rectangle){0, 0, sprites->jugador.textura.width, sprites->jugador.textura.height},
            dest,
            (Vector2){0, 0}, 0, WHITE
        );
    } else {
        // Fallback: rectángulo de debug
        DrawRectangle(jugador->x - JUGADOR_HITBOX/2, jugador->y - JUGADOR_HITBOX/2, 
                     JUGADOR_HITBOX, JUGADOR_HITBOX, COLOR_JUGADOR);
    }
}

void dibujar_padre(const Padre *padre, const SistemaSprites *sprites) {
    if (!padre->activo) return;
    
    if (sprites->padre.cargado) {
        Rectangle dest = {
            padre->x - SPRITE_PADRE_SIZE/2, 
            padre->y - SPRITE_PADRE_SIZE/2,
            SPRITE_PADRE_SIZE, 
            SPRITE_PADRE_SIZE
        };
        DrawTexturePro(
            sprites->padre.textura,
            (Rectangle){0, 0, sprites->padre.textura.width, sprites->padre.textura.height},
            dest,
            (Vector2){0, 0}, 0, WHITE
        );
    } else {
        DrawRectangle(padre->x - 20, padre->y - 20, 40, 40, COLOR_PADRE);
    }
}

void dibujar_cocodrilo(const Cocodrilo *cocodrilo, const SistemaSprites *sprites) {
    if (!cocodrilo->activo) return;
    
    const Texture2D *textura = NULL;
    
    switch (cocodrilo->tipo) {
        case ENEMY_RED_CROCODILE:
            if (sprites->cocodrilo_rojo.cargado) 
                textura = (const Texture2D*)&sprites->cocodrilo_rojo.textura;
            break;
        case ENEMY_BLUE_CROCODILE:
            if (sprites->cocodrilo_azul.cargado) 
                textura = (const Texture2D*)&sprites->cocodrilo_azul.textura;
            break;
    }
    
    if (textura) {
        Rectangle dest = {
            cocodrilo->x - SPRITE_COCODRILO_SIZE/2, 
            cocodrilo->y - SPRITE_COCODRILO_SIZE/2,
            SPRITE_COCODRILO_SIZE, 
            SPRITE_COCODRILO_SIZE
        };
        DrawTexturePro(
            *textura,
            (Rectangle){0, 0, textura->width, textura->height},
            dest,
            (Vector2){0, 0}, 0, WHITE
        );
    } else {
        Color color = (cocodrilo->tipo == ENEMY_RED_CROCODILE) ? COLOR_COCODRILO_ROJO : COLOR_COCODRILO_AZUL;
        DrawRectangle(cocodrilo->x - COCODRILO_HITBOX/2, cocodrilo->y - COCODRILO_HITBOX/2,
                     COCODRILO_HITBOX, COCODRILO_HITBOX, color);
    }
}

void dibujar_fruta(const Fruta *fruta, const SistemaSprites *sprites) {
    if (!fruta->activo) return;
    
    const Texture2D *textura = NULL;
    
    switch (fruta->tipo) {
        case FRUIT_BANANA:
            if (sprites->fruta_banana.cargado) 
                textura = (const Texture2D*)&sprites->fruta_banana.textura;
            break;
        case FRUIT_APPLE:
            if (sprites->fruta_apple.cargado) 
                textura = (const Texture2D*)&sprites->fruta_apple.textura;
            break;
        case FRUIT_PEAR:
            if (sprites->fruta_pear.cargado) 
                textura = (const Texture2D*)&sprites->fruta_pear.textura;
            break;
        case FRUIT_ORANGE:
            if (sprites->fruta_orange.cargado) 
                textura = (const Texture2D*)&sprites->fruta_orange.textura;
            break;
    }
    
    if (textura) {
        Rectangle dest = {
            fruta->x - SPRITE_FRUTA_SIZE/2, 
            fruta->y - SPRITE_FRUTA_SIZE/2,
            SPRITE_FRUTA_SIZE, 
            SPRITE_FRUTA_SIZE
        };
        DrawTexturePro(
            *textura,
            (Rectangle){0, 0, textura->width, textura->height},
            dest,
            (Vector2){0, 0}, 0, WHITE
        );
    } else {
        Color color_fruta;
        switch (fruta->tipo) {
            case FRUIT_BANANA: color_fruta = COLOR_BANANO; break;
            case FRUIT_APPLE: color_fruta = COLOR_FRUTA_ROJA; break;
            case FRUIT_PEAR: color_fruta = COLOR_FRUTA_CELESTE; break;
            case FRUIT_ORANGE: color_fruta = COLOR_NARANJA; break;
            default: color_fruta = GREEN;
        }
        DrawRectangle(fruta->x - FRUTA_HITBOX/2, fruta->y - FRUTA_HITBOX/2,
                     FRUTA_HITBOX, FRUTA_HITBOX, color_fruta);
    }
}

void dibujar_lianas(const Liana lianas[], int num_lianas) {
    for (int i = 0; i < num_lianas; i++) {
        DrawLineEx(
            (Vector2){lianas[i].x, lianas[i].y_inicio},
            (Vector2){lianas[i].x, lianas[i].y_fin},
            LIANA_WIDTH, COLOR_LIANA
        );
    }
}

void dibujar_plataformas(const Plataforma plataformas[], int num_plataformas) {
    for (int i = 0; i < num_plataformas; i++) {
        DrawRectangle(
            plataformas[i].x,
            plataformas[i].y - 5, // Pequeño grosor
            plataformas[i].ancho,
            10, // Altura de plataforma
            COLOR_PLATAFORMA
        );
    }
}

void cargar_fondo(SistemaSprites *sprites) {
    if (FileExists(SPRITE_FONDO)) {
        Image imagen_fondo = LoadImage(SPRITE_FONDO);
        ImageResize(&imagen_fondo, SCREEN_WIDTH, SCREEN_HEIGHT);
        sprites->fondo.textura = LoadTextureFromImage(imagen_fondo);
        UnloadImage(imagen_fondo);
        sprites->fondo.cargado = true;
        printf("✅ Fondo cargado: %s -> %dx%d\n", SPRITE_FONDO, SCREEN_WIDTH, SCREEN_HEIGHT);
    } else {
        sprites->fondo.cargado = false;
        printf("❌ No se encontró: %s\n", SPRITE_FONDO);
    }
}

void descargar_fondo(SistemaSprites *sprites) {
    if (sprites->fondo.cargado) {
        UnloadTexture(sprites->fondo.textura);
        sprites->fondo.cargado = false;
        printf("✅ Fondo descargado\n");
    }
}

void dibujar_fondo(const SistemaSprites *sprites) {
    if (sprites->fondo.cargado) {
        DrawTexture(sprites->fondo.textura, 0, 0, WHITE);
    } else {
        ClearBackground(COLOR_FONDO);
        DrawText("NO SE ENCONTRO fondo.png", 100, SCREEN_HEIGHT/2, 20, RED);
    }
}

// ==================== RENDERIZADO PRINCIPAL ====================

void dibujar_escena_completa(const EstadoJuego *estado, const SistemaSprites *sprites) {
    BeginDrawing();
    
    // 1. Fondo (ahora desde sprites)
    dibujar_fondo(sprites);
    
    // 2. Elementos estáticos del juego
    dibujar_lianas(estado->lianas, estado->num_lianas);
    dibujar_plataformas(estado->plataformas, estado->num_plataformas);
    
    // 3. Entidades dinámicas
    dibujar_padre(&estado->padre, sprites);
    
    for (int i = 0; i < estado->num_frutas; i++) {
        dibujar_fruta(&estado->frutas[i], sprites);
    }
    
    for (int i = 0; i < estado->num_cocodrilos; i++) {
        dibujar_cocodrilo(&estado->cocodrilos[i], sprites);
    }
    
    // 4. Jugador (siempre encima)
    dibujar_jugador(&estado->jugador, sprites);
    
    // 5. UI
    dibujar_ui(estado);
    
    // 6. Debug (si está activado)
    if (DEBUG_COLISIONES) {
        dibujar_debug_info(estado);
        dibujar_hitboxes(estado);
    }
    
    EndDrawing();
}

void dibujar_ui(const EstadoJuego *estado) {
    // Título
    DrawText("Donkey Kong Jr", 10, 10, 20, WHITE);
    
    // Vidas y puntuación
    DrawText(TextFormat("Vidas: %d", estado->jugador.vidas), SCREEN_WIDTH - 150, 10, 20, WHITE);
    DrawText(TextFormat("Puntos: %d", estado->jugador.puntuacion), SCREEN_WIDTH - 300, 10, 20, WHITE);
    
    // Estado del jugador
    const char* estado_str;
    switch (estado->jugador.estado) {
        case ESTADO_SUELO: estado_str = "SUELO"; break;
        case ESTADO_AGARRADO_LIANA: estado_str = "LIANA"; break;
        case ESTADO_SALTANDO: estado_str = "SALTANDO"; break;
        case ESTADO_CAYENDO: estado_str = "CAYENDO"; break;
        default: estado_str = "DESCONOCIDO";
    }
    DrawText(TextFormat("Estado: %s", estado_str), 10, 40, 20, YELLOW);
    
    // Posición del jugador
    DrawText(TextFormat("Pos: (%.0f, %.0f)", estado->jugador.x, estado->jugador.y), 
             SCREEN_WIDTH - 500, 10, 20, LIGHTGRAY);
    
    // Contadores de entidades
    DrawText(TextFormat("Frutas: %d", estado->num_frutas), 10, 70, 20, ORANGE);
    DrawText(TextFormat("Cocodrilos: %d", estado->num_cocodrilos), 10, 100, 20, RED);
    
    // Estado del juego
    if (!estado->juego_activo) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 128}); // Overlay oscuro
        DrawText("JUEGO TERMINADO", SCREEN_WIDTH/2 - 150, SCREEN_HEIGHT/2 - 20, 30, RED);
    }
}

// ==================== FUNCIONES DE DEBUG ====================

void dibujar_debug_info(const EstadoJuego *estado) {
    DrawText("DEBUG: ON (Cambia DEBUG_COLISIONES en config.h)", 10, SCREEN_HEIGHT - 30, 20, RED);
    DrawText(TextFormat("Entidades: %d frutas, %d cocodrilos", 
                       estado->num_frutas, estado->num_cocodrilos), 
             10, SCREEN_HEIGHT - 60, 20, DARKGRAY);
}

void dibujar_hitboxes(const EstadoJuego *estado) {
    // Hitbox del jugador
    DrawRectangleLines(
        estado->jugador.x - JUGADOR_HITBOX/2,
        estado->jugador.y - JUGADOR_HITBOX/2,
        JUGADOR_HITBOX,
        JUGADOR_HITBOX,
        GREEN
    );
    
    // Hitboxes de cocodrilos
    for (int i = 0; i < estado->num_cocodrilos; i++) {
        if (estado->cocodrilos[i].activo) {
            DrawRectangleLines(
                estado->cocodrilos[i].x - COCODRILO_HITBOX/2,
                estado->cocodrilos[i].y - COCODRILO_HITBOX/2,
                COCODRILO_HITBOX,
                COCODRILO_HITBOX,
                RED
            );
        }
    }
    
    // Hitboxes de frutas
    for (int i = 0; i < estado->num_frutas; i++) {
        if (estado->frutas[i].activo) {
            DrawRectangleLines(
                estado->frutas[i].x - FRUTA_HITBOX/2,
                estado->frutas[i].y - FRUTA_HITBOX/2,
                FRUTA_HITBOX,
                FRUTA_HITBOX,
                YELLOW
            );
        }
    }
}

void dibujar_interfaz_menu(EstadoMenu estado, int seleccion, InfoPartida partidas[], int count, const char* partida_actual) {
    // ESTA FUNCIÓN SOLO SE LLAMA DESDE ESTADOS DE MENÚ
    // NO desde estados de juego
    
    switch (estado) {
        case MENU_MAIN:
            dibujar_menu_principal(seleccion);
            break;
            
        case MENU_SELECT_GAME:
            dibujar_seleccion_partida(seleccion, partidas, count);
            break;
            
        case MENU_PLAYING:
            // EL HUD del jugador se dibuja en dibujar_ui() dentro de dibujar_escena_completa()
            break;
            
        case MENU_SPECTATING:
            // EL HUD del espectador se dibuja en dibujar_ui() dentro de dibujar_escena_completa()
            break;
    }
}