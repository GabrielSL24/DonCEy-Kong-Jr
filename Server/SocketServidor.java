import java.net.*;
import java.io.*;
import java.util.*;

import javax.print.DocFlavor.STRING;


public class SocketServidor {    
    private List<ClientHandler> clientes = new ArrayList<>();

    public static void main (String [] args) {
        new SocketServidor();
    }
    
    public SocketServidor() {
        try {
            ServerSocket serverSocket = new ServerSocket(25557);
            System.out.println("Servidor ZeroTer iniciado");
            System.out.println("Escuchando en puerto 25557");
            System.out.println("PARA CONECTAR CLIENTES:");
            System.out.println("- Misma PC: .\\cliente.exe");
            System.out.println("- Otra PC: .\\cliente.exe 10.147.17.196");
            System.out.println("Esperando conexiones...");

            while (true) {
                Socket client = serverSocket.accept();
                String clientIP = client.getInetAddress().getHostAddress();
                System.out.println("🔗 Nuevo cliente conectado desde: " + clientIP);
                
                ClientHandler clientHandler = new ClientHandler(client, this);
                clientes.add(clientHandler);
                new Thread(clientHandler).start();

                 System.out.println("🎯 Juegos activos: " + clientes.size());
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void removeClient(ClientHandler client) {
        clientes.remove(client);
        System.out.println("➖ Cliente desconectado. Juegos activos: " + clientes.size());
    }
}

