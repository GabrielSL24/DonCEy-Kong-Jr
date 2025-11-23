import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

/**
 * Lógica principal del juego (lado servidor, sin red).
 * - Mantiene jugador(es), crocs, frutas, vines fijas.
 * - Aplica reglas al recibir posiciones desde C.
 */
public class GameLogic {

    private final Vine[] vines;
    private final CrocManager crocManager;
    private final List<Fruit> fruits;

    private final Player player;      // si luego tienes 2, se extiende
    private int level = 1;
    private float speedMul = 1.0f;

    public GameLogic() {
        this.vines = GameConfig.createVines();
        this.crocManager = new CrocManager();
        this.fruits = new ArrayList<>();

        // Posición inicial de Jr (ajusta según tu mapa)
        this.player = new Player(10, 5, 3);
    }

    public Player getPlayer() { return player; }
    public Iterable<Croc> getCrocs() { return crocManager.getAllCrocs(); }
    public List<Fruit> getFruits() { return fruits; }
    public Vine[] getVines() { return vines; }

    public int getLevel() { return level; }
    public float getSpeedMul() { return speedMul; }

    // ================== Admin API ==================

    public boolean adminSpawnRedCroc(int vineId, int y, int baseSpeed) {
        Vine v = findVine(vineId);
        if (v == null) return false;
        int speed = Math.max(1, Math.round(baseSpeed * speedMul));
        return crocManager.spawnRed(v, y, speed) != null;
    }

    public boolean adminSpawnBlueCroc(int vineId, int y, int baseSpeed) {
        Vine v = findVine(vineId);
        if (v == null) return false;
        int speed = Math.max(1, Math.round(baseSpeed * speedMul));
        return crocManager.spawnBlue(v, y, speed) != null;
    }

    public void adminCreateFruit(int x, int y, int points) {
        fruits.add(new Fruit(x, y, points));
    }

    public void adminDeleteFruitAt(int x, int y) {
        Iterator<Fruit> it = fruits.iterator();
        while (it.hasNext()) {
            Fruit f = it.next();
            if (f.getX() == x && f.getY() == y) {
                it.remove();
                return;
            }
        }
    }

    // ================== Loop del servidor ==================

    /**
     * El cliente C envía la posición nueva del jugador.
     * Aquí la validamos y actualizamos estado del juego.
     */
    public void updatePlayerFromClient(int newX, int newY) {
        // Validación de límites
        if (newX < 0) newX = 0;
        if (newX >= 40) newX = 39;
        if (newY < 0) newY = 0;
        if (newY >= 30) newY = 29;
        
        System.out.println("GameLogic - Actualizando jugador a: (" + newX + ", " + newY + ")");
        
        // ✅ DEBUG: Verificar abismo ANTES de procesar
        boolean esAbismo = GameConfig.isAbyss(newX);
        System.out.println("🔍 DEBUG Abismo check: y=" + newX + " -> " + esAbismo);
        
        if (esAbismo) {
            System.out.println("JUGADOR CAYÓ AL ABISMO - Activando handlePlayerFall");
            handlePlayerFall();
            return;
        }

        // Actualizar posición
        player.setPosition(newX, newY);

        // DEBUG: Verificar meta
        boolean esMeta = GameConfig.isGoalPosition(newX, newY);
        System.out.println("🔍 DEBUG Meta check: (" + newX + "," + newY + ") -> " + esMeta);
        
        if (esMeta) {
            System.out.println("JUGADOR GANÓ - Activando handleWin");
            handleWin();
            return;
        }

        // DEBUG: Verificar colisiones
        System.out.println("DEBUG Verificando colisiones...");
        checkCrocCollisions();
        checkFruitCollisions();
        
        System.out.println("DEBUG: Update completado sin Game Over");
    }

    /** Llamar cada tick del servidor para mover crocs. */
    public void updateEnemies() {
        crocManager.updateAll();
    }

    // ================== Reglas internas ==================

    private void handlePlayerFall() {
       System.out.println("💀 handlePlayerFall() llamado - Vidas antes: " + player.getLives());
        player.loseLife();
        System.out.println("💀 Vidas después: " + player.getLives());
        
        if (player.getLives() <= 0) {
            System.out.println("🛑 GAME OVER - Sin vidas restantes");
            // Aquí deberías manejar el game over sin cerrar el servidor
        } else {
            System.out.println("🔄 Respawneando jugador...");
            respawnPlayer();
        }
    }

    private void handleWin() {
        System.out.println("🎉 handleWin() llamado");
        player.gainLife();
        level++;
        speedMul += 0.25f;
        System.out.println("🎉 Nuevo nivel: " + level + ", SpeedMul: " + speedMul);
        resetLevel();
    }

    private void resetLevel() {
        // Limpia crocs y frutas, respawnea jugador, etc.
        // Simplificado:
        fruits.clear();
        // no hay método clear en CrocManager, pero podrías recrearlo o añadir clear().
        respawnPlayer();
    }

    private void respawnPlayer() {
        // Ajusta a la posición inicial de tu mapa
        player.setPosition(2, 26);
        player.setOnVine(false);
    }

    private void checkCrocCollisions() {
        int px = player.getX();
        int py = player.getY();

        System.out.println("🔍 DEBUG checkCrocCollisions - Jugador en: (" + px + "," + py + ")");

        for (Croc c : crocManager.getAllCrocs()) {
            if (c != null && c.isAlive()) {
                System.out.println("🔍 DEBUG Cocodrilo en: (" + c.getX() + "," + c.getY() + ")");
                if (c.getX() == px && c.getY() == py) {
                    System.out.println("🐊 COLISIÓN CON COCODRILO DETECTADA");
                    player.loseLife();
                    if (player.getLives() <= 0) {
                        System.out.println("🛑 GAME OVER por cocodrilo");
                    } else {
                        System.out.println("🔄 Respawn por cocodrilo");
                        respawnPlayer();
                    }
                    return;
                }
            }
        }
        System.out.println("✅ DEBUG: Sin colisiones con cocodrilos");
    }

    private void checkFruitCollisions() {
        int px = player.getX();
        int py = player.getY();

        for (Fruit f : fruits) {
            if (f.isActive() && f.getX() == px && f.getY() == py) {
                player.addScore(f.getPoints());
                f.collect();
            }
        }
    }

    private boolean isOnAnyVine(int x, int y) {
        for (Vine v : vines) {
            if (v.contains(x, y)) return true;
        }
        return false;
    }

    private Vine findVine(int vineId) {
        for (Vine v : vines) {
            if (v.getId() == vineId) return v;
        }
        return null;
    }
}
