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
    
    private DataInputStream input;
    private DataOutputStream output;
    private GameLogic gameLogic;

    public ClientHandler(Socket socket, SocketServidor server) {
        this.socket = socket;
        this.server = server;
        this.gameLogic = new GameLogic();
        this.clientId = nextId++;
    }
    
    @Override
    public void run() {
        try {
            input = new DataInputStream(socket.getInputStream());
            output = new DataOutputStream(socket.getOutputStream());

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
                if (input != null) input.close();
                if (output != null) output.close();
                if (socket != null) socket.close();
                server.removeClient(this);
                System.out.println("Cliente " + clientId + " desconectado");
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private void handleJugador() throws IOException {
        System.out.println("Jugador " + clientId + " listo - INICIANDO");
        
        GameLogic gameLogic = new GameLogic();
        Player player = gameLogic.getPlayer();
        
        // ✅ DEBUG: Verificar estado inicial
        System.out.println("🔍 Estado inicial GameLogic:");
        System.out.println("   - Posición: (" + player.getX() + ", " + player.getY() + ")");
        System.out.println("   - Vidas: " + player.getLives());
        System.out.println("   - Puntos: " + player.getScore());
        
        int vidas = player.getLives();
        int puntos = player.getScore();
        boolean juegoActivo = true;

        while (juegoActivo && !socket.isClosed()) {
            try {
                // VERIFICAR que hay datos suficientes (4 ints = 16 bytes)
                if (input.available() >= 16) {
                    System.out.println("📥 Recibiendo estado del cliente...");
                    
                    // 1. Recibir ESTADO ACTUAL del cliente
                    int matrizX = adapter.receiveInt();
                    int matrizY = adapter.receiveInt();
                    int vidasCliente = adapter.receiveInt();
                    int puntosCliente = adapter.receiveInt();
                    
                    System.out.println("📍 Estado recibido (MATRIZ): [" + matrizX + "," + matrizY + "] Vidas:" + vidasCliente + " Puntos:" + puntosCliente);
                
                    // DEBUG ANTES de GameLogic
                    System.out.println("ANTES de GameLogic:");
                    System.out.println("   - Vidas actuales: " + vidas);
                    System.out.println("   - Puntos actuales: " + puntos);
                    System.out.println("   - Juego activo: " + juegoActivo);
                    
                    // 2. ACTUALIZAR GameLogic con coordenadas de matriz directamente
                    System.out.println("🎯 GameLogic - Actualizando jugador a: (" + matrizX + ", " + matrizY + ")");
                    
                    // 3. ACTUALIZAR GameLogic
                    gameLogic.updatePlayerFromClient(matrizX, matrizY);
                    
                    // 4. Sincronizar estado DESPUÉS de GameLogic
                    vidas = player.getLives();
                    puntos = player.getScore();
                    juegoActivo = (vidas > 0);
                    
                    System.out.println(" DESPUÉS de GameLogic:");
                    System.out.println("   - Vidas nuevas: " + vidas);
                    System.out.println("   - Puntos nuevos: " + puntos);
                    System.out.println("   - Juego activo: " + juegoActivo);
                    
                    // 5. Enviar CONSECUENCIAS
                    System.out.println(" Enviando consecuencias:");
                    System.out.println("   - Vidas: " + vidas);
                    System.out.println("   - Puntos: " + puntos);
                    System.out.println("   - Activo: " + juegoActivo);
                    
                    enviarConsecuencias(adapter, vidas, puntos, juegoActivo);
                }
                
                Thread.sleep(10);
                
            } catch (Exception e) {
                System.out.println(" Error en handleJugador: " + e.getMessage());
                e.printStackTrace();
                break;
            }
        }
        
        System.out.println("🏁 Cliente " + clientId + " finalizado - Vidas: " + vidas);
    }

    // ✅ MANTENER solo enviarConsecuencias - ELIMINAR el resto
    private void enviarConsecuencias(AdapterJ adapter, int vidas, int puntos, boolean activo) throws IOException {
        adapter.sendInt(vidas);
        adapter.sendInt(puntos);
        adapter.sendInt(activo ? 1 : 0);
        System.out.println("📤 Consecuencias: " + vidas + "," + puntos + "," + activo);
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