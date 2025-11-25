#include "conexion.h"
#include "game.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Incluir el adapter
#include "../Server/AdapterC.h"

// AISLAR winsock con defines ANTES de incluirlo
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

// Variables globales de conexión
static SOCKET socket_servidor = INVALID_SOCKET;
bool servidor_conectado = false;
static WSADATA wsaData;

// ==================== FUNCIONES DE CONEXIÓN ====================

bool conectar_servidor(const char* ip) {
    printf("🔌 Conectando al servidor en %s:25557...\n", ip);
    
    // Inicializar Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        printf("❌ Error inicializando Winsock\n");
        return false;
    }

    // Crear Socket
    socket_servidor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_servidor == INVALID_SOCKET) {
        printf("❌ Error creando socket\n");
        WSACleanup();
        return false;
    }

    // Configurar dirección del servidor
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(25557);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        // Intentar resolver por nombre de host
        struct hostent *host = gethostbyname(ip);
        if (host == NULL) {
            printf("❌ Error resolviendo host: %s\n", ip);
            closesocket(socket_servidor);
            WSACleanup();
            return false;
        }
        server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    }

    // Conectar
    if(connect(socket_servidor, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("❌ Error conectando al servidor %s\n", ip);
        closesocket(socket_servidor);
        WSACleanup();
        return false;
    }

    servidor_conectado = true;
    printf("✅ ¡Conectado al servidor!\n");
    
    // Identificación inicial
    int tipo_servidor;
    if (adapter_receive_identification(socket_servidor, &tipo_servidor) > 0) {
        printf("📡 Servidor identificado como: %s\n", adapter_get_client_type_name(tipo_servidor));
    }
    
    // Enviar identificación como JUGADOR
    int mi_tipo = 1; // CLIENT_JUGADOR
    if (adapter_send_identification(socket_servidor, mi_tipo) > 0) {
        printf("👤 Identificado como: %s\n", adapter_get_client_type_name(mi_tipo));
    }

    return true;
}

void desconectar_servidor(void) {
    if (servidor_conectado) {
        closesocket(socket_servidor);
        WSACleanup();
        servidor_conectado = false;
        printf("🔌 Desconectado del servidor\n");
    }
}

// ==================== FUNCIONES DE SERIALIZACIÓN JSON ====================

bool serializar_input_a_json(TipoCliente client_type, int player_id, const char* game_id,
                            const char* input_type, const char* key, PaqueteJSON *paquete) {
    // Crear JSON manualmente
    const char* client_type_str = (client_type == CLIENT_PLAYER) ? "PLAYER" : "SPECTATOR";
    
    // Obtener timestamp actual
    time_t timestamp = time(NULL);
    
    // Calcular tamaño necesario
    size_t buffer_size = 256; // Tamaño base
    if (game_id) buffer_size += strlen(game_id);
    if (key) buffer_size += strlen(key);
    
    // Allocar memoria
    paquete->json_data = (char*)malloc(buffer_size);
    if (!paquete->json_data) return false;
    
    // Formatear JSON
    snprintf(paquete->json_data, buffer_size,
        "{\n"
        "  \"client_type\": \"%s\",\n"
        "  \"player_id\": %d,\n"
        "  \"game_id\": \"%s\",\n"
        "  \"input_type\": \"%s\",\n"
        "  \"key\": \"%s\",\n"
        "  \"timestamp\": %lld\n" 
        "}",
        client_type_str, player_id, game_id ? game_id : "default", 
        input_type, key ? key : "UNKNOWN", timestamp);
    
    paquete->json_size = strlen(paquete->json_data);
    printf("📤 JSON Input generado: %s\n", paquete->json_data);
    return true;
}

// ==================== FUNCIONES DE COMUNICACIÓN PRINCIPALES ====================

bool enviar_input_al_servidor(TipoCliente client_type, int player_id, const char* game_id,
                             const char* input_type, const char* key) {
    if (!servidor_conectado) {
        printf("⚠️  Servidor no conectado, input ignorado: %s\n", key);
        return false;
    }
    
    PaqueteJSON paquete;
    if (!serializar_input_a_json(client_type, player_id, game_id, input_type, key, &paquete)) {
        printf("❌ Error serializando input a JSON\n");
        return false;
    }
    
    // Enviar tamaño primero
    adapter_send_int(socket_servidor, (int)paquete.json_size);
    
    // Enviar datos JSON
    int bytes_sent = send(socket_servidor, paquete.json_data, (int)paquete.json_size, 0);
    
    liberar_paquete_json(&paquete);
    
    if (bytes_sent == SOCKET_ERROR) {
        printf("❌ Error enviando JSON al servidor\n");
        return false;
    }
    
    printf("✅ Input enviado al servidor: %s - %s\n", input_type, key);
    return true;
}

