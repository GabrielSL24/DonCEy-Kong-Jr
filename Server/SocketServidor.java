import java.net.*;
import java.io.*;
import java.util.*;
import javax.print.DocFlavor.STRING;


public class SocketServidor {    
    private List<ClientHandler> clientes = new ArrayList<>();
    private Map<String, Partida> partidas = new HashMap<>();
    private Admin adminPanel;

    public static void main (String [] args) {
        new SocketServidor();
    }
    
    public SocketServidor() {
        try {
            ServerSocket serverSocket = new ServerSocket(25557);
            System.out.println("Servidor ZeroTer iniciado");
            System.out.println("Escuchando en puerto 25557");
            System.out.println("Esperando conexiones...");

            adminPanel = new Admin(this);


            while (true) {
                Socket client = serverSocket.accept();
                String clientIP = client.getInetAddress().getHostAddress();
                System.out.println("🔗 Nuevo cliente conectado desde: " + clientIP);
                
                ClientHandler clientHandler = new ClientHandler(client, this);
                clientes.add(clientHandler);
                new Thread(clientHandler).start();

                if (adminPanel != null) {
                    adminPanel.addClient(clientHandler.getClientId(), clientIP, "CONECTADO");
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void removeClient(ClientHandler client) {
        clientes.remove(client);
        System.out.println("Cliente desconectado. Juegos activos: " + clientes.size());

        if (adminPanel != null) {
            adminPanel.removeClient(client.getClientId());
        }
    }

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
            System.out.println("🎮 GameLogic inicializado para partida: " + gameId);
        }
    }


    // === MÉTODOS PRINCIPALES ===

    public synchronized String crearPartida(ClientHandler jugador) {
        String gameId = "partida_" + System.currentTimeMillis();
        Partida nuevaPartida = new Partida(gameId, jugador);
        partidas.put(gameId, nuevaPartida);
        
        System.out.println("✅ NUEVA PARTIDA CREADA: " + gameId);
        System.out.println("📊 Total de partidas en mapa: " + partidas.size());
        System.out.println("👤 Jugador asignado: " + jugador.getClientId());
        
        // VERIFICACIÓN INMEDIATA - Debug crítico
        Partida verificada = partidas.get(gameId);
        System.out.println("🔍 Verificación inmediata: " + (verificada != null ? "EXISTE" : "NO EXISTE"));
        if (verificada != null) {
            System.out.println("   - GameId: " + verificada.gameId);
            System.out.println("   - Jugador: " + (verificada.jugador != null ? verificada.jugador.getClientId() : "null"));
        }
        
        return gameId;
    }
    
    public synchronized boolean unirJugadorAPartida(String gameId, ClientHandler jugador) {
        Partida partida = partidas.get(gameId);
        System.out.println("🔍 Buscando partida para unir jugador: " + gameId);
        System.out.println("📊 Partidas disponibles: " + partidas.keySet());
        
        if (partida != null && partida.jugador == null) {
            partida.jugador = jugador;
            System.out.println("✅ Jugador " + jugador.getClientId() + " unido a partida: " + gameId);
            return true;
        }
        System.out.println("❌ No se pudo unir jugador a partida: " + gameId);
        return false;
    }
    
    public synchronized boolean unirEspectadorAPartida(String gameId, ClientHandler espectador) {
        Partida partida = partidas.get(gameId);
        if (partida != null) {
            partida.espectadores.add(espectador);
            System.out.println("✅ Espectador " + espectador.getClientId() + " unido a partida: " + gameId);
            return true;
        }
        return false;
    }
    
    public synchronized Partida obtenerPartida(String gameId) {
        System.out.println("🔍 BUSCANDO PARTIDA: " + gameId);
        System.out.println("📊 Partidas en mapa: " + partidas.size());
        
        // Mostrar TODAS las partidas para debug
        if (partidas.isEmpty()) {
            System.out.println("   ⚠️  El mapa de partidas está VACÍO");
        } else {
            for (String key : partidas.keySet()) {
                Partida p = partidas.get(key);
                System.out.println("   - " + key + " -> Jugador: " + 
                    (p.jugador != null ? p.jugador.getClientId() : "null"));
            }
        }
        
        Partida partida = partidas.get(gameId);
        System.out.println("📋 Resultado búsqueda: " + (partida != null ? "ENCONTRADA" : "NO ENCONTRADA"));
        return partida;
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
            System.out.println("👤 Jugador removido de partida: " + gameId);
            if (partida.espectadores.isEmpty()) {
                eliminarPartida(gameId);
            }
        }
    }

    public synchronized void removerEspectadorDePartida(String gameId, ClientHandler espectador) {
        Partida partida = partidas.get(gameId);
        if (partida != null) {
            partida.espectadores.remove(espectador);
            System.out.println("👀 Espectador removido de partida: " + gameId);
            if (partida.jugador == null && partida.espectadores.isEmpty()) {
                eliminarPartida(gameId);
            }
        }
    }
}

