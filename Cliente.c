#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string.h>

#include "Socket_Cliente.h"
#include "Socket.h"
#include "AdapterC.h"

#pragma comment(lib, "ws2_32.lib")
int main (int argc, char *argv[])
{
	//Descriptor del socket y buffer para datos
	SOCKET Socket_Con_Servidor;
   	int valor_recibido, valor_enviado;
    int client_type;

    char server_ip[100] = "localhost";

    if (__argc == 2) {
        strcpy_s(server_ip, sizeof(server_ip), __argv[1]);
    }

    printf("🎮 Cliente DonCEy Kong Jr\n");
    printf("🔌 Conectando a: %s:25557\n", server_ip);

	// Inicializar Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
		printf("Error incializando Winsock\n");
		return -1;
	}

	//Conectar al servidor
	Socket_Con_Servidor = Abre_Conexion_Inet (server_ip, "25557");
	if (Socket_Con_Servidor == INVALID_SOCKET) {
		        printf("No se pudo conectar al servidor: %s\n", server_ip);
        printf("💡 Verifica:\n");
        printf("   - Servidor ejecutándose\n");
        printf("   - ZeroTier conectado\n");
        printf("   - IP correcta del servidor\n");
		WSACleanup();
		return -1;
	}

    printf("Conexion establecida con el servidor\n");

   // 1. Recibir identificacion del servidor
   if (adapter_receive_identification(Socket_Con_Servidor, &client_type) > 0) {
		printf("Servidor: %s\n", adapter_get_client_type_name(client_type));
   }
   
    // 2. ENVIAR nuestra identificacion como JUGADOR
    int mi_tipo = CLIENT_JUGADOR;
    if (adapter_send_identification(Socket_Con_Servidor, mi_tipo) > 0) {
         printf("Conectado como: %s\n", adapter_get_client_type_name(mi_tipo));
    }

    // 3. Comunicación básica de prueba
    printf("Probando comunicación...\n");

    // Recibir confirmación del servidor
    if (adapter_receive_int(Socket_Con_Servidor, &valor_recibido) > 0) {
        printf("Confirmación servidor: %d\n", valor_recibido);
    }

    // Enviar algunos datos de prueba
    for (int i = 1; i <= 3; i++) {
        printf("Enviando dato: %d\n", i);
        adapter_send_int(Socket_Con_Servidor, i);
        
        if (adapter_receive_int(Socket_Con_Servidor, &valor_recibido) > 0) {
            printf("Respuesta: %d\n", valor_recibido);
        }
        
        Sleep(2000); // Esperar 2 segundos
    }

    //Cerrar conexion
    closesocket(Socket_Con_Servidor);
    WSACleanup();

    printf("Cliente finalizado\n");
    return 0;
}
