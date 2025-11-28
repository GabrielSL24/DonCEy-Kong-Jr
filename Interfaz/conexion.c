#include "conexion.h"
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


// ==================== DEFINICIÓN DE VARIABLES GLOBALES ====================
bool servidor_conectado = false;
InfoPartida partidas_disponibles[10];
int cantidad_partidas = 0;
EstadoMenu estado_menu_actual = MENU_MAIN;
char partida_seleccionada_global[50] = "";

// Variable para controlar estado de partida (EVITA SPAM)
static bool partida_activa = false;
static char* extraer_string_json(const char *json, const char *clave);
static float extraer_float_json(const char *json, const char *clave);
static int extraer_int_json(const char *json, const char *clave);
static bool extraer_bool_json(const char *json, const char *clave);

// Variables globales de conexión
static SOCKET socket_servidor = INVALID_SOCKET;
static WSADATA wsaData;

// ==================== INICIALIZACIÓN DE PARTIDAS DISPONIBLES ====================
void inicializar_partidas_disponibles(void) {
    cantidad_partidas = 0;
    for (int i = 0; i < 10; i++) {
        partidas_disponibles[i].game_id[0] = '\0';
        partidas_disponibles[i].player_count = 0;
        partidas_disponibles[i].spectators = 0;
        partidas_disponibles[i].active = false;
    }
}

// ==================== CONTROL DE PARTIDA ACTIVA ====================
bool esta_en_partida_activa(void) {
    return partida_activa;
}

void set_partida_activa(bool activa) {
    partida_activa = activa;
    printf("🎮 Partida %s\n", activa ? "ACTIVADA" : "DESACTIVADA");
}

// ==================== FUNCIONES DE CONEXIÓN ====================
bool conectar_servidor(const char* ip) {
    printf("🔌 Conectando al servidor en %s:25557...\n", ip);

    inicializar_partidas_disponibles();
    set_partida_activa(false); // Asegurar que empiece desactivada

    // ... (el resto de la función conectar_servidor se mantiene igual)
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
        set_partida_activa(false); // Desactivar partida al desconectar
        closesocket(socket_servidor);
        WSACleanup();
        servidor_conectado = false;
        printf("🔌 Desconectado del servidor\n");
    }
}


// ==================== FUNCIONES DE SERIALIZACIÓN JSON ====================

bool serializar_input_a_json(TipoCliente client_type, const char* game_id,
                            const char* input_type, const char* key, PaqueteJSON *paquete) {
    // Crear JSON manualmente
    const char* client_type_str = (client_type == CLIENT_PLAYER) ? "PLAYER" : "SPECTATOR";
    
    // Obtener timestamp actual
    time_t timestamp = time(NULL);
    
    // Calcular tamaño necesario
    size_t buffer_size = 300; // Aumentado por el nuevo campo
    if (game_id) buffer_size += strlen(game_id);
    if (key) buffer_size += strlen(key);
    
    // Allocar memoria
    paquete->json_data = (char*)malloc(buffer_size);
    if (!paquete->json_data) return false;
    
    // Formatear JSON CON request_type
    snprintf(paquete->json_data, buffer_size,
        "{\n"
        "  \"client_type\": \"%s\",\n"
        "  \"game_id\": \"%s\",\n"
        "  \"request_type\": \"GAME_INPUT\",\n"  // ← NUEVO CAMPO
        "  \"input_type\": \"%s\",\n"
        "  \"key\": \"%s\",\n"
        "  \"timestamp\": %lld\n" 
        "}",
        client_type_str, game_id ? game_id : "default", 
        input_type, key ? key : "UNKNOWN", timestamp);
    
    paquete->json_size = strlen(paquete->json_data);
    printf("📤 JSON Input generado: %s\n", paquete->json_data);
    return true;
}

// ==================== FUNCIONES DE COMUNICACIÓN PRINCIPALES ====================

