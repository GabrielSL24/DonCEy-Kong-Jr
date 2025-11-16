#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>

#include "Socket.h"

#pragma comment(lib, "ws2_32.lib")

/*
* Lee datos del socket. Supone que se le pasa un buffer con hueco 
*	suficiente para los datos. Devuelve el numero de bytes leidos o
* 0 si se cierra fichero o -1 si hay error.
*/
int Lee_Socket (SOCKET fd, char *Datos, int Longitud) {
	int Leido = 0;
	int Aux = 0;

	//Comprobacion de que los parametros de entrada son correctos
	if ((fd == INVALID_SOCKET) || (Datos == NULL) || (Longitud < 1))
		return -1;

	//Mientras no hayamos leido todos los datos solicitados
	while (Leido < Longitud) {
		Aux = recv(fd, Datos + Leido, Longitud - Leido, 0);
		if (Aux > 0) {
			//Si hemos conseguido leer datos, incrementamos la variable
			//que contiene los datos leidos hasta el momento
			Leido = Leido + Aux;
		}
		else {
			//Si recv devuelve 0, es que se ha cerrado el socket. Devolvemos
			//los caracteres leidos hasta ese momento
			if (Aux == 0) 
				return Leido;
			if (Aux == SOCKET_ERROR) {
				int error = WSAGetLastError();
				switch (error) {
					case WSAEINTR:
					case WSAEWOULDBLOCK:
						Sleep(1);  // 1ms en lugar de usleep
						break;
					default:
						return -1;
				}
			}
		}
	}

	//Se devuelve el total de los caracteres leidos
	return Leido;
}

/*
* Escribe dato en el socket cliente. Devuelve numero de bytes escritos,
* o -1 si hay error.
*/
int Escribe_Socket (SOCKET fd, char *Datos, int Longitud)
{
	int Escrito = 0;
	int Aux = 0;

	//Comprobacion de los parametros de entrada
	if ((fd == INVALID_SOCKET) || (Datos == NULL) || (Longitud < 1))
		return -1;

	//Bucle hasta que hayamos escrito todos los caracteres que nos han
	//indicado.
	while (Escrito < Longitud) {
		Aux = send(fd, Datos + Escrito, Longitud - Escrito, 0);
		if (Aux > 0)
		{
			//Si hemos conseguido escribir caracteres, se actualiza la
			//variable Escrito
			Escrito = Escrito + Aux;
		}
		else {
			//Si se ha cerrado el socket, devolvemos el numero de caracteres leidos.
			//Si ha habido error, devolvemos -1
			if (Aux == 0)
				return Escrito;
			else
				return -1;
		}
	}

	//Devolvemos el total de caracteres leidos
	return Escrito;
}

/**
 * Lee un mensaje de un socket udp. Los parámetros que se pasan son:
 * - Descriptor fd del socket del que se quiere leer.
 * - Estructura Remoto, en la que se devolverá los datos del que ha enviado el
 * mensaje que acabamos de leer.
 * - Longitud_Remoto de la estructura anterior. Debe pasarse relleno con el tamaño
 * de Remoto y se devolverá rellena con el tamaño de los datos en Remoto.
 * - Buffer de Datos donde se quiere que aparezca el mensaje.
 * - Longitud_Datos del buffer anterior.
 *
 * Devuelve el número de bytes leidos o -1 en caso de error.
 */
int Lee_Socket_Udp (
	SOCKET fd, struct sockaddr *Remoto, int *Longitud_Remoto,
	char *Datos, int Longitud_Datos)
{
	int Leido = 0;
	int Aux = 0;

	//Comprobacion de que los parametros de entrada son correctos
	if ((fd == INVALID_SOCKET) || (Datos == NULL) || (Longitud_Datos < 1)
			|| (Remoto == NULL) || (Longitud_Remoto == NULL))
	{
		return -1;
	}

	//Mientras no hayamos leido todos los datos solicitados
	while (Leido < Longitud_Datos)
	{
		Aux = recvfrom(fd, Datos + Leido, Longitud_Datos - Leido, 0,
			Remoto, Longitud_Remoto);

		if (Aux > 0)
		{
			//Si hemos conseguido leer datos, incrementamos la variable
			//que contiene los datos leidos hasta el momento
			Leido = Leido + Aux;
		}
		else
		{
			//Si recvfrom devuelve 0, es que se ha cerrado el socket. Devolvemos
			//los caracteres leidos hasta ese momento
			if (Aux == 0) 
				return Leido;
			if (Aux == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				switch (error)
				{
					case WSAEINTR:
					case WSAEWOULDBLOCK:
						Sleep(1);
						break;
					default:
						return -1;
				}
			}
		}
	}
	//Se devuelve el total de los caracteres leidos
	return Leido;
}

/**
 * Abre un socket INET con un servidor que esté corriendo en Host_Servidor y que atienda
 * al servicio cuyo nombre es Servicio. 
 * En Windows, Servicio puede ser el número de puerto directamente.
 */
