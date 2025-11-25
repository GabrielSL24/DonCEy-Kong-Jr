import java.io.*;
import java.net.Socket;
import java.nio.charset.StandardCharsets;

//Adapter basico para enviar/recepcionar mensajes entre Java y C
public class AdapterJ {
    private DataInputStream input;
    private DataOutputStream output;
    private Socket socket;

    public AdapterJ(Socket socket) throws IOException {
        this.socket = socket;
        this.input = new DataInputStream(socket.getInputStream());
        this.output = new DataOutputStream(socket.getOutputStream());
    }

    public boolean hayDatosDisponibles() throws IOException {
        return input.available() > 0;
    }

    public void sendInt(int value) throws IOException {
        output.writeInt(value);
        output.flush();
        System.out.println("Adapter: Enviando entero -> " + value);
    }

    public int receiveInt() throws IOException {
        int value = input.readInt();
        System.out.println("Adapter: Recibido entero -> " + value);;
        return value;
    }

    public void sendString(String data) throws IOException {
        byte[] bytes = data.getBytes(StandardCharsets.UTF_8);
        output.writeInt(bytes.length);          //Enviar longitud
        output.write(bytes);                    //Enviar datos  
        output.flush();
        System.out.println("Adapter: Enviando string (" + bytes.length + " bytes)");
    }

    public String receiveString() throws IOException {
        int length = input.readInt();
        if (length <= 0 || length > 10000) {
            throw new IOException("Tamaño de string invalido: " + length);
        }
        
        byte[] bytes = new byte[length];
        input.readFully(bytes);
        String result = new String(bytes, StandardCharsets.UTF_8);
        System.out.println("Adapter: Recibido string (" + length + " bytes)");
        return result;
    }


    public void sendIdentification(String clientType) throws IOException {
        int code = getClientType(clientType);
        sendInt(code);
        System.out.println("Adapter: Enviada identidicacion -> "+ clientType + " (Codigo : " + code + ")");
    }

    public String receiveIdentification() throws IOException {
        int code = receiveInt();
        return getClientTypeFrom(code);
    }

    private int getClientType(String clientType) {
        switch (clientType.toUpperCase()) {
            case "JUGADOR": return 1;
            case "ESPECTADOR": return 2;
            case "ADMIN": return 3;
            case "SERVIDOR": return 4;
            default: return 0;
        }
    }

    private String getClientTypeFrom(int code) {
        switch (code) {
            case 1: return "JUGADOR";
            case 2: return "ESPECTADOR";
            case 3: return "ADMIN";
            case 4: return "SERVIDOR";  
            default: return "DESCONOCIDO";
        }
    }

    public void close() {
        try {
            if (input != null) {
                input.close();
            }
            if (output != null) {
                output.close();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

}