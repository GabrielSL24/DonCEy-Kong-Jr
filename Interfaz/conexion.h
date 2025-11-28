#ifndef CONEXION_H
#define CONEXION_H

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
bool enviar_input_al_servidor(TipoCliente client_type,  const char* game_id, 
                             const char* input_type, const char* key);
bool recibir_estado_actualizado(EstadoJuego *estado);

// ==================== FUNCIONES DE SERIALIZACIÓN/DESERIALIZACIÓN ====================
bool serializar_input_a_json(TipoCliente client_type,  const char* game_id,
                            const char* input_type, const char* key, PaqueteJSON *paquete);
bool deserializar_json_a_estado(const char *json_data, EstadoJuego *estado);

// ==================== FUNCIONES UTILITARIAS JSON ====================
const char* estado_jugador_a_string(EstadoPlayerJSON estado);
EstadoPlayerJSON estado_jugador_desde_string(const char* estado_str);
const char* tipo_enemigo_a_string(TipoEnemigo tipo);
const char* tipo_fruta_a_string(TipoFruta tipo);
void liberar_paquete_json(PaqueteJSON *paquete);

// ==================== FUNCIONES PARA JUGADOR ====================
bool crear_nueva_partida(const char* game_id);
bool unirse_partida_jugador(const char* game_id);
bool salir_partida_jugador(const char* game_id);
bool confirmar_inicio_partida(const char* game_id);

// ==================== FUNCIONES PARA ESPECTADOR ====================
bool enviar_solicitud_espectador(TipoRequest request_type, const char* game_id);
bool solicitar_actualizacion_lista_partidas(void);
bool solicitar_lista_partidas(void);
bool unirse_partida_espectador(const char* game_id);
bool salir_partida_espectador(const char* game_id);
bool parsear_lista_partidas(const char* json_str, InfoPartida partidas[], int* count);

// ==================== FUNCIONES DE HANDSHAKE ====================
bool esperar_respuesta_servidor(void);
bool procesar_respuesta_servidor(EstadoJuego *estado);
bool esta_en_partida_activa(void);
void set_partida_activa(bool activa);

// ==================== VARIABLE GLOBAL ====================
extern bool servidor_conectado;
extern InfoPartida partidas_disponibles[10];
extern int cantidad_partidas;
extern EstadoMenu estado_menu_actual;
extern char partida_seleccionada_global[50];

#endif