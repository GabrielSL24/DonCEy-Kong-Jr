#ifndef CONEXION_H
#define CONEXION_H

#include "game.h"
#include "types.h"
#include <stdbool.h>
#include <stddef.h>

// Estructura para paquete JSON
typedef struct {
    char* json_data;
    size_t json_size;
} PaqueteJSON;

// ==================== FUNCIONES DE CONEXIÓN ====================
bool conectar_servidor(const char* ip);
void desconectar_servidor(void);

// ==================== FUNCIONES DE COMUNICACIÓN JSON ====================
bool enviar_input_al_servidor(TipoCliente client_type, int player_id, const char* game_id, 
                             const char* input_type, const char* key);
bool recibir_estado_actualizado(EstadoJuego *estado);

// ==================== FUNCIONES DE SERIALIZACIÓN/DESERIALIZACIÓN ====================
bool serializar_input_a_json(TipoCliente client_type, int player_id, const char* game_id,
                            const char* input_type, const char* key, PaqueteJSON *paquete);
bool deserializar_json_a_estado(const char *json_data, EstadoJuego *estado);

// ==================== FUNCIONES UTILITARIAS JSON ====================
const char* estado_jugador_a_string(EstadoPlayerJSON estado);
EstadoPlayerJSON estado_jugador_desde_string(const char* estado_str);
const char* tipo_enemigo_a_string(TipoEnemigo tipo);
const char* tipo_fruta_a_string(TipoFruta tipo);
void liberar_paquete_json(PaqueteJSON *paquete);

// ==================== VARIABLE GLOBAL ====================
extern bool servidor_conectado;

#endif