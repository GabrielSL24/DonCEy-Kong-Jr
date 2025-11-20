#include "conexion.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Incluir el adapter
#include "../Server/AdapterC.h"

// AISLAR winsock con defines ANTES de incluirlo
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

//Variables globales de conexion
static SOCKET socket_servidor = INVALID_SOCKET;
bool servidor_conectado = false;
static WSADATA wsaData;


bool conectar_servidor(const char* ip) {
    printf("Conectando al servidor en %s:25557...\n", ip);
    
    //Inicializar Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        printf("Error incializando Winsock\n");
        return false;
    }

    //Crear Socket
    socket_servidor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_servidor == INVALID_SOCKET) {
        printf("Error creando socket\n");
        WSACleanup();
        return false;
    }

    //Configurar direccion del servidor
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(25557);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        //Intentar resolver por nombre de host
        struct hostent *host = gethostbyname(ip);
        if (host == NULL) {
            printf("Error resolviendo host: %s\n", ip);
            closesocket(socket_servidor);
            WSACleanup();
            return false;
        }
        server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    }

    //Conectar
    if(connect(socket_servidor, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Error conectando al servidor %s\n", ip);
        closesocket(socket_servidor);
        WSACleanup();
        return false;
    }

    servidor_conectado = true;
    printf("¡Conectado al servidor!\n");
    printf("Enviando identificación como JUGADOR...\n");

    // 1. Recibir identificación del servidor
    int tipo_servidor;
    if (adapter_receive_identification(socket_servidor, &tipo_servidor) > 0) {
        printf("Servidor identificado como: %s\n", adapter_get_client_type_name(tipo_servidor));
    }
    
    // 2. Enviar nuestra identificación como JUGADOR
    int mi_tipo = 1; // CLIENT_JUGADOR
    if (adapter_send_identification(socket_servidor, mi_tipo) > 0) {
        printf("Identificado como: %s\n", adapter_get_client_type_name(mi_tipo));
    }

    return true;
}


bool enviar_estado_actual_al_servidor(int matriz_x, int matriz_y, int vidas, int puntuacion) {
    if (!servidor_conectado) return false;
    
    printf("📤 Enviando estado MATRIZ: [%d,%d] Vidas:%d Puntos:%d\n", 
           matriz_x, matriz_y, vidas, puntuacion);
    
    // Enviar coordenadas de matriz en lugar de coordenadas del mundo
    adapter_send_int(socket_servidor, matriz_x);
    adapter_send_int(socket_servidor, matriz_y);
    adapter_send_int(socket_servidor, vidas);
    adapter_send_int(socket_servidor, puntuacion);
    
    return true;
}


bool recibir_consecuencias_del_servidor(int *vidas, int *puntuacion, bool *juego_activo) {
    if (!servidor_conectado) return false;

    printf("🔄 Esperando consecuencias del servidor...\n");
    
    int temp_vidas, temp_puntos, temp_activo;
    
    // ✅ DEBUG: Mostrar qué estamos recibiendo exactamente
    printf("🔍 Intentando recibir 3 enteros...\n");
    
    int resultado1 = adapter_receive_int(socket_servidor, &temp_vidas);
    int resultado2 = adapter_receive_int(socket_servidor, &temp_puntos);
    int resultado3 = adapter_receive_int(socket_servidor, &temp_activo);
    
    printf("🔍 Resultados recepción: %d, %d, %d\n", resultado1, resultado2, resultado3);
    printf("🔍 Valores recibidos: vidas=%d, puntos=%d, activo=%d\n", 
           temp_vidas, temp_puntos, temp_activo);
    
    if (resultado1 > 0 && resultado2 > 0 && resultado3 > 0) {
        *vidas = temp_vidas;
        *puntuacion = temp_puntos;
        *juego_activo = (temp_activo != 0);
        
        printf("✅ Consecuencias aplicadas - Vidas:%d Puntos:%d Activo:%d\n", 
               *vidas, *puntuacion, *juego_activo);
        
        return true;
    }
    
    printf("❌ Error recibiendo consecuencias\n");
    return false;
}


void desconectar_servidor(void) {
    if (servidor_conectado) {
        closesocket(socket_servidor);
        WSACleanup();
        servidor_conectado = false;
        printf("Desconectado del servidor\n");
    }
}