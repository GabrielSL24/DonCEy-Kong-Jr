#ifndef MESSAGE_ADAPTER_H
#define MESSAGE_ADAPTER_H

#include <WinSock2.h>

#define CLIENT_JUGADOR 1
#define CLIENT_ESPECTADOR 2
#define CLIENT_ADMIN 3
#define CLIENT_SERVIDOR 4

int adapter_send_int(SOCKET socket, int value);
int adapter_receive_int(SOCKET socket, int* value);
int adapter_send_identification(SOCKET socket, int clientType);
int adapter_receive_identification(SOCKET socket, int* clientType);
const char* adapter_get_client_type_name(int client_type);

#endif