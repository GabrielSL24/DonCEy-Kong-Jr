import java.net.Socket;
import java.util.List;
import java.io.IOException;

/**
 * Maneja la comunicación con UN cliente.
 * 
 * En el caso de JUGADOR:
 * - Recibe JSON de input con teclas.
 * - Actualiza PlayerInput.
 * - Llama a GameLogic.update(input) para avanzar la lógica.
 * - Construye un JSON de estado leyendo DIRECTO desde GameLogic
 *   (jugador, cocodrilos, frutas) y lo envía al cliente C.
 *
 * NO se modifica la lógica interna de GameLogic:
 * - No se implementa aún ganar/perder.
 * - Solo se usa como fuente de verdad para el estado.
 */
public class ClientHandler implements Runnable {

    private final Socket socket;
    private final SocketServidor server;
    private AdapterJ adapter;
    private String clientType;
    private final int clientId;
    private static int nextId = 1;

    // --- Lógica de juego (carpeta Logica) ---
    private final GameLogic gameLogic;

    // Estado actual de las teclas del jugador
    private PlayerInput currentInput;

    // Estado visual de animación (para el campo "state" del JSON)
    private String playerState = "STANDING";

    // Estado de la conexion
    private String gameId = null;
    private boolean enPartida = false;
    private boolean esEspectador = false;

    private long ultimoEnvio = System.currentTimeMillis();
    private final long intervaloEnvioMs = 100; // ~10 FPS

    public ClientHandler(Socket socket, SocketServidor server) {
        this.socket = socket;
        this.server = server;
        this.clientId = nextId++;

        // Instanciamos la lógica tal como está en Logica/
        this.gameLogic = new GameLogic();

        // Por defecto, ninguna tecla presionada
        this.currentInput = new PlayerInput(false, false, false, false, false);
    }

