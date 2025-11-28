import javax.swing.*;
import java.awt.*;
import java.util.*;

public class Admin {

    private JTextArea areaLog;
    private JPanel panelClients;
    private Map<Integer, JLabel> labelsClientes;

    public Admin() {
        labelsClientes = new HashMap<>();

        SwingUtilities.invokeLater(() -> {
            MostrarGUI();
        });
    };

    private void MostrarGUI() {
        JFrame frame = new JFrame("Admin Panel - Control del Servidor");
        
        JButton buttonFruta = new JButton("Agregar Fruta");
        JButton buttonCroc = new JButton("Agregar Cocodrilo");
        areaLog = new JTextArea(10, 30);  
        areaLog.setEditable(false);

        // Panel de clientes
        panelClients = new JPanel();
        panelClients.setLayout(new BoxLayout(panelClients, BoxLayout.Y_AXIS));
        panelClients.setBorder(BorderFactory.createTitledBorder("Clientes Conectados"));
        panelClients.add(new JLabel("No hay clientes conectados"));

        //Acciones de los botones
        buttonFruta.addActionListener(e -> {
            areaLog.append("Fruta agregada\n");
        });

        buttonCroc.addActionListener(e -> {
            areaLog.append("Cocodrilo agregado\n");
        });

        JPanel panelButtons = new JPanel();
        panelButtons.add(buttonFruta);
        panelButtons.add(buttonCroc);

        JScrollPane scrollLog = new JScrollPane(areaLog);
        JScrollPane scrollClients = new JScrollPane(panelClients);
        scrollClients.setPreferredSize(new Dimension(250, 150));

        // Layout principal
        JPanel panelMain = new JPanel(new BorderLayout());

        // Panel superior con botones
        panelMain.add(panelButtons, BorderLayout.NORTH);
        
        // Panel central dividido
        JSplitPane splitPane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, scrollClients, scrollLog);
        splitPane.setResizeWeight(0.3);

        panelMain.add(splitPane, BorderLayout.CENTER);

        frame.add(panelMain);
        frame.setSize(700, 500);
        frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
        frame.setVisible(true);

        areaLog.append("Panel de administración iniciado\n");
    }

    //Metodo para agregar cliente al panel
    public void addClient(int clientId, String ip, String estado) {
        SwingUtilities.invokeLater(() -> {
            // Si es el primer cliente, removemos el label de "No hay clientes"
            if (panelClients.getComponentCount() == 1 &&
                panelClients.getComponent(0) instanceof JLabel &&
                ((JLabel) panelClients.getComponent(0)).getText().equals("No hay clientes conectados")) {
                panelClients.removeAll();
            }

            JLabel labelClient = new JLabel("Cliente " + clientId + " - " + ip + " - " + estado);
            labelClient.setBorder(BorderFactory.createEmptyBorder(2, 5, 2, 5));
            panelClients.add(labelClient);
            labelsClientes.put(clientId, labelClient);

            // Actualizar la interfaz
            panelClients.revalidate();
            panelClients.repaint();

            areaLog.append("Cliente " + clientId + " conectado desde: " + ip + "\n");
        });
    }

    public void removeClient(int clientId) {
        SwingUtilities.invokeLater(() -> {
            JLabel label = labelsClientes.remove(clientId);
            if (label != null) {
                panelClients.remove(label);

                // Si no hay más clientes, mostramos el mensaje
                if (panelClients.getComponentCount() == 0) {
                    panelClients.add(new JLabel("No hay clientes conectados"));
                }

                // Actualizar la interfaz
                panelClients.revalidate();
                panelClients.repaint();

                areaLog.append("Cliente " + clientId + " desconectado\n");
            }
        });
    }

    private void createFruit() {
        
    }

    private void createCrocodile() {

    }
}