bool recibir_estado_actualizado(EstadoJuego *estado) {
    if (!servidor_conectado) {
        printf("⚠️  Servidor no conectado, no se puede recibir estado\n");
        return false;
    }
    
    printf("🔄 Esperando estado actualizado del servidor...\n");
    
    // Recibir tamaño del JSON
    int json_size;
    if (adapter_receive_int(socket_servidor, &json_size) <= 0) {
        printf("❌ Error recibiendo tamaño del JSON\n");
        return false;
    }
    
    if (json_size <= 0 || json_size > 100000) { // Límite razonable
        printf("❌ Tamaño de JSON inválido: %d\n", json_size);
        return false;
    }
    
    // Recibir datos JSON
    char* json_buffer = (char*)malloc(json_size + 1);
    if (!json_buffer) {
        printf("❌ Error allocando memoria para JSON\n");
        return false;
    }
    
    int bytes_received = recv(socket_servidor, json_buffer, json_size, 0);
    if (bytes_received != json_size) {
        printf("❌ Error recibiendo datos JSON: %d/%d bytes\n", bytes_received, json_size);
        free(json_buffer);
        return false;
    }
    
    json_buffer[json_size] = '\0'; // Null-terminate
    printf("📥 JSON recibido (%d bytes)\n", json_size);
    
    // Deserializar JSON a estado
    bool resultado = deserializar_json_a_estado(json_buffer, estado);
    
    free(json_buffer);
    return resultado;
}

// ==================== PARSER JSON SIMPLIFICADO ====================

// Función auxiliar para extraer valores string del JSON
static char* extraer_string_json(const char *json, const char *clave) {
    char patron[100];
    snprintf(patron, sizeof(patron), "\"%s\":\"", clave);
    
    const char *inicio = strstr(json, patron);
    if (!inicio) return NULL;
    
    inicio += strlen(patron);
    const char *fin = strchr(inicio, '"');
    if (!fin) return NULL;
    
    size_t longitud = fin - inicio;
    char *resultado = (char*)malloc(longitud + 1);
    strncpy(resultado, inicio, longitud);
    resultado[longitud] = '\0';
    
    return resultado;
}

// Función auxiliar para extraer valores numéricos del JSON
static float extraer_float_json(const char *json, const char *clave) {
    char patron[100];
    snprintf(patron, sizeof(patron), "\"%s\":", clave);
    
    const char *inicio = strstr(json, patron);
    if (!inicio) return 0.0f;
    
    inicio += strlen(patron);
    return atof(inicio);
}

static int extraer_int_json(const char *json, const char *clave) {
    char patron[100];
    snprintf(patron, sizeof(patron), "\"%s\":", clave);
    
    const char *inicio = strstr(json, patron);
    if (!inicio) return 0;
    
    inicio += strlen(patron);
    return atoi(inicio);
}

static bool extraer_bool_json(const char *json, const char *clave) {
    char patron[100];
    snprintf(patron, sizeof(patron), "\"%s\":", clave);
    
    printf("Buscando clave '%s' con patrón: '%s'\n", clave, patron);

    const char *inicio = strstr(json, patron);
    if (!inicio) {
        printf("❌ Clave '%s' no encontrada en JSON\n", clave);
        // Vamos a mostrar el JSON completo para debug
        printf("📄 JSON completo (primeros 500 chars):\n%.500s\n", json);
        return false;
    }
    
    inicio += strlen(patron);
     printf("✅ Clave '%s' encontrada, valor empieza en: '%.50s'\n", clave, inicio);
    
    //Buscar "true" o "false"
    if (strncmp(inicio, "true", 4) == 0) {
        printf(" %s: true\n", clave);
        return true;
    } else if (strncmp(inicio, "false", 5) == 0) {
        printf(" %s: false\n", clave);
        return false;
    }

    //Si no encuentra true/false, buscar como numero(0 o 1)
    if (strncmp(inicio, "1", 1) == 0) {
        printf(" %s: true\n", clave);
        return true;
    } else if (strncmp(inicio, "0", 1) == 0) {
        printf(" %s: false\n", clave);
        return false;
    }
    
    // Mostrar exactamente qué hay después de la clave
    printf("❌ Valor no reconocido para '%s'. Contenido: '", clave);
    for (int i = 0; i < 20 && inicio[i] != '\0' && inicio[i] != ',' && inicio[i] != '}'; i++) {
        printf("%c", inicio[i]);
    }
    printf("'\n");


    return false;
}

