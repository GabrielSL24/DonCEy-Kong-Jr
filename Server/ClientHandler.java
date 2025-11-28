import java.net.Socket;
import java.util.List;
import java.io.IOException;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

/**
 * Maneja la comunicacion con solo un cliente.
 */
public class ClientHandler implements Runnable {
    private final Object socketLock = new Object();
    private final Socket socket;
    private final SocketServidor server;
    private AdapterJ adapter;
    private String clientType;
    private final int clientId;
    private static int nextId = 1;

    private final GameLogic gameLogic;
    private PlayerInput currentInput;
    private String playerState = "STANDING";

    private String gameId = null;
    private boolean enPartida = false;
    private boolean esEspectador = false;

    // Cola para comunicacion entre hilos
    private final BlockingQueue<String> inputQueue = new LinkedBlockingQueue<>();
    private volatile boolean running = true;

    public ClientHandler(Socket socket, SocketServidor server) {
        this.socket = socket;
        this.server = server;
        this.clientId = nextId++;
        this.gameLogic = new GameLogic();
        this.currentInput = new PlayerInput(false, false, false, false, false);
        
        System.out.println("NUEVO ClientHandler creado para cliente " + clientId);
    }

    @Override
    public void run() {
        try {
            adapter = new AdapterJ(socket);
            socket.setSoLinger(true, 10);
            socket.setSoTimeout(5000); // Aumentar timeout a 5 segundos

            System.out.println("Iniciando comunicacion con cliente " + clientId);

            // 1. Enviar identificacion del servidor
            adapter.sendIdentification("SERVIDOR");

            // 2. Recibir identificacion del cliente
            clientType = adapter.receiveIdentification();
            System.out.println("Cliente " + clientId + " es " + clientType);

            // 3. Bucle principal - UNICO LUGAR DONDE SE LEE DEL SOCKET
            while (running && !socket.isClosed()) {
                try {
                    if (adapter.hayDatosDisponibles()) {
                        String mensajeJson = leerMensajeSeguro();
                        
                        if (mensajeJson == null) {
                            System.out.println("Mensaje nulo recibido, ignorando");
                            continue;
                        }
                        
                        System.out.println("Mensaje recibido del cliente " + clientId + ":");
                        System.out.println("   " + mensajeJson.substring(0, Math.min(100, mensajeJson.length())));

                        // Determina tipo de mensaje
                        String requestType = extraerValor(mensajeJson, "request_type");
                        
                        if ("GAME_INPUT".equals(requestType) && enPartida) {
                            // Input de juego - agregar a cola para procesamiento
                            inputQueue.offer(mensajeJson);
                        } else {
                            // Mensaje de control - procesar inmediatamente
                            procesarMensajeCliente(mensajeJson);
                        }
                    }
                    
                    Thread.sleep(10); // Pequena pausa
                    
                } catch (Exception e) {
                    System.out.println("Error en bucle principal cliente " + clientId + ": " + e.getMessage());
                    if (e.getMessage() != null && e.getMessage().contains("Socket closed")) {
                        break;
                    }
                }
            }

        } catch (Exception e) {
            System.out.println("Error con cliente " + clientId + ": " + e.getMessage());
            e.printStackTrace();
        } finally {
            cleanup();
        }
    }

    /**
     * Metodo seguro para leer mensajes del socket CON VALIDACION
     */
    private String leerMensajeSeguro() {
        synchronized (socketLock) {
            try {
                String mensaje = adapter.receiveString();
                
                // Validacion basica
                if (mensaje == null || mensaje.isEmpty()) {
                    return null;
                }
                
                // Verifica que sea JSON valido (comienza con '{' y termina con '}')
                String trimmed = mensaje.trim();
                if (!trimmed.startsWith("{") || !trimmed.endsWith("}")) {
                    System.out.println("Mensaje no parece JSON valido: " + trimmed.substring(0, Math.min(50, trimmed.length())));
                    return null;
                }
                
                return mensaje;
                
            } catch (IOException e) {
                System.out.println("Error leyendo mensaje: " + e.getMessage());
                running = false;
                return null;
            }
        }
    }

