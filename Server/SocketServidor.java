import java.net.*;
import java.io.*;
import java.util.*;
import javax.print.DocFlavor.STRING;


public class SocketServidor {    
    private List<ClientHandler> clientes = new ArrayList<>();
    private Map<String, Partida> partidas = new HashMap<>();
    //private Admin adminPanel;


    // Clase interna para representar una partida
    public static class Partida {
        public String gameId;
        public ClientHandler jugador;
        public List<ClientHandler> espectadores = new ArrayList<>();
        public boolean activa = false;
        public GameLogic logica;

        public Partida(String gameId, ClientHandler jugador) {
            this.gameId = gameId;
            this.jugador = jugador;
            this.logica = new GameLogic();
        }
    }


    // Metodo para gestionar partidas
    public synchronized String crearPartida(ClientHandler jugador) {
        String gameId = "partida_" + System.currentTimeMillis();
        Partida nuevaPartida = new Partida(gameId, jugador);
        partidas.put(gameId, nuevaPartida);
        System.out.println("Nueva partida creada: " + gameId);
        return gameId;
    }

    public synchronized boolean unirJugadorAPartida(String gameId, ClientHandler jugador) {
        Partida partida = partidas.get(gameId);
        if (partida != null && partida.jugador == null) {
            partida.jugador = jugador;
            return true;
        }
        return false;
    } 

    public synchronized boolean unirEspectadorAPartida(String gameId, ClientHandler espectador) {
        Partida partida = partidas.get(gameId);
        if (partida != null) {
            partida.espectadores.add(espectador);
            return true;
        }
        return false;
    }

    public synchronized Partida obtenerPartida(String gameId) {
        return partidas.get(gameId);
    }

    public synchronized List<InfoPartida> obtenerListaPartidas() {
        List<InfoPartida> lista = new ArrayList<>();
        for (Partida partida : partidas.values()) {
            InfoPartida info = new InfoPartida();
            info.gameId = partida.gameId;
            info.playerCount = (partida.jugador != null) ? 1 : 0;
            info.spectators = partida.espectadores.size();
            info.active = partida.activa;
            lista.add(info);
        }
        return lista;
    }

    // Clase para informacion de partida
    public static class InfoPartida {
        public String gameId;
        public int playerCount;
        public int spectators;
        public boolean active;
    }

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

            //adminPanel = new Admin();


            while (true) {
                Socket client = serverSocket.accept();
                String clientIP = client.getInetAddress().getHostAddress();
                System.out.println("🔗 Nuevo cliente conectado desde: " + clientIP);
                
                ClientHandler clientHandler = new ClientHandler(client, this);
                //clientes.add(clientHandler);
                new Thread(clientHandler).start();

                //if (adminPanel != null) {
                //    adminPanel.addClient(clientHandler.getClientId(), clientIP, "CONECTADO");
                //}

                //System.out.println("Juegos activos: " + clientes.size());
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void removeClient(ClientHandler client) {
        clientes.remove(client);
        System.out.println("➖ Cliente desconectado. Juegos activos: " + clientes.size());

        //if (adminPanel != null) {
        //    adminPanel.removeClient(client.getClientId());
        //}
    }

    public List<ClientHandler> getClientes() {
        return clientes;
    }  
    
    public synchronized void eliminarPartida(String gameId) {
        partidas.remove(gameId);
        System.out.println("Partida eliminada: " + gameId);
    }

    public synchronized void removerJugadorDePartida(String gameId, ClientHandler jugador) {
        Partida partida = partidas.get(gameId);
        if (partida != null && partida.jugador == jugador) {
            partida.jugador = null;
            // Si no hay jugador ni espectadores, eliminar partida
            if (partida.espectadores.isEmpty()) {
                eliminarPartida(gameId);
            }
        }
    }

    public synchronized void removerEspectadorDePartida(String gameId, ClientHandler espectador) {
        Partida partida = partidas.get(gameId);
        if (partida != null) {
            partida.espectadores.remove(espectador);
            // Si no hay jugador ni espectadores, eliminar partida
            if (partida.jugador == null && partida.espectadores.isEmpty()) {
                eliminarPartida(gameId);
            }
        }
    }
}