    @Override
    public void run() {
        try {
            adapter = new AdapterJ(socket);
            socket.setSoLinger(true, 10);

            System.out.println("Iniciando comunicacion con cliente " + clientId);

            // 1. Enviar identificacion del servidor
            adapter.sendIdentification("SERVIDOR");

            // 2. Recibir identificación del cliente
            clientType = adapter.receiveIdentification();
            System.out.println("Cliente " + clientId + " es " + clientType +
                    " desde " + socket.getInetAddress().getHostAddress());

            
            // 3. Bucle principal de recepcion de mensajes
            while (!socket.isClosed()) {
                try {
                    if (adapter.hayDatosDisponibles()) {
                        String mensajeJson = adapter.receiveString();
                        System.out.println("Mensaje recibido del cliente " + clientId + ":");
                        System.out.println("   " + mensajeJson);

                        procesarMensajeCliente(mensajeJson);
                    }
                    Thread.sleep(10);

                } catch (Exception e) {
                    System.out.println("Error en bucle principal cliente " + clientId + ": " + e.getMessage());
                    break;
                }
            }

        } catch (Exception e) {
            System.out.println("Error con cliente " + clientId + ": " + e.getMessage());
            e.printStackTrace();

        } finally {
            // Limpieza al desconectar
            if (enPartida && gameId != null) {
                salirDePartida();
            }

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

    //Procesa cualquier mensaje JSON
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
            enviarError("CLIENT_TYPE_INVALID", "Tipo de cliente no válido: " + clientType);
            }
        } catch (Exception e) {
            System.out.println("Error procesando mensaje: " + e.getMessage());
            e.printStackTrace();
            enviarError("PROCESSING_ERROR", "Error procesando mensaje: " + e.getMessage());
        }
    }

    // Procesa mensajes de JUGADOR
    private void procesarMensajeJugador(String requestType, String gameId, String mensajeJson) {
        switch (requestType) {
            case "CREATE_GAME":
                crearPartidaJugador(gameId);
                break;
            case "JOIN_GAME":
                unirJugadorAPartida(gameId);
                break;
            case "START_GAME":
                iniciarPartida(gameId);
                break;
            case "LEAVE_GAME":
                salirDePartida();
                break;
            case "GAME_INPUT":
                procesarInput(mensajeJson);
                break;
            default:
                enviarError("REQUEST_INVALID", "Tipo de request no válido: " + requestType);
        }
    }

    // Procesa mensajes de ESPECTADOR
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
                enviarError("REQUEST_INVALID", "Tipo de request no válido: " + requestType);
        }
    }

    // Crear una nueva partida para el jugador
    private void crearPartidaJugador(String gameIdSolicitado) {
        try {
            String gameIdFinal = (gameIdSolicitado != null && !gameIdSolicitado.isEmpty()) ? 
                                gameIdSolicitado : server.crearPartida(this);
            
            this.gameId = gameIdFinal;
            this.enPartida = true;
            this.esEspectador = false;
            
            SocketServidor.Partida partida = server.obtenerPartida(gameIdFinal);
            
            // Enviar confirmación
            String respuesta = String.format(
                "{\"response_type\":\"GAME_CREATED\",\"game_id\":\"%s\",\"timestamp\":%d,\"status\":\"SUCCESS\",\"message\":\"Partida creada exitosamente\"}",
                gameIdFinal, System.currentTimeMillis());
                
            adapter.sendString(respuesta);
            System.out.println("✅ Partida creada: " + gameIdFinal + " para jugador " + clientId);
            
            // ENVIAR ESTADO INICIAL INMEDIATAMENTE
            if (partida != null) {
                String estadoInicial = generarEstadoJuego(partida.logica);
                adapter.sendString(estadoInicial);
                System.out.println("📤 Estado inicial enviado al jugador");
            }
            
        } catch (Exception e) {
            System.out.println("Error creando partida: " + e.getMessage());
            enviarError("GAME_CREATION_FAILED", "No se pudo crear la partida");
        }
    }

    // Unir jugador a una partida existente
     private void unirJugadorAPartida(String gameId) {
        try {
            if (server.unirJugadorAPartida(gameId, this)) {
                this.gameId = gameId;
                this.enPartida = true;
                this.esEspectador = false;
                
                String respuesta = String.format(
                    "{\n" +
                    "  \"response_type\": \"GAME_JOINED\",\n" +
                    "  \"game_id\": \"%s\",\n" +
                    "  \"timestamp\": %d,\n" +
                    "  \"status\": \"SUCCESS\",\n" +
                    "  \"message\": \"Unido a partida exitosamente\"\n" +
                    "}", gameId, System.currentTimeMillis());
                    
                adapter.sendString(respuesta);
                System.out.println("Jugador " + clientId + " unido a partida: " + gameId);
            } else {
                enviarError("GAME_JOIN_FAILED", "No se pudo unir a la partida: " + gameId);
            }
        } catch (Exception e) {
            System.out.println("Error uniendo jugador a partida: " + e.getMessage());
            enviarError("GAME_JOIN_FAILED", "Error uniéndose a partida");
        }
    }

    // Iniciar la partida
    private void iniciarPartida(String gameId) {
    try {
        SocketServidor.Partida partida = server.obtenerPartida(gameId);
        if (partida != null && partida.jugador == this) {
            partida.activa = true;
            
            // 1. Enviar confirmación de inicio
            String respuesta = String.format(
                "{\"response_type\":\"GAME_STARTED\",\"game_id\":\"%s\",\"timestamp\":%d,\"status\":\"SUCCESS\"}",
                gameId, System.currentTimeMillis());
            adapter.sendString(respuesta);
            
            // 2. Enviar estado inicial del juego inmediatamente
            String estadoInicial = generarEstadoJuego(partida.logica);
            adapter.sendString(estadoInicial);
            
            System.out.println("🎮 Partida iniciada: " + gameId);
            
            // 3. Iniciar el bucle de juego
            new Thread(this::bucleJuegoJugador).start();
        } else {
            enviarError("GAME_START_FAILED", "No se puede iniciar la partida");
        }
    } catch (Exception e) {
        System.out.println("Error iniciando partida: " + e.getMessage());
        enviarError("GAME_START_FAILED", "Error iniciando partida");
    }
}

    // Une espectador a una partida existente
     private void unirEspectadorAPartida(String gameId) {
        try {
            if (server.unirEspectadorAPartida(gameId, this)) {
                this.gameId = gameId;
                this.enPartida = true;
                this.esEspectador = true;
                
                String respuesta = String.format(
                    "{\n" +
                    "  \"response_type\": \"SPECTATOR_JOINED\",\n" +
                    "  \"game_id\": \"%s\",\n" +
                    "  \"timestamp\": %d,\n" +
                    "  \"status\": \"SUCCESS\"\n" +
                    "}", gameId, System.currentTimeMillis());
                    
                adapter.sendString(respuesta);
                System.out.println("Espectador " + clientId + " unido a partida: " + gameId);
                
                // Iniciar el bucle de espectador
                new Thread(this::bucleJuegoEspectador).start();
            } else {
                enviarError("SPECTATOR_JOIN_FAILED", "No se pudo unir como espectador: " + gameId);
            }
        } catch (Exception e) {
            System.out.println("Error uniendo espectador: " + e.getMessage());
            enviarError("SPECTATOR_JOIN_FAILED", "Error uniéndose como espectador");
        }
    }

    // Envia lista de partidas disponibles
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
            
            adapter.sendString(json.toString());
            System.out.println("Lista de partidas enviada a espectador " + clientId);
            
        } catch (Exception e) {
            System.out.println("Error enviando lista de partidas: " + e.getMessage());
            enviarError("LIST_GAMES_FAILED", "Error obteniendo lista de partidas");
        }
    }

    // Bucle principal de juego para JUGADOR
    private void bucleJuegoJugador() {
        try {
            System.out.println("🎮 Iniciando bucle de juego para jugador " + clientId + " en partida: " + gameId);
            SocketServidor.Partida partida = server.obtenerPartida(gameId);
            
            if (partida == null) {
                System.out.println("❌ Partida no encontrada: " + gameId);
                return;
            }
            
            long ultimoEnvioLocal = System.currentTimeMillis();
            final long intervaloEnvioMsLocal = 100; // ~10 FPS
            
            // IMPORTANTE: Configurar timeout para no bloquear
            socket.setSoTimeout(50);
            
            while (enPartida && !socket.isClosed() && partida.activa) {
                try {
                    long ahora = System.currentTimeMillis();
                    
                    // 1) Leer input del cliente si hay datos
                    if (adapter.hayDatosDisponibles()) {
                        String jsonInput = adapter.receiveString();
                        System.out.println("🎮 Input recibido del jugador " + clientId + ": " + jsonInput);
                        procesarInput(jsonInput);
                    }
                    
                    // 2) Actualizar lógica y enviar estado cada intervalo
                    if (ahora - ultimoEnvioLocal >= intervaloEnvioMsLocal) {
                        // Avanzar la lógica con el input actual
                        partida.logica.update(currentInput);
                        
                        // Generar y enviar estado
                        String jsonEstado = generarEstadoJuego(partida.logica);
                        adapter.sendString(jsonEstado);
                        
                        ultimoEnvioLocal = ahora;
                    }
                    
                    Thread.sleep(5);
                } catch (java.net.SocketTimeoutException e) {
                    // Timeout normal, continuar
                } catch (Exception e) {
                    System.out.println("Error en bucle de juego: " + e.getMessage());
                    break;
                }
            }
        } catch (Exception e) {
            System.out.println("Error en bucle de juego jugador: " + e.getMessage());
            e.printStackTrace();
        }
    }

    // Bucle para ESPECTADOR
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
            
            while (enPartida && !socket.isClosed() && partida.activa) {
                try {
                    long ahora = System.currentTimeMillis();
                    
                    // Enviar estado al espectador periódicamente
                    if (ahora - ultimoEnvioLocal >= intervaloEnvioMsLocal) {
                        String jsonEstado = generarEstadoJuego(partida.logica);
                        adapter.sendString(jsonEstado);
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

    // Enviar el estado de la partida a todos los clientes
     private void enviarEstadoPartida(SocketServidor.Partida partida) {
        try {
            String jsonEstado = generarEstadoJuego(partida.logica);
            
            // Enviar al jugador
            if (partida.jugador != null && partida.jugador.adapter != null) {
                partida.jugador.adapter.sendString(jsonEstado);
            }
            
            // Enviar a todos los espectadores
            for (ClientHandler espectador : partida.espectadores) {
                if (espectador.adapter != null) {
                    try {
                        espectador.adapter.sendString(jsonEstado);
                    } catch (IOException e) {
                        System.out.println("Error enviando a espectador " + espectador.clientId + ": " + e.getMessage());
                        // Remover espectador si hay error
                        partida.espectadores.remove(espectador);
                    }
                }
            }
        } catch (Exception e) {
            System.out.println("Error enviando estado de partida: " + e.getMessage());
        }
    }

    // Salir de la partida actual
    private void salirDePartida() {
        if (gameId != null && enPartida) {
            System.out.println("Cliente " + clientId + " saliendo de partida: " + gameId);
            
            // Remover de la partida según el tipo
            if (esEspectador) {
                server.removerEspectadorDePartida(gameId, this);
            } else {
                server.removerJugadorDePartida(gameId, this);
            }
            
            enPartida = false;
            gameId = null;
            esEspectador = false;
            
            // Enviar confirmación
            try {
                String respuesta = String.format(
                    "{\n" +
                    "  \"response_type\": \"%s\",\n" +
                    "  \"game_id\": \"%s\",\n" +
                    "  \"timestamp\": %d,\n" +
                    "  \"status\": \"SUCCESS\"\n" +
                    "}", esEspectador ? "SPECTATOR_LEFT" : "GAME_LEFT", 
                    gameId, System.currentTimeMillis());
                    
                adapter.sendString(respuesta);
            } catch (Exception e) {
                System.out.println("Error enviando confirmación de salida: " + e.getMessage());
            }
        }
    }

    // Envia mensaje de error al cliente
    private void enviarError(String errorCode, String mensaje) {
        try {
            String errorJson = String.format(
                "{\n" +
                "  \"response_type\": \"ERROR\",\n" +
                "  \"timestamp\": %d,\n" +
                "  \"error_code\": \"%s\",\n" +
                "  \"message\": \"%s\"\n" +
                "}", System.currentTimeMillis(), errorCode, mensaje);
                
            adapter.sendString(errorJson);
            System.out.println("Error enviado a cliente " + clientId + ": " + errorCode + " - " + mensaje);
        } catch (Exception e) {
            System.out.println("Error enviando mensaje de error: " + e.getMessage());
        }
    }

    /**
     * Procesa el JSON de input del cliente y actualiza:
     * - currentInput (estado de teclas)
     * - playerState (string para animación)
     *
     * JSON esperado:
     * {
     *   "input_type": "KEY_PRESSED" | "KEY_RELEASED",
     *   "key": "LEFT" | "RIGHT" | "UP" | "DOWN" | "JUMP"
     * }
     */
    private void procesarInput(String jsonInput) {
        try {
            String inputType = extraerValor(jsonInput, "input_type");
            String key = extraerValor(jsonInput, "key");

            System.out.println("Procesando input: " + inputType + " - " + key);

            if (key.endsWith("_RELEASED")) {
                String realKey = key.substring(0, key.length() - 9); // quitar "_RELEASED"
                System.out.println("CORRECCIÓN: Convirtiendo '" + key + "' a KEY_RELEASED con key '" + realKey + "'");
                inputType = "KEY_RELEASED";
                key = realKey;
            }
            
            if (key.endsWith("_PRESSED")) {
                String realKey = key.substring(0, key.length() - 8); // quitar "_PRESSED"  
                System.out.println("CORRECCIÓN: Convirtiendo '" + key + "' a KEY_PRESSED con key '" + realKey + "'");
                inputType = "KEY_PRESSED";
                key = realKey;
            }
            
            // Validar que tenemos valores correctos
            if (inputType.isEmpty() || key.isEmpty()) {
                System.out.println("ERROR: InputType o Key están vacíos. JSON: " + jsonInput);
                return;
            }
            
            boolean pressed = "KEY_PRESSED".equals(inputType);

            boolean left = currentInput.isLeft();
            boolean right = currentInput.isRight();
            boolean up = currentInput.isUp();
            boolean down = currentInput.isDown();
            boolean jump = currentInput.isJump();

            switch (key) {
                case "LEFT":
                    left = pressed;
                    if (pressed) {
                        playerState = "MOVING_LEFT";
                    }
                    break;
                case "RIGHT":
                    right = pressed;
                    if (pressed) {
                        playerState = "MOVING_RIGHT";
                    }
                    break;
                case "UP":
                    up = pressed;
                    if (pressed) {
                        playerState = "CLIMBING";
                    }
                    break;
                case "DOWN":
                    down = pressed;
                    if (pressed) {
                        playerState = "FALLING";
                    }
                    break;
                case "JUMP":
                    jump = pressed;
                    if (pressed) {
                        playerState = "JUMPING";
                    }
                    break;
                default:
                    // Tecla desconocida, la ignoramos
                    break;
            }

            // Actualizamos el objeto PlayerInput que la lógica consumirá
            currentInput = new PlayerInput(left, right, up, down, jump);

            System.out.println("Input actualizado - L:" + left + " R:" + right + 
                        " U:" + up + " D:" + down + " J:" + jump + 
                        " State:" + playerState);


        } catch (Exception e) {
            System.out.println("Error procesando input JSON: " + e.getMessage());
            e.printStackTrace();
        }
    }

    /**
     * Extrae un valor string del JSON en formato "clave":"valor".
     * Parser mínimo para este uso concreto.
     */
    private String extraerValor(String json, String clave) {
        try {
            // Buscar el patrón con la clave
            String patron = "\"" + clave + "\":";
            int inicio = json.indexOf(patron);
            if (inicio == -1) {
                System.out.println("DEBUG: No se encontró la clave '" + clave + "' en JSON: " + json);
                return "";
            }
            
            inicio += patron.length();
            
            // Buscar el inicio del valor (saltar espacios)
            while (inicio < json.length() && Character.isWhitespace(json.charAt(inicio))) {
                inicio++;
            }
            
            if (inicio >= json.length()) {
                return "";
            }
            
            char primerChar = json.charAt(inicio);
            
            // Determinar si el valor es string (entre comillas) o otro tipo
            if (primerChar == '"') {
                // Valor es string entre comillas
                inicio++; // saltar la comilla inicial
                int fin = json.indexOf("\"", inicio);
                if (fin == -1) {
                    return "";
                }
                return json.substring(inicio, fin);
            } else {
                // Valor es número, bool, o cualquier cosa sin comillas
                int fin = json.indexOf(",", inicio);
                if (fin == -1) {
                    fin = json.indexOf("}", inicio);
                }
                if (fin == -1) {
                    fin = json.indexOf("\n", inicio); // también buscar fin de línea
                }
                if (fin == -1) {
                    return json.substring(inicio).trim();
                }
                return json.substring(inicio, fin).trim();
            }
        } catch (Exception e) {
            System.out.println("Error extrayendo valor '" + clave + "' de JSON: " + json);
            e.printStackTrace();
            return "";
        }
    }

    /**
     * Construye el JSON de estado leyendo DIRECTAMENTE desde GameLogic:
     *
     * {
     *   "game_id": "...",
     *   "timestamp": ...,
     *   "game_active": true,
     *   "changes": {
     *     "player": {...},
     *     "enemies": [...],
     *     "fruits": [...]
     *   }
     * }
     *
     * NOTA: game_active se deja en true por ahora.
     */
    private String generarEstadoJuego(GameLogic gameLogic) {
        StringBuilder json = new StringBuilder();

        // Por ahora el juego se considera siempre activo
        boolean gameActive = true;

        // --- Player desde lógica ---
        Player player = gameLogic.getPlayer();
        float playerX = player.getX(); 
        float playerY = player.getY();

        json.append("{\n");
        json.append("  \"response_type\": \"GAME_STATE\",\n"); // ← AGREGAR response_type
        json.append("  \"game_id\":\"").append(gameId).append("\",\n");
        json.append("  \"timestamp\":").append(System.currentTimeMillis()).append(",\n");
        json.append("  \"game_active\":").append(gameActive).append(",\n");
        json.append("  \"changes\":{\n");
        json.append("    \"player\":{\n");
        json.append("      \"x\": ").append(playerX).append(",\n");
        json.append("      \"y\": ").append(playerY).append(",\n");
        json.append("      \"state\":\"").append(playerState).append("\",\n");
        json.append("      \"lives\":").append(player.getLives()).append(",\n");
        json.append("      \"score\":").append(player.getScore()).append(",\n");
        json.append("      \"active\":").append(gameActive).append("\n");
        json.append("    },\n");

        // --- Enemigos (cocodrilos) ---
        json.append("    \"enemies\":[\n");
        int enemyIndex = 0;
        for (Croc c : gameLogic.getCrocs()) {
            if (c != null && c.isAlive()) {
                if (enemyIndex > 0) {
                    json.append(",\n");
                }
                float ex = c.getX();
                float ey = c.getY();
                String type = (c instanceof RedCroc) ? "RED_CROCODILE" : "BLUE_CROCODILE";

                json.append("      {\n");
                json.append("        \"id\":\"croc_").append(enemyIndex).append("\",\n");
                json.append("        \"type\":\"").append(type).append("\",\n");
                json.append("        \"x\":").append(ex).append(",\n");
                json.append("        \"y\":").append(ey).append(",\n");
                json.append("        \"active\":true\n");
                json.append("      }");

                enemyIndex++;
            }
        }
        json.append("\n    ],\n");

        // --- Frutas ---
        json.append("    \"fruits\":[\n");
        int fruitIndex = 0;
        for (Fruit f : gameLogic.getFruits()) {
            if (!f.isActive()) {
                continue;
            }
            if (fruitIndex > 0) {
                json.append(",\n");
            }
            float fx = f.getX();
            float fy = f.getY();

            json.append("      {\n");
            json.append("        \"id\":\"fruit_").append(fruitIndex).append("\",\n");
            json.append("        \"type\":\"BANANA\",\n");
            json.append("        \"points\":").append(f.getPoints()).append(",\n");
            json.append("        \"x\":").append(fx).append(",\n");
            json.append("        \"y\":").append(fy).append(",\n");
            json.append("        \"active\":true\n");
            json.append("      }");

            fruitIndex++;
        }
        json.append("\n    ]\n");

        json.append("  }\n");
        json.append("}");

        String resultado = json.toString();
        System.out.println("📤 JSON generado para partida " + gameId + ":");
        System.out.println(resultado);

        return resultado;
    }

    // =========================================================
    //                   ESPECTADOR
    // =========================================================

    private void handleEspectador() throws IOException {
        System.out.println("Espectador " + clientId + " observando");

        // Ejemplo simple de protocolo con enteros
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

    // =========================================================
    //                       ADMIN
    // =========================================================

    private void handleAdmin() throws IOException {
        System.out.println("Admin " + clientId + " conectado");

        adapter.sendInt(300);

        // Ejemplo simple de eco de comandos
        for (int i = 0; i < 3; i++) {
            int comando = adapter.receiveInt();
            System.out.println("Admin " + clientId + " envió comando: " + comando);
            adapter.sendInt(comando + 1000);
        }
    }

    public int getClientId() {
        return clientId;
    }
}