SOCKET Abre_Conexion_Inet (char *Host_Servidor, char *Servicio)
{
    WSADATA wsaData;
    SOCKET sd;
    struct sockaddr_in server_addr;
    struct hostent *host;
    int port;
    
    // Inicializar Winsock si no está inicializado
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return INVALID_SOCKET;
    }
    
    // Crear socket
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == INVALID_SOCKET) {
        WSACleanup();
        return INVALID_SOCKET;
    }
    
    // Convertir servicio/puerto a número
    port = atoi(Servicio);
    if (port == 0) {
        closesocket(sd);
        WSACleanup();
        return INVALID_SOCKET;
    }
    
    // Obtener información del host
    host = gethostbyname(Host_Servidor);
    if (host == NULL) {
        closesocket(sd);
        WSACleanup();
        return INVALID_SOCKET;
    }
    
    // Configurar estructura de dirección del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    memset(&(server_addr.sin_zero), 0, 8);
    
    // Conectar al servidor
    if (connect(sd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == SOCKET_ERROR) {
        closesocket(sd);
        WSACleanup();
        return INVALID_SOCKET;
    }
    
    return sd;
}

/**
* Escribe dato en el socket cliente. Devuelve numero de bytes escritos,
* o -1 si hay error.
* - fd es el descriptor del socket.
* - Remoto es el destinatario del mensaje, a quién se lo queremos enviar. 
* - Longitud_Remoto es el tamaño de Remoto en bytes.
* - Datos es el mensaje que queremos enviar.
* - Longitud_Datos es el tamaño del mensaje en bytes.
*/
int Escribe_Socket_Udp (SOCKET fd, struct sockaddr *Remoto, 
	int Longitud_Remoto, char *Datos, int Longitud_Datos)
{
	int Escrito = 0;
	int Aux = 0;

	//Comprobacion de los parametros de entrada
	if ((fd == INVALID_SOCKET) || (Datos == NULL) || (Longitud_Datos < 1) 
			|| (Remoto == NULL) )
   {
		return -1;
   }
	//Bucle hasta que hayamos escrito todos los caracteres que nos han
	//indicado.
	while (Escrito < Longitud_Datos)
	{
		Aux = sendto(fd, Datos + Escrito, Longitud_Datos - Escrito, 0,
			Remoto, Longitud_Remoto);

		if (Aux > 0)
		{
			//Si hemos conseguido escribir caracteres, se actualiza la
			//variable Escrito
			Escrito = Escrito + Aux;
		}
		else
		{
			//Si se ha cerrado el socket, devolvemos el numero de caracteres leidos.
			//Si ha habido error, devolvemos -1
			if (Aux == 0)
				return Escrito;
			else
         {
				return -1;
         }
		}
	}
	//Devolvemos el total de caracteres leidos
	return Escrito;
}


/**
 * Rellena una estructura sockaddr_in con los datos que se le pasan. Esta estrutura es
 * útil para el envio o recepción de mensajes por sockets Udp o para abrir conexiones.
 * Se le pasa el host. Puede ser NULL (para abrir socket servidor Udp o para recepción de
 * mensajes de cualquier host).
 * Se le pasa el servicio. Puede ser NULL (para abrir socket cliente Udp).
 * Se le pasa una estructura sockaddr_in que devolverá rellena.
 * Se le pasa una Longitud. Debe contener el tamaño de la estructura sockaddr_in y
 * devolverá el tamaño de la estructura una vez rellena.
 * Devuelve -1 en caso de error.
 */
int Dame_Direccion_Udp (char *Host, char *Servicio, struct sockaddr_in *Servidor,
   int *Longitud_Servidor)
{
   struct servent *Puerto;
   struct hostent *Maquina;
   int port;
  
   //Comprobación de parámetros
   if (Servidor == NULL) return -1;

   //Relleno del primer campo de la estructura
   Servidor->sin_family = AF_INET;

   //Si nos han pasado un host ...
   if (Host != NULL)
   {
      //... obtenemos la dirección del host y la ponemos en la estructura
      Maquina = gethostbyname(Host);
      if (Maquina == NULL)
         return -1;

      memcpy(&Servidor->sin_addr, Maquina->h_addr, Maquina->h_length);
   }
   else
      //Si no nos han pasado un host, ponemos cualquier host.
      Servidor->sin_addr.s_addr = INADDR_ANY;

   //Si servicio en NULL, hacemos que el puerto lo eliga el sistema operativo libremente.
   if (Servicio == NULL)
      Servidor->sin_port = 0;
   else
   {
      port = atoi(Servicio);
      if (port == 0)
      {
         Puerto = getservbyname(Servicio, "udp");
         if (Puerto == NULL)
            return -1;
         Servidor->sin_port = Puerto->s_port;
      }
      else
      {
         Servidor->sin_port = htons(port);
      }
   }
   

   return 0;
}