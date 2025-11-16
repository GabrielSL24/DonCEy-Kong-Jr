
//Funciones para que un cliente pueda abrir sockets con un servidor.
#ifndef _SOCKET_CLIENTE_H
#define _SOCKET_CLIENTE_H

#include <WinSock2.h>
#include <WS2tcpip.h>

SOCKET Abre_Conexion_Inet(char *Host_Servidor, char *Servicio);

#endif