// ==================== DESERIALIZACIÓN JSON COMPLETA ====================

bool deserializar_json_a_estado(const char *json_data, EstadoJuego *estado) {
    printf("🔧 Deserializando JSON del servidor...\n");
    
    if (!json_data || strlen(json_data) == 0) {
        printf("❌ JSON vacío o nulo\n");
        return false;
    }

    printf("📄 JSON COMPLETO RECIBIDO:\n%s\n", json_data);
    printf("📏 Longitud del JSON: %zu caracteres\n", strlen(json_data));
    
    // 1. GAME ID y TIMESTAMP (información general)
    char* game_id = extraer_string_json(json_data, "game_id");
    if (game_id) {
        printf("🎮 Partida: %s\n", game_id);
        free(game_id);
    }
    
    // 2. PLAYER DATA
    estado->jugador.x = extraer_float_json(json_data, "x");
    estado->jugador.y = extraer_float_json(json_data, "y");
    estado->jugador.vidas = extraer_int_json(json_data, "lives");
    estado->jugador.puntuacion = extraer_int_json(json_data, "score");
    estado->jugador.activo = extraer_bool_json(json_data, "active");
    
    // Estado del jugador desde JSON
    char* estado_str = extraer_string_json(json_data, "state");
    if (estado_str) {
        if (strcmp(estado_str, "CLIMBING") == 0) estado->jugador.estado = ESTADO_AGARRADO_LIANA;
        else if (strcmp(estado_str, "JUMPING") == 0) estado->jugador.estado = ESTADO_SALTANDO;
        else if (strcmp(estado_str, "FALLING") == 0) estado->jugador.estado = ESTADO_CAYENDO;
        else estado->jugador.estado = ESTADO_SUELO;
        free(estado_str);
    }
    
    // 3. PADRE (Donkey Kong)
    estado->padre.activo = true; // Siempre activo por ahora
    
    // 4. JUEGO ACTIVO
    estado->juego_activo = extraer_bool_json(json_data, "game_active");
    
    printf("✅ JSON deserializado - Jugador: (%.1f, %.1f), Vidas: %d, Puntos: %d, Estado: %d\n",
           estado->jugador.x, estado->jugador.y, estado->jugador.vidas, 
           estado->jugador.puntuacion, estado->jugador.estado, 
           estado->juego_activo ? "SI" : "NO");
    
    return true;
}

// ==================== FUNCIONES UTILITARIAS ====================

const char* estado_jugador_a_string(EstadoPlayerJSON estado) {
    switch (estado) {
        case PLAYER_STANDING: return "STANDING";
        case PLAYER_MOVING_LEFT: return "MOVING_LEFT";
        case PLAYER_MOVING_RIGHT: return "MOVING_RIGHT";
        case PLAYER_CLIMBING: return "CLIMBING";
        case PLAYER_JUMPING: return "JUMPING";
        case PLAYER_FALLING: return "FALLING";
        default: return "UNKNOWN";
    }
}

EstadoPlayerJSON estado_jugador_desde_string(const char* estado_str) {
    if (!estado_str) return PLAYER_STANDING;
    if (strcmp(estado_str, "STANDING") == 0) return PLAYER_STANDING;
    if (strcmp(estado_str, "MOVING_LEFT") == 0) return PLAYER_MOVING_LEFT;
    if (strcmp(estado_str, "MOVING_RIGHT") == 0) return PLAYER_MOVING_RIGHT;
    if (strcmp(estado_str, "CLIMBING") == 0) return PLAYER_CLIMBING;
    if (strcmp(estado_str, "JUMPING") == 0) return PLAYER_JUMPING;
    if (strcmp(estado_str, "FALLING") == 0) return PLAYER_FALLING;
    return PLAYER_STANDING;
}

const char* tipo_enemigo_a_string(TipoEnemigo tipo) {
    return (tipo == ENEMY_RED_CROCODILE) ? "RED_CROCODILE" : "BLUE_CROCODILE";
}

const char* tipo_fruta_a_string(TipoFruta tipo) {
    switch (tipo) {
        case FRUIT_BANANA: return "BANANA";
        case FRUIT_APPLE: return "APPLE";
        case FRUIT_PEAR: return "PEAR";
        case FRUIT_ORANGE: return "ORANGE";
        default: return "UNKNOWN";
    }
}

void liberar_paquete_json(PaqueteJSON *paquete) {
    if (paquete->json_data) {
        free(paquete->json_data);
        paquete->json_data = NULL;
        paquete->json_size = 0;
    }
}