    private void cleanup() {
        running = false;
        
        if (enPartida && gameId != null) {
            salirDePartida();
        }
        
        try {
            if (adapter != null) adapter.close();
            if (socket != null && !socket.isClosed()) socket.close();
            server.removeClient(this);
            System.out.println("Cliente " + clientId + " desconectado");
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    // ==================== PROCESAR MENSAJES ====================

    private void procesarMensajeCliente(String mensajeJson) {
        try {
            String clientType = extraerValor(mensajeJson, "client_type");
            String requestType = extraerValor(mensajeJson, "request_type");
            String gameId = extraerValor(mensajeJson, "game_id");

            System.out.println("Procesando: client_type=" + clientType + 
                                ", request_type=" + requestType + 
                                ", game_id=" + gameId);

            if ("PLAYER".equals(clientType)) {
                procesarMensajeJugador(requestType, gameId, mensajeJson);
            } else if ("SPECTATOR".equals(clientType)) {
                procesarMensajeEspectador(requestType, gameId, mensajeJson);
            } else {
                enviarError("CLIENT_TYPE_INVALID", "Tipo de cliente no valido: " + clientType);
            }
        } catch (Exception e) {
            System.out.println("Error procesando mensaje: " + e.getMessage());
            e.printStackTrace();
            enviarError("PROCESSING_ERROR", "Error procesando mensaje: " + e.getMessage());
        }
    }

    private void procesarMensajeJugador(String requestType, String gameId, String mensajeJson) {
        System.out.println("Procesando mensaje jugador: " + requestType + " para partida: " + gameId);
        
        switch (requestType) {
            case "CREATE_GAME":
                crearPartidaJugador(gameId);
                break;
            case "JOIN_GAME":
                unirJugadorAPartida(gameId);
                break;
            case "START_GAME":
                System.out.println("START_GAME recibido, iniciando partida...");
                iniciarPartida(gameId);
                break;
            case "LEAVE_GAME":
                salirDePartida();
                break;
            case "GAME_INPUT":
                // NO procesar aqui - se procesa en el bucle de juego
                System.out.println("GAME_INPUT recibido fuera del bucle de juego - ignorando");
                break;
            default:
                enviarError("REQUEST_INVALID", "Tipo de request no valido: " + requestType);
        }
    }

    private void procesarMensajeEspectador(String requestType, String gameId, String mensajeJson) {
        switch (requestType) {
            case "LIST_GAMES":
                enviarListaPartidas();
                break;
            case "JOIN_GAME":
                unirEspectadorAPartida(gameId);
                break;
            case "LEAVE_GAME":
                salirDePartida();
                break;
            default:
                enviarError("REQUEST_INVALID", "Tipo de request no valido: " + requestType);
        }
    }

    // ==================== CREAR/UNIR PARTIDAS ====================

    private void crearPartidaJugador(String gameIdSolicitado) {
        try {
            System.out.println("SOLICITUD CREAR PARTIDA: " + gameIdSolicitado);
            
            String gameIdFinal = server.crearPartida(this);
            
            System.out.println("PARTIDA ASIGNADA: " + gameIdFinal);
            this.gameId = gameIdFinal;
            this.enPartida = true;
            this.esEspectador = false;
            
            Thread.sleep(100);
            
            SocketServidor.Partida partida = server.obtenerPartida(gameIdFinal);
            
            if (partida == null) {
                System.out.println("ERROR: Partida no encontrada despues de crearla!");
                Thread.sleep(200);
                partida = server.obtenerPartida(gameIdFinal);
                
                if (partida == null) {
                    System.out.println("ERROR CRITICO: Partida sigue sin existir despues de 2 intentos");
                    enviarError("GAME_CREATION_FAILED", "Error interno del servidor");
                    return;
                }
            }
            
            System.out.println("PARTIDA CONFIRMADA EN SERVIDOR");
            
            String respuesta = String.format(
                "{\"response_type\":\"GAME_CREATED\",\"game_id\":\"%s\",\"timestamp\":%d,\"status\":\"SUCCESS\",\"message\":\"Partida creada exitosamente\"}",
                gameIdFinal, System.currentTimeMillis());
                
            enviarMensajeSeguro(respuesta);
            System.out.println("Confirmacion GAME_CREATED enviada al cliente");
            
        } catch (Exception e) {
            System.out.println("Error creando partida: " + e.getMessage());
            e.printStackTrace();
            enviarError("GAME_CREATION_FAILED", "Error: " + e.getMessage());
        }
    }

    private void unirJugadorAPartida(String gameId) {
        try {
            if (server.unirJugadorAPartida(gameId, this)) {
                this.gameId = gameId;
                this.enPartida = true;
                this.esEspectador = false;
                
                String respuesta = String.format(
                    "{\"response_type\":\"GAME_JOINED\",\"game_id\":\"%s\",\"timestamp\":%d,\"status\":\"SUCCESS\",\"message\":\"Unido a partida exitosamente\"}",
                    gameId, System.currentTimeMillis());
                    
                enviarMensajeSeguro(respuesta);
                System.out.println("Jugador " + clientId + " unido a partida: " + gameId);
            } else {
                enviarError("GAME_JOIN_FAILED", "No se pudo unir a la partida: " + gameId);
            }
        } catch (Exception e) {
            System.out.println("Error uniendo jugador a partida: " + e.getMessage());
            enviarError("GAME_JOIN_FAILED", "Error uniéndose a partida");
        }
    }

    private void iniciarPartida(String gameId) {
        try {
            System.out.println("SOLICITUD INICIAR PARTIDA: " + gameId);
            
            SocketServidor.Partida partida = server.obtenerPartida(gameId);
            
            if (partida == null) {
                System.out.println("PARTIDA NO ENCONTRADA: " + gameId);
                enviarError("GAME_START_FAILED", "Partida no encontrada: " + gameId);
                return;
            }
            
            if (partida.jugador != this) {
                System.out.println("JUGADOR NO COINCIDE");
                enviarError("GAME_START_FAILED", "No eres el jugador de esta partida");
                return;
            }
            
            partida.activa = true;
            
            String respuesta = String.format(
                "{\"response_type\":\"GAME_STARTED\",\"game_id\":\"%s\",\"timestamp\":%d,\"status\":\"SUCCESS\"}",
                gameId, System.currentTimeMillis());
            enviarMensajeSeguro(respuesta);
            
            System.out.println("Partida INICIADA: " + gameId + " para jugador " + clientId);
            
            // Iniciar bucle de juego en hilo separado
            Thread gameThread = new Thread(this::bucleJuegoJugador);
            gameThread.setDaemon(true);
            gameThread.setName("GameLoop-" + clientId);
            gameThread.start();
            
            System.out.println("Bucle de juego iniciado en hilo separado");
            
        } catch (Exception e) {
            System.out.println("Error iniciando partida: " + e.getMessage());
            e.printStackTrace();
            enviarError("GAME_START_FAILED", "Error iniciando partida: " + e.getMessage());
        }
    }

    // ==================== BUCLE DE JUEGO ====================

    private void bucleJuegoJugador() {
        try {
            System.out.println("INICIANDO BUCLE DE JUEGO para jugador " + clientId);
            SocketServidor.Partida partida = server.obtenerPartida(gameId);
            
            if (partida == null) {
                System.out.println("Partida no encontrada: " + gameId);
                return;
            }
            
            long ultimoEnvioLocal = System.currentTimeMillis();
            final long intervaloEnvioMsLocal = 50;
            int frameCount = 0;
            
            while (running && enPartida && !socket.isClosed() && partida.activa) {
                try {
                    long ahora = System.currentTimeMillis();
                    frameCount++;
                    
                    // 1) Procesar inputs de la cola (NO leer del socket)
                    String jsonInput = inputQueue.poll(); // No bloqueante
                    if (jsonInput != null) {
                        procesarInput(jsonInput);
                    }
                    
                    // 2) Actualizar logica del juego
                    partida.logica.update(currentInput);
                    
                    // 3) Enviar estado periodicamente
                    if (ahora - ultimoEnvioLocal >= intervaloEnvioMsLocal) {
                        String jsonEstado = generarEstadoJuego(partida.logica);
                        enviarMensajeSeguro(jsonEstado);
                        ultimoEnvioLocal = ahora;
                        
                        if (frameCount % 20 == 0) {
                            System.out.println("Estado enviado (frame " + frameCount + ")");
                        }
                    }
                    
                    Thread.sleep(10);
                } catch (Exception e) {
                    System.out.println("Error en bucle de juego: " + e.getMessage());
                    break;
                }
            }
            
            System.out.println("Bucle de juego terminado");
        } catch (Exception e) {
            System.out.println("Error en bucle de juego: " + e.getMessage());
            e.printStackTrace();
        }
    }

    private void bucleJuegoEspectador() {
        try {
            System.out.println("Iniciando bucle de espectador " + clientId + " en partida: " + gameId);
            SocketServidor.Partida partida = server.obtenerPartida(gameId);
            
            if (partida == null) {
                System.out.println("Partida no encontrada para espectador: " + gameId);
                return;
            }
            
            long ultimoEnvioLocal = System.currentTimeMillis();
            final long intervaloEnvioMsLocal = 100;
            
            while (running && enPartida && !socket.isClosed() && partida.activa) {
                try {
                    long ahora = System.currentTimeMillis();
                    
                    if (ahora - ultimoEnvioLocal >= intervaloEnvioMsLocal) {
                        String jsonEstado = generarEstadoJuego(partida.logica);
                        enviarMensajeSeguro(jsonEstado);
                        ultimoEnvioLocal = ahora;
                    }
                    
                    Thread.sleep(50);
                } catch (Exception e) {
                    System.out.println("Error en bucle espectador: " + e.getMessage());
                    break;
                }
            }
        } catch (Exception e) {
            System.out.println("Error en bucle espectador: " + e.getMessage());
            e.printStackTrace();
        }
    }

    /**
     * Metodo sincronizado para enviar mensajes
     */
    private void enviarMensajeSeguro(String mensaje) {
        synchronized (socketLock) {
            try {
                if (adapter != null && !socket.isClosed()) {
                    adapter.sendString(mensaje);
                }
            } catch (IOException e) {
                System.out.println("Error enviando mensaje: " + e.getMessage());
                running = false;
            }
        }
    }

    // ==================== ESPECTADOR ====================

    private void unirEspectadorAPartida(String gameId) {
        try {
            if (server.unirEspectadorAPartida(gameId, this)) {
                this.gameId = gameId;
                this.enPartida = true;
                this.esEspectador = true;
                
                String respuesta = String.format(
                    "{\"response_type\":\"SPECTATOR_JOINED\",\"game_id\":\"%s\",\"timestamp\":%d,\"status\":\"SUCCESS\"}",
                    gameId, System.currentTimeMillis());
                    
                enviarMensajeSeguro(respuesta);
                System.out.println("Espectador " + clientId + " unido a partida: " + gameId);
                
                new Thread(this::bucleJuegoEspectador).start();
            } else {
                enviarError("SPECTATOR_JOIN_FAILED", "No se pudo unir como espectador: " + gameId);
            }
        } catch (Exception e) {
            System.out.println("Error uniendo espectador: " + e.getMessage());
            enviarError("SPECTATOR_JOIN_FAILED", "Error uniéndose como espectador");
        }
    }

    private void enviarListaPartidas() {
        try {
            List<SocketServidor.InfoPartida> partidas = server.obtenerListaPartidas();
            StringBuilder json = new StringBuilder();
            
            json.append("{\n");
            json.append("  \"response_type\": \"GAME_LIST\",\n");
            json.append("  \"timestamp\": ").append(System.currentTimeMillis()).append(",\n");
            json.append("  \"games\": [\n");
            
            for (int i = 0; i < partidas.size(); i++) {
                SocketServidor.InfoPartida partida = partidas.get(i);
                json.append("    {\n");
                json.append("      \"game_id\": \"").append(partida.gameId).append("\",\n");
                json.append("      \"player_count\": ").append(partida.playerCount).append(",\n");
                json.append("      \"spectators\": ").append(partida.spectators).append(",\n");
                json.append("      \"active\": ").append(partida.active).append("\n");
                json.append("    }");
                if (i < partidas.size() - 1) json.append(",");
                json.append("\n");
            }
            
            json.append("  ]\n");
            json.append("}");
            
            enviarMensajeSeguro(json.toString());
            System.out.println("Lista de partidas enviada a espectador " + clientId);
            
        } catch (Exception e) {
            System.out.println("Error enviando lista de partidas: " + e.getMessage());
            enviarError("LIST_GAMES_FAILED", "Error obteniendo lista de partidas");
        }
    }

    // ==================== SALIR ====================

    private void salirDePartida() {
        if (gameId != null && enPartida) {
            System.out.println("Cliente " + clientId + " saliendo de partida: " + gameId);
            
            String currentGameId = gameId;
            
            if (esEspectador) {
                server.removerEspectadorDePartida(currentGameId, this);
            } else {
                server.removerJugadorDePartida(currentGameId, this);
            }
            
            enPartida = false;
            esEspectador = false;
            
            try {
                String respuesta = String.format(
                    "{\"response_type\":\"%s\",\"game_id\":\"%s\",\"timestamp\":%d,\"status\":\"SUCCESS\"}",
                    esEspectador ? "SPECTATOR_LEFT" : "GAME_LEFT", 
                    currentGameId, System.currentTimeMillis());
                    
                enviarMensajeSeguro(respuesta);
                System.out.println("Confirmacion de salida enviada para partida: " + currentGameId);
            } catch (Exception e) {
                System.out.println("Error enviando confirmacion de salida: " + e.getMessage());
            }
            
            gameId = null;
        }
    }

    private void enviarError(String errorCode, String mensaje) {
        try {
            String errorJson = String.format(
                "{\"response_type\":\"ERROR\",\"timestamp\":%d,\"error_code\":\"%s\",\"message\":\"%s\"}",
                System.currentTimeMillis(), errorCode, mensaje);
                
            enviarMensajeSeguro(errorJson);
            System.out.println("Error enviado a cliente " + clientId + ": " + errorCode + " - " + mensaje);
        } catch (Exception e) {
            System.out.println("Error enviando mensaje de error: " + e.getMessage());
        }
    }

    // ==================== PROCESAMIENTO DE INPUT ====================

    private void procesarInput(String jsonInput) {
        try {
            String inputType = extraerValor(jsonInput, "input_type");
            String key = extraerValor(jsonInput, "key");

            System.out.println("Procesando input: " + inputType + " - " + key);

            boolean pressed = "KEY_PRESSED".equals(inputType);

            boolean left = currentInput.isLeft();
            boolean right = currentInput.isRight();
            boolean up = currentInput.isUp();
            boolean down = currentInput.isDown();
            boolean jump = currentInput.isJump();

            switch (key) {
                case "LEFT":
                    left = pressed;
                    if (pressed) playerState = "MOVING_LEFT";
                    else if (!right) playerState = "STANDING";
                    break;
                case "RIGHT":
                    right = pressed;
                    if (pressed) playerState = "MOVING_RIGHT";
                    else if (!left) playerState = "STANDING";
                    break;
                case "UP":
                    up = pressed;
                    if (pressed) playerState = "CLIMBING";
                    break;
                case "DOWN":
                    down = pressed;
                    if (pressed) playerState = "FALLING";
                    break;
                case "JUMP":
                    jump = pressed;
                    if (pressed) playerState = "JUMPING";
                    break;
                default:
                    System.out.println("Tecla desconocida: " + key);
                    break;
            }

            if (!left && !right && !"CLIMBING".equals(playerState) && 
                !"JUMPING".equals(playerState) && !"FALLING".equals(playerState)) {
                playerState = "STANDING";
            }

            currentInput = new PlayerInput(left, right, up, down, jump);

            System.out.println("Input actualizado - L:" + left + " R:" + right + 
                        " U:" + up + " D:" + down + " J:" + jump + 
                        " State:" + playerState);

        } catch (Exception e) {
            System.out.println("Error procesando input JSON: " + e.getMessage());
            e.printStackTrace();
        }
    }

    // ==================== GENERACION DE ESTADO ====================

    private String generarEstadoJuego(GameLogic gameLogic) {
        StringBuilder json = new StringBuilder();
        boolean gameActive = true;
        Player player = gameLogic.getPlayer();

        json.append("{\n");
        json.append("  \"response_type\": \"GAME_STATE\",\n");
        json.append("  \"game_id\":\"").append(gameId).append("\",\n");
        json.append("  \"timestamp\":").append(System.currentTimeMillis()).append(",\n");
        json.append("  \"game_active\":").append(gameActive).append(",\n");
        json.append("  \"changes\":{\n");
        json.append("    \"player\":{\n");
        json.append("      \"x\": ").append(player.getX()).append(",\n");
        json.append("      \"y\": ").append(player.getY()).append(",\n");
        json.append("      \"state\":\"").append(playerState).append("\",\n");
        json.append("      \"lives\":").append(player.getLives()).append(",\n");
        json.append("      \"score\":").append(player.getScore()).append(",\n");
        json.append("      \"active\":").append(gameActive).append("\n");
        json.append("    },\n");

        json.append("    \"enemies\":[\n");
        int enemyIndex = 0;
        for (Croc c : gameLogic.getCrocs()) {
            if (c != null && c.isAlive()) {
                if (enemyIndex > 0) json.append(",\n");
                String type = (c instanceof RedCroc) ? "RED_CROCODILE" : "BLUE_CROCODILE";
                json.append("      {\n");
                json.append("        \"id\":\"croc_").append(enemyIndex).append("\",\n");
                json.append("        \"type\":\"").append(type).append("\",\n");
                json.append("        \"x\":").append(c.getX()).append(",\n");
                json.append("        \"y\":").append(c.getY()).append(",\n");
                json.append("        \"active\":true\n");
                json.append("      }");
                enemyIndex++;
            }
        }
        json.append("\n    ],\n");

        json.append("    \"fruits\":[\n");
        int fruitIndex = 0;
        for (Fruit f : gameLogic.getFruits()) {
            if (!f.isActive()) continue;
            if (fruitIndex > 0) json.append(",\n");
            json.append("      {\n");
            json.append("        \"id\":\"fruit_").append(fruitIndex).append("\",\n");
            json.append("        \"type\":\"BANANA\",\n");
            json.append("        \"points\":").append(f.getPoints()).append(",\n");
            json.append("        \"x\":").append(f.getX()).append(",\n");
            json.append("        \"y\":").append(f.getY()).append(",\n");
            json.append("        \"active\":true\n");
            json.append("      }");
            fruitIndex++;
        }
        json.append("\n    ]\n");
        json.append("  }\n");
        json.append("}");

        return json.toString();
    }

    // ==================== UTILIDADES ====================

    private String extraerValor(String json, String clave) {
        try {
            String patron = "\"" + clave + "\":";
            int inicio = json.indexOf(patron);
            if (inicio == -1) return "";
            
            inicio += patron.length();
            while (inicio < json.length() && Character.isWhitespace(json.charAt(inicio))) {
                inicio++;
            }
            
            if (inicio >= json.length()) return "";
            
            char primerChar = json.charAt(inicio);
            
            if (primerChar == '"') {
                inicio++;
                int fin = json.indexOf("\"", inicio);
                if (fin == -1) return "";
                return json.substring(inicio, fin);
            } else {
                int fin = json.indexOf(",", inicio);
                if (fin == -1) fin = json.indexOf("}", inicio);
                if (fin == -1) fin = json.indexOf("\n", inicio);
                if (fin == -1) return json.substring(inicio).trim();
                return json.substring(inicio, fin).trim();
            }
        } catch (Exception e) {
            System.out.println("Error extrayendo valor '" + clave + "'");
            return "";
        }
    }

    public int getClientId() {
        return clientId;
    }
}