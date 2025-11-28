import javax.swing.*;
import java.awt.*;
import java.util.*;
import java.util.List;
import java.awt.event.*;

public class Admin {

    private JTextArea areaLog;
    private JPanel panelClients;
    private Map<Integer, JLabel> labelsClientes;
    private SocketServidor server;
    private JComboBox<String> comboPartidas;
    private JComboBox<String> comboLianas;
    private JTextField txtPuntosFruta;
    private JTextField txtAlturaFruta;
    private Map<String, String> partidasMap;

    public Admin(SocketServidor server) {
        this.server = server;
        labelsClientes = new HashMap<>();
        partidasMap = new HashMap<>();

        SwingUtilities.invokeLater(() -> {
            MostrarGUI();
            actualizarListaPartidas();
        });
    }

    private void MostrarGUI() {
        JFrame frame = new JFrame("Admin Panel - Control del Servidor");
        
        // Botones principales
        JButton buttonFruta = new JButton("Agregar Fruta");
        JButton buttonCrocRojo = new JButton("Agregar Cocodrilo Rojo");
        JButton buttonCrocAzul = new JButton("Agregar Cocodrilo Azul");
        JButton buttonEliminarFruta = new JButton("Eliminar Fruta");
        JButton buttonActualizar = new JButton("Actualizar Lista"); 
        
        areaLog = new JTextArea(10, 30);  
        areaLog.setEditable(false);

        // Panel de seleccion de partida
        JPanel panelPartida = new JPanel(new FlowLayout());
        panelPartida.setBorder(BorderFactory.createTitledBorder("Seleccionar Partida"));

        comboPartidas = new JComboBox<>();
        comboPartidas.setPreferredSize(new Dimension(300, 25));
        panelPartida.add(new JLabel("Partida: "));
        panelPartida.add(comboPartidas);
        panelPartida.add(buttonActualizar);

        // Panel de lianas
        JPanel panelLianas = new JPanel(new FlowLayout());
        panelLianas.setBorder(BorderFactory.createTitledBorder("Lianas Disponibles"));
        comboLianas = new JComboBox<>();
        for (int i = 0; i <= 8; i++) {
            comboLianas.addItem("Liana " + i);
        }
        panelLianas.add(new JLabel("Liana: "));
        panelLianas.add(comboLianas);

        // Panel de configuracion de frutas
        JPanel panelFrutas = new JPanel(new GridLayout(2, 2, 5, 5));
        panelFrutas.setBorder(BorderFactory.createTitledBorder("Configuración de Frutas"));
        panelFrutas.add(new JLabel("Puntos:"));
        txtPuntosFruta = new JTextField("100");
        panelFrutas.add(txtPuntosFruta);
        panelFrutas.add(new JLabel("Altura (offsetY):"));
        txtAlturaFruta = new JTextField("100");
        panelFrutas.add(txtAlturaFruta);

        // Panel de clientes
        panelClients = new JPanel();
        panelClients.setLayout(new BoxLayout(panelClients, BoxLayout.Y_AXIS));
        panelClients.setBorder(BorderFactory.createTitledBorder("Clientes Conectados"));
        panelClients.add(new JLabel("No hay clientes conectados"));

        // Acciones de los botones
        buttonActualizar.addActionListener(e -> actualizarListaPartidas());
        buttonFruta.addActionListener(e -> crearFruta());
        buttonCrocRojo.addActionListener(e -> crearCocodrilo("ROJO"));
        buttonCrocAzul.addActionListener(e -> crearCocodrilo("AZUL"));
        buttonEliminarFruta.addActionListener(e -> eliminarFruta());

        // Panel de botones de entidades
        JPanel panelBotonesEntidades = new JPanel(new GridLayout(2, 2, 5, 5));
        panelBotonesEntidades.add(buttonFruta);
        panelBotonesEntidades.add(buttonEliminarFruta);
        panelBotonesEntidades.add(buttonCrocRojo);
        panelBotonesEntidades.add(buttonCrocAzul);

        // Panel izquierdo (controles)
        JPanel panelControles = new JPanel();
        panelControles.setLayout(new BoxLayout(panelControles, BoxLayout.Y_AXIS));
        panelControles.add(panelPartida);
        panelControles.add(Box.createRigidArea(new Dimension(0, 10)));
        panelControles.add(panelLianas);
        panelControles.add(Box.createRigidArea(new Dimension(0, 10)));
        panelControles.add(panelFrutas);
        panelControles.add(Box.createRigidArea(new Dimension(0, 10)));
        panelControles.add(panelBotonesEntidades);

        JScrollPane scrollLog = new JScrollPane(areaLog);
        JScrollPane scrollClients = new JScrollPane(panelClients);
        scrollClients.setPreferredSize(new Dimension(250, 150));

        // Layout principal
        JPanel panelMain = new JPanel(new BorderLayout());
        panelMain.add(panelControles, BorderLayout.NORTH);
        
        JSplitPane splitPane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, scrollClients, scrollLog);
        splitPane.setResizeWeight(0.3);

