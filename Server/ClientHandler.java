import java.net.*;
import java.io.*;
import java.util.*;

import java.util.concurrent.atomic.AtomicReference;

public class ClientHandler implements Runnable {
    private Socket socket;
    private SocketServidor server;
    private AdapterJ adapter;
    private String clientType;
    private int clientId;
    private static int nextId = 1;

    //Simulacion
    private float playerX = 100.0f;
    private float playerY = 300.0f;
    private int playerLives = 3;
    private int playerScore = 0;
    private String playerState = "STANDING";
    private boolean gameActive = true;
    private String gameId = "partida_" + System.currentTimeMillis();

    //private DataInputStream input;
    //private DataOutputStream output;
    //private GameLogic gameLogic;

    public ClientHandler(Socket socket, SocketServidor server) {
        this.socket = socket;
        this.server = server;
        //this.gameLogic = new GameLogic();
        this.clientId = nextId++;
    }
    
    @Override
    public void run() {
        try {
            //input = new DataInputStream(socket.getInputStream());
            //output = new DataOutputStream(socket.getOutputStream());

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
                //if (input != null) input.close();
                //if (output != null) output.close();
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
        
        System.out.println("Enviando estado inicial al jugador " + clientId);
        String jsonEstadoInicial = generarEstadoJuego();
        adapter.sendString(jsonEstadoInicial);
        System.out.println("Estado inicial enviado al jugador " + clientId);

        long ultimoEnvio = System.currentTimeMillis();

        while (gameActive && !socket.isClosed()) {
            try {

                //Verificar si hay datos disponibles
                if (adapter.hayDatosDisponibles()) {

                    // 1. Recibir JSON de input del cliente
                    String jsonInput = adapter.receiveString();
                    System.out.println("JSON recibido del cliente:");
                    System.out.println("   " + jsonInput);


                    // 2. Procesar input (simulacion)
                    procesarInput(jsonInput);
                }

                 // 2. ENVIAR ESTADO PERIÓDICAMENTE (cada 100ms) incluso sin inputs
                long ahora = System.currentTimeMillis();
                if (ahora - ultimoEnvio >= 100) { // 10 FPS para el estado
                    String jsonEstado = generarEstadoJuego();
                    adapter.sendString(jsonEstado);
                    System.out.println("📤 Estado enviado al cliente (posición: " + playerX + ", " + playerY + ")");
                    ultimoEnvio = ahora;
                }

                Thread.sleep(10); // Simular tiempo de frame

            } catch (Exception e) {
                System.out.println("Error en handleJugador " + e.getMessage());
                e.printStackTrace();
                break;
            }
        }
        System.out.println("Cliente " + clientId + " finalizado");    
    }

   

    private void procesarInput(String jsonInput) {
        try {
            // Parseaer JSON manualmente
            String inputType = extraerValor(jsonInput, "input_type");
            String key = extraerValor(jsonInput, "key");

            System.out.println("Procesando input: " + inputType + " - " + key);

            //Simulacion de logica de juego
            if("KEY_PRESSED".equals(inputType)) {
                switch (key) {
                    case "LEFT":
                        playerX -= 5.0f;
                        playerState = "MOVING_LEFT";
                        break;
                    case "RIGHT":
                        playerX += 5.0f;
                        playerState = "MOVING_RIGHT";
                        break;
                    case "UP":
                        playerY -= 5.0f;
                        playerState = "CLIMBING";
                        break;
                    case "DOWN":
                        playerY += 5.0f;
                        playerState = "FALLING";
                        break;
                    case "JUMP":
                        playerY -= 10.0f; // Simular salto
                        playerState = "JUMPING";
                        break;
                }
            } else if ("KEY_RELEASED".equals(inputType)) {
                if ("LEFT".equals(key) || "RIGHT".equals(key)) {
                    playerState = "STANDING";
                }
            }

            //Simular recoleccion de frutas y colisiones
            playerScore += 10;
            if (playerScore > 1000) {
                playerScore = 0;
                playerLives++;
            }

            //Limitar poscion (bordes)
            if (playerX < 0) playerX = 0;
            if (playerX > 800) playerX = 800;
            if (playerY < 0) playerY = 0;
            if (playerY > 600) {
                playerY = 600;
                playerLives--;
                if (playerLives <= 0) {
                    gameActive = false;
                    System.out.println("Jugador " + clientId + " ha perdido todas las vidas. Fin del juego.");
                }
            }

        } catch (Exception e) {
            System.out.println("Error procesando input JSON: " + e.getMessage());
        }
    }

    private String extraerValor(String json, String clave) {
        String patron = "\"" + clave + "\":\"";
        int inicio = json.indexOf(patron);
        if (inicio == -1) return "";

        inicio += patron.length();
        int fin = json.indexOf("\"", inicio);
        if (fin == -1) return "";

        return json.substring(inicio, fin);
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
        json.append("      \"x\": ").append(playerX).append(",\n");
        json.append("      \"y\": ").append(playerY).append(",\n");
        json.append("      \"state\":\"").append(playerState).append("\",\n");
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
        System.out.println("📤 JSON generado para cliente:");
        System.out.println(resultado);
        System.out.println("📏 Tamaño: " + resultado.length() + " caracteres");
        
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