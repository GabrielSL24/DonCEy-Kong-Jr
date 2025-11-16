import java.net.*;
import java.io.*;


public class ClientHandler implements Runnable {
    private Socket socket;
    private SocketServidor server;
    private AdapterJ adapter;
    private String clientType;
    private int clientId;
    private static int nextId = 1;
    

    public ClientHandler(Socket socket, SocketServidor server) {
        this.socket = socket;
        this.server = server;
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
        System.out.println("Jugador " + clientId + " listo para jugar");

        // Enviar confirmacion simple
        adapter.sendInt(100);

        // Simular juego por 10 segundos
        for (int i = 0; i < 5; i++) {
            int movimiento = adapter.receiveInt();
            System.out.println("Jugador " + clientId + " movio: " + movimiento);
            adapter.sendInt(movimiento * 10);
        }
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