        panelMain.add(splitPane, BorderLayout.CENTER);

        frame.add(panelMain);
        frame.setSize(800, 600);
        frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
        frame.setVisible(true);

        areaLog.append("Panel de administración iniciado\n");
        areaLog.append("Use 'Actualizar Lista' para ver las partidas activas\n");
    }

    private void actualizarListaPartidas() {
        try {
            List<SocketServidor.InfoPartida> partidas = server.obtenerListaPartidas();
            comboPartidas.removeAllItems();
            partidasMap.clear();
            
            if (partidas.isEmpty()) {
                comboPartidas.addItem("No hay partidas activas");
                areaLog.append("No se encontraron partidas activas\n");
            } else {
                for (SocketServidor.InfoPartida partida : partidas) {
                    String display = String.format("%s (Jugadores: %d, Espectadores: %d)", 
                        partida.gameId, partida.playerCount, partida.spectators);
                    comboPartidas.addItem(display);
                    partidasMap.put(display, partida.gameId);
                }
                areaLog.append("Lista de partidas actualizada. Encontradas: " + partidas.size() + "\n");
            }
        } catch (Exception e) {
            areaLog.append("Error actualizando lista de partidas: " + e.getMessage() + "\n");
        }
    }

    private void crearFruta() {
        String partidaSeleccionada = (String) comboPartidas.getSelectedItem();
        if (partidaSeleccionada == null || partidaSeleccionada.equals("No hay partidas activas")) {
            JOptionPane.showMessageDialog(null, "Seleccione una partida válida");
            return;
        }

        String lianaSeleccionada = (String) comboLianas.getSelectedItem();
        if (lianaSeleccionada == null) {
            JOptionPane.showMessageDialog(null, "Seleccione una liana");
            return;
        }

        try {
            // Extraer el ID de la liana (0-8)
            int idLiana = Integer.parseInt(lianaSeleccionada.split(" ")[1]);
            int puntos = Integer.parseInt(txtPuntosFruta.getText());
            int offsetY = Integer.parseInt(txtAlturaFruta.getText());
            
            String gameId = partidasMap.get(partidaSeleccionada);
            SocketServidor.Partida partida = server.obtenerPartida(gameId);
            
            if (partida != null && partida.logica != null) {
                // CORREGIDO: Usar directamente el ID de la liana (0-8)
                boolean exito = partida.logica.adminSpawnFruitOnVine(idLiana, offsetY, puntos);
                
                if (exito) {
                    areaLog.append(String.format("✅ Fruta creada en Liana %d, offsetY %d, %d puntos\n", 
                        idLiana, offsetY, puntos));
                } else {
                    areaLog.append(String.format("❌ Error creando fruta en Liana %d (offsetY fuera de rango o liana no existe)\n", idLiana));
                }
            } else {
                areaLog.append("❌ Partida no encontrada o lógica no disponible\n");
            }
        } catch (NumberFormatException ex) {
            JOptionPane.showMessageDialog(null, "Ingrese valores numéricos válidos para puntos y altura");
        } catch (Exception ex) {
            areaLog.append("Error creando fruta: " + ex.getMessage() + "\n");
            ex.printStackTrace();
        }
    }

    private void crearCocodrilo(String tipo) {
        String partidaSeleccionada = (String) comboPartidas.getSelectedItem();
        if (partidaSeleccionada == null || partidaSeleccionada.equals("No hay partidas activas")) {
            JOptionPane.showMessageDialog(null, "Seleccione una partida válida");
            return;
        }

        String lianaSeleccionada = (String) comboLianas.getSelectedItem();
        if (lianaSeleccionada == null) {
            JOptionPane.showMessageDialog(null, "Seleccione una liana");
            return;
        }

        try {
            // Extraer el ID de la liana (0-8)
            int idLiana = Integer.parseInt(lianaSeleccionada.split(" ")[1]);
            String gameId = partidasMap.get(partidaSeleccionada);
            SocketServidor.Partida partida = server.obtenerPartida(gameId);
            
            if (partida != null && partida.logica != null) {
                boolean exito = false;
                
                if ("ROJO".equals(tipo)) {
                    exito = partida.logica.adminSpawnRedCroc(idLiana);
                } else {
                    exito = partida.logica.adminSpawnBlueCroc(idLiana);
                }
                
                if (exito) {
                    areaLog.append(String.format("✅ Cocodrilo %s creado en Liana %d\n", tipo, idLiana));
                } else {
                    areaLog.append(String.format("❌ Error creando cocodrilo %s en Liana %d (ya existe un cocodrilo vivo en esa liana)\n", tipo, idLiana));
                }
            } else {
                areaLog.append("❌ Partida no encontrada o lógica no disponible\n");
            }
        } catch (Exception ex) {
            areaLog.append("Error creando cocodrilo: " + ex.getMessage() + "\n");
            ex.printStackTrace();
        }
    }

    private void eliminarFruta() {
        String partidaSeleccionada = (String) comboPartidas.getSelectedItem();
        if (partidaSeleccionada == null || partidaSeleccionada.equals("No hay partidas activas")) {
            JOptionPane.showMessageDialog(null, "Seleccione una partida válida");
            return;
        }

        String lianaSeleccionada = (String) comboLianas.getSelectedItem();
        if (lianaSeleccionada == null) {
            JOptionPane.showMessageDialog(null, "Seleccione una liana");
            return;
        }

        try {
            // Extraer el ID de la liana (0-8)
            int idLiana = Integer.parseInt(lianaSeleccionada.split(" ")[1]);
            int offsetY = Integer.parseInt(txtAlturaFruta.getText());
            
            String gameId = partidasMap.get(partidaSeleccionada);
            SocketServidor.Partida partida = server.obtenerPartida(gameId);
            
            if (partida != null && partida.logica != null) {
                // CORREGIDO: Usar directamente el ID de la liana
                Integer fruitId = partida.logica.buscarIdFruta(idLiana, offsetY);
                boolean exito = false;
                
                if (fruitId != null) {
                    exito = partida.logica.adminRemoveFruit(fruitId);
                }
                
                if (exito) {
                    areaLog.append(String.format("✅ Fruta eliminada de Liana %d, offsetY %d\n", 
                        idLiana, offsetY));
                } else {
                    areaLog.append(String.format("❌ No se encontró fruta activa en Liana %d, offsetY %d\n", 
                        idLiana, offsetY));
                }
            } else {
                areaLog.append("❌ Partida no encontrada o lógica no disponible\n");
            }
        } catch (NumberFormatException ex) {
            JOptionPane.showMessageDialog(null, "Ingrese una altura numérica válida");
        } catch (Exception ex) {
            areaLog.append("Error eliminando fruta: " + ex.getMessage() + "\n");
            ex.printStackTrace();
        }
    }

    // Método para agregar/remover clientes al panel
    public void addClient(int clientId, String ip, String estado) {
        SwingUtilities.invokeLater(() -> {
            if (panelClients.getComponentCount() == 1 &&
                panelClients.getComponent(0) instanceof JLabel &&
                ((JLabel) panelClients.getComponent(0)).getText().equals("No hay clientes conectados")) {
                panelClients.removeAll();
            }

            JLabel labelClient = new JLabel("Cliente " + clientId + " - " + ip + " - " + estado);
            labelClient.setBorder(BorderFactory.createEmptyBorder(2, 5, 2, 5));
            panelClients.add(labelClient);
            labelsClientes.put(clientId, labelClient);

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

                if (panelClients.getComponentCount() == 0) {
                    panelClients.add(new JLabel("No hay clientes conectados"));
                }

                panelClients.revalidate();
                panelClients.repaint();

                areaLog.append("Cliente " + clientId + " desconectado\n");
            }
        });
    }
}