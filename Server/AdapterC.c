#include "AdapterC.h"
#include <stdio.h>

// Incluir winsock2 SOLO en el .c, no en el .h
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

int adapter_send_int(SOCKET socket, int value) {
    //Convertir a formato de red
    int network_value = htonl(value);
    int result = send(socket, (char*)&network_value, sizeof(network_value), 0);
    if (result > 0) {
        printf("Adapter C: Enviado entero -> %d\n", value);
    } 
    return result;
}

int adapter_receive_int(SOCKET socket, int* value) {
    int network_value;
    int result = recv(socket, (char*)&network_value, sizeof(network_value), 0);
    if (result > 0) {
        *value = ntohl(network_value);
        printf("Adapter C: Recibido entero -> %d\n", *value);
    }
    return result;
}

int adapter_send_identification(SOCKET socket, int client_type) {
    return adapter_send_int(socket, client_type);
}

int adapter_receive_identification(SOCKET socket, int* client_type) {
    return adapter_receive_int(socket, client_type);
}

const char* adapter_get_client_type_name(int client_type) {
    switch (client_type) {
    case CLIENT_JUGADOR: return "JUGADOR";
    case CLIENT_ESPECTADOR: return "ESPECTADOR";
    case CLIENT_ADMIN: return "ADMIN";
    case CLIENT_SERVIDOR: return "SERVIDOR";
    default: return "DESCONOCIDO";
    }
}