bool enviar_input_al_servidor(TipoCliente client_type, const char* game_id,
                             const char* input_type, const char* key) {
    if (!servidor_conectado || !partida_activa) {  // ← SOLO si partida activa
        printf("⚠️  Servidor no conectado o partida no activa, input ignorado: %s\n", key);
        return false;
    }
    
    PaqueteJSON paquete;
    if (!serializar_input_a_json(client_type, game_id, input_type, key, &paquete)) {
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
// ==================== FUNCIONES DE HANDSHAKE ====================

bool confirmar_inicio_partida(const char* game_id) {
    if (!servidor_conectado || !game_id) return false;
    
    time_t timestamp = time(NULL);
    size_t buffer_size = 512;
    char* json_data = (char*)malloc(buffer_size);
    
    if (!json_data) return false;
    
    snprintf(json_data, buffer_size,
        "{\n"
        "  \"client_type\": \"PLAYER\",\n"
        "  \"request_type\": \"START_GAME\",\n"
        "  \"game_id\": \"%s\",\n"
        "  \"timestamp\": %lld\n"
        "}",
        game_id, (long long)timestamp);
    
    printf("📤 Confirmando inicio de partida: %s\n", json_data);
    
    // Enviar tamaño primero
    adapter_send_int(socket_servidor, (int)strlen(json_data));
    
    // Enviar datos JSON
    int bytes_sent = send(socket_servidor, json_data, (int)strlen(json_data), 0);
    
    free(json_data);
    
    if (bytes_sent == SOCKET_ERROR) {
        printf("❌ Error confirmando inicio de partida\n");
        return false;
    }
    
    printf("✅ Inicio de partida confirmado: %s\n", game_id);
    return true;
}

// Función auxiliar para extraer valores string del JSON
static char* extraer_string_json(const char *json, const char *clave) {
    char patron[100];
    snprintf(patron, sizeof(patron), "\"%s\":", clave);

    printf("Buscando clave '%s' con patrón: '%s'\n", clave, patron);
    
    const char *inicio = strstr(json, patron);
    if (!inicio) {
        printf("Clave '%s' no encontrada en JSON\n", clave);
        return NULL;
    }
    
    inicio += strlen(patron);

    // Saltar espacios, tabs, newlines después de los dos puntos
    while (*inicio && (*inicio == ' ' || *inicio == '\t' || *inicio == '\n' || *inicio == '\r')) {
        inicio++;
    }

     if (*inicio == '\0') {
        printf("No hay valor después de la clave '%s'\n", clave);
        return NULL;
    }

    if (*inicio == '"') {
        // Valor entre comillas
        inicio++; // saltar la comilla inicial
        const char *fin = strchr(inicio, '"');
        if (!fin) {
            printf("No se encontró comilla de cierre para '%s'\n", clave);
            return NULL;
        }
        
        size_t longitud = fin - inicio;
        char *resultado = (char*)malloc(longitud + 1);
        if (!resultado) return NULL;
        
        strncpy(resultado, inicio, longitud);
        resultado[longitud] = '\0';
        
        printf("Clave '%s' encontrada: '%s'\n", clave, resultado);
        return resultado;
    } else {
        // Valor sin comillas (boolean, number, etc.)
        const char *fin = inicio;
        while (*fin && *fin != ',' && *fin != '}' && *fin != ' ' && *fin != '\t' && *fin != '\n' && *fin != '\r') {
            fin++;
        }
        
        size_t longitud = fin - inicio;
        char *resultado = (char*)malloc(longitud + 1);
        if (!resultado) return NULL;
        
        strncpy(resultado, inicio, longitud);
        resultado[longitud] = '\0';
        
        printf("Clave '%s' encontrada: '%s'\n", clave, resultado);
        return resultado;
    }
}

bool procesar_respuesta_servidor(EstadoJuego *estado) {
    if (!servidor_conectado || !hay_datos_disponibles()) {
        return false;
    }
    
    // Recibir tamaño del JSON
    int json_size;
    int result = adapter_receive_int(socket_servidor, &json_size);
    
    if (result <= 0) {
        printf("🔌 Conexión perdida o error recibiendo tamaño del JSON\n");
        servidor_conectado = false;
        return false;
    }
    
    // Recibir datos JSON
    char* json_buffer = (char*)malloc(json_size + 1);
    if (!json_buffer) {
        printf("Error allocando memoria para JSON\n");
        return false;
    }
    
    int bytes_received = recv(socket_servidor, json_buffer, json_size, 0);
    if (bytes_received != json_size) {
         printf("Error recibiendo datos JSON: esperados %d, recibidos %d\n", json_size, bytes_received);
        free(json_buffer);
        return false;
    }
    
    json_buffer[json_size] = '\0';
    printf("Mensaje del servidor (%d bytes): %s\n", json_size, json_buffer);
    
    // Determinar tipo de mensaje por response_type
    char* response_type = extraer_string_json(json_buffer, "response_type");
    
    if (response_type) {
        printf("Tipo de respuesta: %s\n", response_type);
        
        if (strcmp(response_type, "GAME_LIST") == 0) {
            printf("Procesando lista de partidas...\n");
            bool resultado = parsear_lista_partidas(json_buffer, partidas_disponibles, &cantidad_partidas);
            free((void*)response_type);
            free(json_buffer);
            return resultado;
        }
        else if (strcmp(response_type, "GAME_CREATED") == 0) {
            printf("Partida creada confirmada por servidor\n");
            
            // Extraer game_id de la respuesta
            char* game_id_respuesta = extraer_string_json(json_buffer, "game_id");
            if (game_id_respuesta) {
                strcpy(partida_seleccionada_global, game_id_respuesta);
                free(game_id_respuesta);
            }
            
            set_partida_activa(true);
            free(response_type);
            free(json_buffer);
            return true;
        }
        else if (strcmp(response_type, "GAME_JOINED") == 0) {
            printf("Unido a partida confirmado por servidor\n");
            set_partida_activa(true);
            free(response_type);
            free(json_buffer);
            return true;
        }
        else if (strcmp(response_type, "GAME_STARTED") == 0) {
            printf("Partida iniciada confirmada por servidor\n");
            set_partida_activa(true);
            free(response_type);
            free(json_buffer);
            return true;
        }
        else if (strcmp(response_type, "SPECTATOR_JOINED") == 0) {
            printf("Espectador unido confirmado por servidor\n");
            set_partida_activa(true);
            free(response_type);
            free(json_buffer);
            return true;
        }
        else if (strcmp(response_type, "GAME_STATE") == 0) {
            if (partida_activa) {
                printf("Procesando estado del juego...\n");
                bool resultado = deserializar_json_a_estado(json_buffer, estado);
                free(response_type);
                free(json_buffer);
                return resultado;
            } else {
                printf("Ignorando GAME_STATE - partida no activa\n");
                free(response_type);
                free(json_buffer);
                return false;
            }
        }
        else if (strcmp(response_type, "ERROR") == 0) {
            char* error_code = extraer_string_json(json_buffer, "error_code");
            char* error_msg = extraer_string_json(json_buffer, "message");
            printf("Error del servidor: %s - %s\n", error_code ? error_code : "UNKNOWN", error_msg ? error_msg : "No message");
            if (error_code) free(error_code);
            if (error_msg) free(error_msg);
            free(response_type);
            free(json_buffer);
            return false;
        }
        else if (strcmp(response_type, "GAME_LEFT") == 0) {
            printf("🎮 Partida terminada - desconectando...\n");
            set_partida_activa(false);
            free(response_type);
            free(json_buffer);
            return false; // Indicar que la partida terminó
        }
        else {
            printf("Respuesta no manejada: %s\n", response_type);
            free(response_type);
            free(json_buffer);
            return false;
        }
    } else {
         printf("Mensaje sin response_type, mostrando JSON completo:\n%s\n", json_buffer);
        // Intentar ver si es un mensaje de error antiguo
        if (strstr(json_buffer, "error") != NULL || strstr(json_buffer, "ERROR") != NULL) {
            printf("🔍 Parece ser un mensaje de error\n");
        }
    }
    
    free(json_buffer);
    return false;
}

bool recibir_estado_actualizado(EstadoJuego *estado) {
    // Esta función ahora usa procesar_respuesta_servidor
    return procesar_respuesta_servidor(estado);
}


// ==================== PARSER JSON SIMPLIFICADO ====================

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

    // 1. Extraer datos del jugador directamente del JSON principal
    // Buscar el objeto "player" dentro de "changes"
    const char* player_start = strstr(json_data, "\"player\"");
    if (!player_start) {
        printf("❌ No se encontró objeto 'player' en JSON\n");
        return false;
    }
    
    // Buscar el inicio del objeto player { ... }
    player_start = strchr(player_start, '{');
    if (!player_start) {
        printf("❌ No se encontró inicio del objeto player\n");
        return false;
    }
    
    // Extraer datos del jugador desde el objeto player
    estado->jugador.x = extraer_float_json(player_start, "x");
    estado->jugador.y = extraer_float_json(player_start, "y");
    estado->jugador.vidas = extraer_int_json(player_start, "lives");
    estado->jugador.puntuacion = extraer_int_json(player_start, "score");
    estado->jugador.activo = extraer_bool_json(player_start, "active");
    
    // Estado del jugador desde JSON - BUSCAR DENTRO DEL OBJETO PLAYER
    char* estado_str = extraer_string_json(player_start, "state");
    if (estado_str) {
        printf("🎯 Estado del jugador recibido: %s\n", estado_str);
        if (strcmp(estado_str, "CLIMBING") == 0) estado->jugador.estado = ESTADO_AGARRADO_LIANA;
        else if (strcmp(estado_str, "JUMPING") == 0) estado->jugador.estado = ESTADO_SALTANDO;
        else if (strcmp(estado_str, "FALLING") == 0) estado->jugador.estado = ESTADO_CAYENDO;
        else estado->jugador.estado = ESTADO_SUELO;
        free(estado_str);
    } else {
        printf("⚠️  No se pudo extraer estado del jugador\n");
        estado->jugador.estado = ESTADO_SUELO;
    }
    
    // 2. PADRE (Donkey Kong) - por ahora siempre activo
    estado->padre.activo = true;
    
    // 3. JUEGO ACTIVO
    estado->juego_activo = extraer_bool_json(json_data, "game_active");
    
    printf("✅ JSON deserializado - Jugador: (%.1f, %.1f), Vidas: %d, Puntos: %d, Estado: %d\n",
           estado->jugador.x, estado->jugador.y, estado->jugador.vidas, 
           estado->jugador.puntuacion, estado->jugador.estado);
    
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

// ==================== FUNCIONES  PARA ESPECTADOR ====================

bool enviar_solicitud_espectador(TipoRequest request_type, const char* game_id) {
    if (!servidor_conectado) {
        printf("⚠️  Servidor no conectado\n");
        return false;
    }
    
    const char* request_type_str;
    switch (request_type) {
        case REQUEST_LIST_GAMES: request_type_str = "LIST_GAMES"; break;
        case REQUEST_JOIN_GAME: request_type_str = "JOIN_GAME"; break;
        case REQUEST_LEAVE_GAME: request_type_str = "LEAVE_GAME"; break;
        default: return false;
    }
    
    time_t timestamp = time(NULL);
    size_t buffer_size = 512;
    char* json_data = (char*)malloc(buffer_size);
    
    if (!json_data) return false;
    
    snprintf(json_data, buffer_size,
        "{\n"
        "  \"client_type\": \"SPECTATOR\",\n"
        "  \"request_type\": \"%s\",\n"
        "  \"game_id\": \"%s\",\n"
        "  \"timestamp\": %lld\n"
        "}",
        request_type_str, game_id ? game_id : "", (long long)timestamp);
    
    printf("📤 JSON Espectador: %s\n", json_data);
    
    // Enviar tamaño primero
    adapter_send_int(socket_servidor, (int)strlen(json_data));
    
    // Enviar datos JSON
    int bytes_sent = send(socket_servidor, json_data, (int)strlen(json_data), 0);
    
    free(json_data);
    
    if (bytes_sent == SOCKET_ERROR) {
        printf("❌ Error enviando solicitud de espectador\n");
        return false;
    }
    
    printf("✅ Solicitud de espectador enviada: %s\n", request_type_str);
    return true;
}

bool solicitar_lista_partidas(void) {
    return enviar_solicitud_espectador(REQUEST_LIST_GAMES, NULL);
}

bool unirse_partida_espectador(const char* game_id) {
    if (!game_id) return false;
    return enviar_solicitud_espectador(REQUEST_JOIN_GAME, game_id);
}

bool salir_partida_espectador(const char* game_id) {
    if (!game_id) return false;
    return enviar_solicitud_espectador(REQUEST_LEAVE_GAME, game_id);
}

bool parsear_lista_partidas(const char* json_str, InfoPartida partidas[], int* count) {
    printf("🔧 Parseando lista de partidas...\n");
    
    // Buscar el array de games
    const char* games_start = strstr(json_str, "\"games\"");
    if (!games_start) {
        printf("❌ No se encontró array de games en JSON\n");
        return false;
    }
    
    games_start = strchr(games_start, '[');
    if (!games_start) return false;
    games_start++; // Saltar '['
    
    int partida_count = 0;
    const char* current = games_start;
    
    while (*current && *current != ']' && partida_count < 10) {
        // Buscar cada objeto de partida entre { }
        const char* obj_start = strchr(current, '{');
        const char* obj_end = strchr(current, '}');
        
        if (!obj_start || !obj_end || obj_start > obj_end) break;
        
        // Extraer game_id
        const char* id_start = strstr(obj_start, "\"game_id\"");
        if (id_start && id_start < obj_end) {
            id_start = strchr(id_start, '"');
            if (id_start) id_start++;
            const char* id_end = strchr(id_start, '"');
            if (id_end && id_end < obj_end) {
                strncpy(partidas[partida_count].game_id, id_start, id_end - id_start);
                partidas[partida_count].game_id[id_end - id_start] = '\0';
            }
        }
        
        // Extraer player_count
        const char* players_start = strstr(obj_start, "\"player_count\"");
        if (players_start && players_start < obj_end) {
            players_start = strchr(players_start, ':');
            if (players_start) partidas[partida_count].player_count = atoi(players_start + 1);
        }
        
        // Extraer spectators  
        const char* specs_start = strstr(obj_start, "\"spectators\"");
        if (specs_start && specs_start < obj_end) {
            specs_start = strchr(specs_start, ':');
            if (specs_start) partidas[partida_count].spectators = atoi(specs_start + 1);
        }
        
        // Extraer active
        const char* active_start = strstr(obj_start, "\"active\"");
        if (active_start && active_start < obj_end) {
            active_start = strchr(active_start, ':');
            if (active_start) {
                if (strstr(active_start, "true")) partidas[partida_count].active = true;
                else partidas[partida_count].active = false;
            }
        }
        
        printf("🎮 Partida %d: %s (J:%d, E:%d, A:%s)\n",
               partida_count, partidas[partida_count].game_id,
               partidas[partida_count].player_count,
               partidas[partida_count].spectators,
               partidas[partida_count].active ? "SI" : "NO");
        
        partida_count++;
        current = obj_end + 1;
    }
    
    *count = partida_count;
    return partida_count > 0;
}

bool crear_nueva_partida(const char* game_id) {
    if (!servidor_conectado || !game_id) return false;
    
    time_t timestamp = time(NULL);
    size_t buffer_size = 512;
    char* json_data = (char*)malloc(buffer_size);
    
    if (!json_data) return false;
    
    snprintf(json_data, buffer_size,
        "{\n"
        "  \"client_type\": \"PLAYER\",\n"
        "  \"request_type\": \"CREATE_GAME\",\n"
        "  \"game_id\": \"%s\",\n"
        "  \"timestamp\": %lld\n"
        "}",
        game_id, (long long)timestamp);
    
    printf("📤 Creando nueva partida: %s\n", json_data);
    
    // Enviar tamaño primero
    adapter_send_int(socket_servidor, (int)strlen(json_data));
    
    // Enviar datos JSON
    int bytes_sent = send(socket_servidor, json_data, (int)strlen(json_data), 0);
    
    free(json_data);
    
    if (bytes_sent == SOCKET_ERROR) {
        printf("❌ Error creando partida\n");
        return false;
    }
    
    printf("✅ Partida creada: %s\n", game_id);
    return true;
}

bool unirse_partida_jugador(const char* game_id) {
    if (!servidor_conectado || !game_id) return false;
    
    time_t timestamp = time(NULL);
    size_t buffer_size = 512;
    char* json_data = (char*)malloc(buffer_size);
    
    if (!json_data) return false;
    
    snprintf(json_data, buffer_size,
        "{\n"
        "  \"client_type\": \"PLAYER\",\n"
        "  \"request_type\": \"JOIN_GAME\",\n"
        "  \"game_id\": \"%s\",\n"
        "  \"timestamp\": %lld\n"
        "}",
        game_id, (long long)timestamp);
    
    printf("📤 Uniéndose a partida como jugador: %s\n", json_data);
    
    // Enviar tamaño primero
    adapter_send_int(socket_servidor, (int)strlen(json_data));
    
    // Enviar datos JSON
    int bytes_sent = send(socket_servidor, json_data, (int)strlen(json_data), 0);
    
    free(json_data);
    
    if (bytes_sent == SOCKET_ERROR) {
        printf("❌ Error uniéndose a partida\n");
        return false;
    }
    
    printf("✅ Unido a partida como jugador: %s\n", game_id);
    return true;
}

bool salir_partida_jugador(const char* game_id) {
    if (!servidor_conectado || !game_id) return false;
    
    time_t timestamp = time(NULL);
    size_t buffer_size = 512;
    char* json_data = (char*)malloc(buffer_size);
    
    if (!json_data) return false;
    
    snprintf(json_data, buffer_size,
        "{\n"
        "  \"client_type\": \"PLAYER\",\n"
        "  \"request_type\": \"LEAVE_GAME\",\n"
        "  \"game_id\": \"%s\",\n"
        "  \"timestamp\": %lld\n"
        "}",
        game_id, (long long)timestamp);
    
    printf("📤 Saliendo de partida como jugador: %s\n", json_data);
    
    // Enviar tamaño primero
    adapter_send_int(socket_servidor, (int)strlen(json_data));
    
    // Enviar datos JSON
    int bytes_sent = send(socket_servidor, json_data, (int)strlen(json_data), 0);
    
    free(json_data);
    
    if (bytes_sent == SOCKET_ERROR) {
        printf("❌ Error saliendo de partida\n");
        return false;
    }
    
    printf("✅ Salido de partida como jugador: %s\n", game_id);
    return true;
}

bool solicitar_actualizacion_lista_partidas(void) {
    return solicitar_lista_partidas();  // Alias por ahora
}

bool hay_datos_disponibles(void) {
    if (!servidor_conectado) return false;
    
    fd_set readfds;
    struct timeval timeout;
    
    FD_ZERO(&readfds);
    FD_SET(socket_servidor, &readfds);
    
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    
    int result = select(0, &readfds, NULL, NULL, &timeout);
    return result > 0 && FD_ISSET(socket_servidor, &readfds);
}