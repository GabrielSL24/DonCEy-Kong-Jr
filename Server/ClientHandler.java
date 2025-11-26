import java.net.*;
import java.io.*;
import java.util.*;

public class ClientHandler implements Runnable {
    private Socket socket;
    private SocketServidor server;
    private AdapterJ adapter;
    private String clientType;
    private int clientId;
    private static int nextId = 1;

    // Usar GameLogic en lugar de variables sueltas
    private GameLogic gameLogic;
    private String gameId = "partida_" + System.currentTimeMillis();
    
    // Variables que aún necesitamos para el estado del juego
    private int playerLives = 3;
    private int playerScore = 0;
    private boolean gameActive = true;

    public ClientHandler(Socket socket, SocketServidor server) {
        this.socket = socket;
        this.server = server;
        this.gameLogic = new GameLogic();
        this.clientId = nextId++;
    }
    
    @Override
    public void run() {
        try {
            adapter = new AdapterJ(socket);
            socket.setSoLinger(true, 10);

            System.out.println("Iniciando comunicacion con cliente " + clientId);

            // 1. Enviar identificacion del servidor
            adapter.sendIdentification("SERVIDOR");;

            // 2. Recibir identificación del cliente
            clientType = adapter.receiveIdentification();
            System.out.println("Cliente " + clientId + " es " + clientType + " desde " + socket.getInetAddress().getHostAddress());
        
            // 3. Manejar segun el tipo de cliente
            switch (clientType) {
                case "JUGADOR":
                    handleJugador();
                    break;
                case "ESPECTADOR":
                    handleEspectador();
                    break;
                case "ADMIN":
                    handleAdmin();
                    break;
                default:
                    System.out.println("Tipo de cliente desconocido: " + clientType);
            }     
        } catch (Exception e){
            System.out.println("Error con cliente " + clientId + ": " + e.getMessage());
        } finally {
            try {
                if (adapter != null) adapter.close();
                if (socket != null) socket.close();
                server.removeClient(this);
                System.out.println("Cliente " + clientId + " desconectado");
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private void handleJugador() throws IOException {
        System.out.println("Jugador " + clientId + " listo - INICIANDO PARTIDA: " + gameId);
        
        // CONFIGURAR TIMEOUT en el socket
        socket.setSoTimeout(50); // 50ms timeout para receive
        
        System.out.println("Enviando estado inicial al jugador " + clientId);
        String jsonEstadoInicial = generarEstadoJuego();
        adapter.sendString(jsonEstadoInicial);
        System.out.println("Estado inicial enviado al jugador " + clientId);

        long ultimoEnvio = System.currentTimeMillis();

        while (gameActive && !socket.isClosed()) {
            try {
                // Intentar recibir con timeout
                try {
                    String jsonInput = adapter.receiveString();
                    System.out.println("JSON recibido del cliente:");
                    System.out.println("   " + jsonInput);

                    procesarInput(jsonInput);
                    System.out.println("Input procesado - nueva posicion: " + 
                                    gameLogic.getPlayerX() + ", " + gameLogic.getPlayerY());
                    
                } catch (SocketTimeoutException e) {
                    // Timeout normal, no hay datos disponibles
                    // System.out.println("Timeout - no hay datos"); // Opcional: muy verbose
                }

                // ENVIAR ESTADO PERIÓDICAMENTE (cada 100ms)
                long ahora = System.currentTimeMillis();
                if (ahora - ultimoEnvio >= 100) {
                    String jsonEstado = generarEstadoJuego();
                    adapter.sendString(jsonEstado);
                    System.out.println("Estado enviado - pos: " + 
                                    gameLogic.getPlayerX() + ", " + gameLogic.getPlayerY());
                    ultimoEnvio = ahora;
                }

                Thread.sleep(10);

            } catch (Exception e) {
                if (!(e instanceof SocketTimeoutException)) {
                    System.out.println("Error en handleJugador: " + e.getMessage());
                    break;
                }
            }
        }
        System.out.println("Cliente " + clientId + " finalizado");    
    }

    private void procesarInput(String jsonInput) {
        try {
            String inputType = extraerValor(jsonInput, "input_type");
            String key = extraerValor(jsonInput, "key");

            System.out.println("Procesando input: " + inputType + " - " + key);
            
            // DEBUG: mostrar el JSON completo para verificar
            System.out.println("JSON completo: " + jsonInput);

            // DELEGAR A LA LOGICA
            gameLogic.processInput(inputType, key);

        } catch (Exception e) {
            System.out.println("Error procesando input JSON: " + e.getMessage());
            e.printStackTrace();
        }
    }

    private String extraerValor(String json, String clave) {
        // Buscar el patron con la clave
        String patron = "\"" + clave + "\":";
        int inicio = json.indexOf(patron);
        if (inicio == -1) return "";
        
        inicio += patron.length();
        
        // Buscar el inicio del valor (saltar espacios)
        while (inicio < json.length() && Character.isWhitespace(json.charAt(inicio))) {
            inicio++;
        }
        
        if (inicio >= json.length()) return "";
        
        // Determinar si el valor es string (entre comillas) o numero/bool
        if (json.charAt(inicio) == '"') {
            // Valor es string entre comillas
            inicio++; // saltar la comilla inicial
            int fin = json.indexOf("\"", inicio);
            if (fin == -1) return "";
            return json.substring(inicio, fin);
        } else {
            //Valor es numero o bool
            int fin = json.indexOf(",", inicio);
            if (fin == -1) fin = json.indexOf("}", inicio);
            if (fin == -1) return "";
            
            String valor = json.substring(inicio, fin).trim();
            //Remueve comilla final si existe
            if (valor.endsWith("\"")) {
                valor = valor.substring(0, valor.length() - 1);
            }
            return valor;
        }
    }

    private String generarEstadoJuego() {
        // Generar JSON manualmente
        StringBuilder json = new StringBuilder();

        json.append("{\n");
        json.append("  \"game_id\":\"").append(gameId).append("\",\n");
        json.append("  \"timestamp\":").append(System.currentTimeMillis()).append(",\n");
        json.append("  \"game_active\":").append(gameActive).append(",\n");
        json.append("  \"changes\":{\n");
        json.append("    \"player\":{\n");
        // USAR DATOS DE gameLogic
        json.append("      \"x\": ").append(gameLogic.getPlayerX()).append(",\n");
        json.append("      \"y\": ").append(gameLogic.getPlayerY()).append(",\n");
        json.append("      \"state\":\"").append(gameLogic.getPlayerState()).append("\",\n");
        json.append("      \"lives\":").append(playerLives).append(",\n");
        json.append("      \"score\":").append(playerScore).append(",\n");
        json.append("      \"active\":").append(gameActive).append("\n");
        json.append("    },\n");
        json.append("    \"enemies\":[\n");
        json.append("      {\n");
        json.append("        \"id\":\"croco1\",\n");
        json.append("        \"type\":\"RED_CROCODILE\",\n");
        json.append("        \"x\":200.0,\n");
        json.append("        \"y\":150.0,\n");
        json.append("        \"active\":true\n");
        json.append("      }\n");
        json.append("    ],\n");
        json.append("    \"fruits\":[\n");
        json.append("      {\n");
        json.append("        \"id\":\"fruit1\",\n");
        json.append("        \"type\":\"BANANA\",\n");
        json.append("        \"points\":100,\n");
        json.append("        \"x\":350.0,\n");
        json.append("        \"y\":220.0,\n");
        json.append("        \"active\":true\n");
        json.append("      }\n");
        json.append("    ]\n");
        json.append("  }\n");
        json.append("}");
        
        String resultado = json.toString();
        System.out.println("JSON generado para cliente:");
        System.out.println(resultado);
        System.out.println("Tamaño: " + resultado.length() + " caracteres");
        
        return resultado;
    }

    private void handleEspectador() throws IOException {
        System.out.println("Espectador " + clientId + " observando");

        adapter.sendInt(200);

        for (int i = 0; i < 10; i++) {
            adapter.sendInt(i);
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                System.out.println("Sleep interrumpido");
                break;
            }
        }
    }

    private void handleAdmin() throws IOException {
        System.out.println("Admin " + clientId + " conectado");

        adapter.sendInt(300);

        // Recibir algunos comandos de ejemplo
        for (int i = 0; i < 3; i++) {
            int comando = adapter.receiveInt();
            System.out.println("Admin " + clientId + " envió comando: " + comando);
            adapter.sendInt(comando + 1000); // Confirmación
        }
    }
}