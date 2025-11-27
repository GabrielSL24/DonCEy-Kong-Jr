import java.net.Socket;
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

    // Id simple de la partida (ej. para espectadores/admin, a futuro)
    private final String gameId = "partida_" + System.currentTimeMillis();

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

            // 3. Manejar según tipo
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

        } catch (Exception e) {
            System.out.println("Error con cliente " + clientId + ": " + e.getMessage());
            e.printStackTrace();
        } finally {
            try {
                if (adapter != null) {
                    adapter.close();
                }
                if (socket != null) {
                    socket.close();
                }
                server.removeClient(this);
                System.out.println("Cliente " + clientId + " desconectado");
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    // =========================================================
    //                       JUGADOR
    // =========================================================

    private void handleJugador() throws IOException {
        System.out.println("Jugador " + clientId + " listo - INICIANDO PARTIDA: " + gameId);

        // Estado inicial tomando la lógica real
        System.out.println("Enviando estado inicial al jugador " + clientId);
        String jsonEstadoInicial = generarEstadoJuego();
        adapter.sendString(jsonEstadoInicial);
        System.out.println("Estado inicial enviado al jugador " + clientId);

        long ultimoEnvio = System.currentTimeMillis();
        final long intervaloEnvioMs = 100; // ~10 FPS de estado

        socket.setSoTimeout(50); // 50ms timeout

        // Por ahora, no usamos estado global: el juego se considera siempre activo
        while (!socket.isClosed()) {
            try {
                long ahora = System.currentTimeMillis();

                // 1) Leer input del cliente si hay datos
                if (adapter.hayDatosDisponibles()) {
                    String jsonInput = adapter.receiveString();
                    System.out.println("JSON recibido del cliente:");
                    System.out.println("   " + jsonInput);
                    procesarInput(jsonInput);
                }

                // 2) Actualizar lógica y enviar estado cada intervalo
                if (ahora - ultimoEnvio >= intervaloEnvioMs) {
                    // Avanzar la lógica con el input actual
                    gameLogic.update(currentInput);

                    // Generar JSON del estado actual
                    String jsonEstado = generarEstadoJuego();
                    adapter.sendString(jsonEstado);

                    ultimoEnvio = ahora;
                }

                // Pequeño sleep para no saturar CPU
                Thread.sleep(5);
            } catch (Exception e) {
                System.out.println("Error en handleJugador: " + e.getMessage());
                e.printStackTrace();
                break;
            }
        }

        System.out.println("Cliente " + clientId + " finalizado");
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
    private String generarEstadoJuego() {
        StringBuilder json = new StringBuilder();

        // Por ahora el juego se considera siempre activo
        boolean gameActive = true;

        // --- Player desde lógica ---
        Player player = gameLogic.getPlayer();
        float playerX = player.getX(); // en píxeles, según tu lógica
        float playerY = player.getY();

        json.append("{\n");
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
        System.out.println("📤 JSON generado para cliente " + clientId + ":");
        System.out.println(resultado);

        return resultado;
    }

    // =========================================================
    //                   ESPECTADOR / ADMIN
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
}
