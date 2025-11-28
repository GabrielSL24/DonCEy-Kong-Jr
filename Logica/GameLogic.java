import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

/**
 * Lógica principal del juego (lado servidor, sin red).
 * <p>
 * Esta versión implementa:
 * <ul>
 *     <li>Mapa estático (plataformas, lianas, meta, abismo) vía {@link GameConfig}.</li>
 *     <li>Jugador con movimiento y colisión contra plataformas.</li>
 *     <li>Administración de cocodrilos y frutas creados por el administrador.</li>
 * </ul>
 */
public final class GameLogic {

    private final List<Platform> platforms;
    private final List<Vine> vines;
    private final Point2i goalPosition;
    private static final int DEATH_Y = GameConfig.SCREEN_HEIGHT;


    private final Point2i playerSpawn;

    private boolean gameOver;

    private final Player player;

    private final CrocManager crocManager;
    private final List<Fruit> fruits;

    private int level;

    private int nextFruitId = 1;


    

    /**
     * Crea un nuevo estado de juego con el nivel por defecto.
     */
    public GameLogic() {
        this.platforms = GameConfig.createPlatforms();
        this.vines = GameConfig.createVines();
        this.goalPosition = GameConfig.createGoalPosition();

        this.playerSpawn = GameConfig.createPlayerSpawn(platforms);
        this.player = new Player(
                playerSpawn.getX(),
                playerSpawn.getY(),
                Integer.valueOf(3),          // vidas iniciales
                GameConfig.PLAYER_SIZE       // tamaño del jugador
        );

        this.crocManager = new CrocManager();
        this.fruits = new ArrayList<>();
        this.gameOver = false;
        this.level = 1;
    }

    // ==================== BUCLE PRINCIPAL ====================

    /**
     * Actualiza la lógica del juego para un frame.
     *
     * @param input estado de entrada del jugador recibido desde el cliente.
     */
    public void update(final PlayerInput input) {
        if (gameOver) {
            return;
        }

        // 1) Actualizar movimiento del jugador con colisión contra plataformas
        player.update(input, platforms, vines);

        // 2) Actualizar cocodrilos
        crocManager.updateAll();
        crocManager.removeDead();

        // 3) Colisión jugador/cocodrilos
        final Rect playerBounds = player.getBounds();
        for (Croc croc : crocManager.getAllCrocs()) {
            if (croc != null && croc.isAlive() && playerBounds.intersects(croc.getBounds())) {
                onPlayerKilled();
                return;
            }
        }

        // 4) Colisión jugador/frutas
        for (Fruit fruit : fruits) {
            if (fruit != null && fruit.isActive() &&
                playerBounds.intersects(fruit.getBounds())) {

                // sumar puntos
                player.addScore(fruit.getPoints());

                // marcar fruta como recogida
                fruit.collect();
            }
        }

        // Colisión con abismo (caída fuera del nivel)
        if (player.getY() > GameConfig.SCREEN_HEIGHT) {
            onPlayerKilled();
            return;
}
        // 6) Victoria si llega a la meta
        if (checkVictory()) {
            onVictory();
        }
    }

    // ==================== GETTERS DE ESTADO ====================

    public Player getPlayer() {
        return player;
    }

    /**
     * Entrada: ninguna.
     * Restricción: ninguna.
     * Salida: true si la partida ya terminó.
     */
    public boolean isGameOver() {
        return gameOver;
    }

    public int getLevel() {
        return level;
    }

    public List<Platform> getPlatforms() {
        return new ArrayList<>(platforms);
    }

    public List<Vine> getVines() {
        return new ArrayList<>(vines);
    }

    public Point2i getGoalPosition() {
        return goalPosition;
    }

    /**
     * @return colección de todos los cocodrilos.
     */
    public Collection<Croc> getCrocs() {
        return crocManager.getAllCrocs();
    }

    /**
     * @return lista de frutas activas/inactivas.
     */
    public List<Fruit> getFruits() {
        return new ArrayList<>(fruits);
    }

    // ==================== API PARA ADMIN ====================

    /**
     * Crea un cocodrilo rojo en la liana indicada, si las reglas lo permiten.
     *
     * @param vineId   identificador de la liana.
     * @param initialY posición vertical inicial en píxeles.
     * @param speed    velocidad del cocodrilo.
     * @return {@code true} si se creó con éxito; {@code false} si no se pudo.
     */
    public boolean adminSpawnRedCroc(final Integer vineId) {
        if (vineId == null) {
            return false;
        }
        final Vine vine = findVine(vineId);
        if (vine == null) {
            return false;
        }
        if (!crocManager.canSpawnOn(vine)) {
            return false;
        }
        final Croc croc = new RedCroc(vine);
        crocManager.putCroc(croc);
        return true;
    }

    /**
     * Crea un cocodrilo azul en la liana indicada, si las reglas lo permiten.
     */
    public boolean adminSpawnBlueCroc(final Integer vineId) {
        if (vineId == null) {
            return false;
        }
        final Vine vine = findVine(vineId);
        if (vine == null) {
            return false;
        }
        if (!crocManager.canSpawnOn(vine)) {
            return false;
        }
        final Croc croc = new BlueCroc(vine);
        crocManager.putCroc(croc);
        return true;
    }

    /**
     * Crea una fruta sobre una liana.
     *
     * @param vineId  identificador de la liana.
     * @param offsetY desplazamiento desde la parte superior de la liana, en píxeles.
     *                Debe estar entre 0 y vine.getLength().
     * @param points  puntos que otorga la fruta.
     * @return {@code true} si se creó con éxito; {@code false} si algún dato es inválido.
     */
    public boolean adminSpawnFruitOnVine(final Integer vineId,
                                        final Integer offsetY,
                                        final Integer points) {
        if (vineId == null || offsetY == null || points == null) {
            return false;
        }

        final Vine vine = findVine(vineId);
        if (vine == null) {
            return false;
        }

        final int length = vine.getLength();
        if (offsetY < 0 || offsetY > length) {
            return false;
        }

        final int id = generateFruitId();
        final int x = vine.getCenterX();
        final int y = vine.getYTop() + offsetY;

        final Fruit fruit = new Fruit(id, x, y, points);
        fruits.add(fruit);

        return true;
    }



    /**
     * Elimina la fruta con el ID especificado.
     *
     * @param fruitId ID único de la fruta.
     * @return true si la fruta fue encontrada y eliminada, false si no existe.
     */
    public boolean adminRemoveFruit(final Integer fruitId) {
        if (fruitId == null) {
            return false;
        }

        for (int i = 0; i < fruits.size(); i++) {
            final Fruit f = fruits.get(i);
            if (f != null && fruitId.equals(f.getId())) {
                fruits.remove(i);
                return true;
            }
        }

        return false;
    }


    /**
     * Elimina todas las frutas y cocodrilos.
     */
    public void adminClearEntities() {
        crocManager.clear();
        fruits.clear();
    }

    // ==================== HELPERS PRIVADOS ====================

    /**
     * Entrada: ninguna.
     * Restricción: playerSpawn debe ser una posición válida.
     * Salida: Reposiciona al jugador al punto de aparición inicial.
     */
    private void respawnPlayer() {
        player.resetTo(playerSpawn.getX(), playerSpawn.getY());
    }

    /**
     * Entrada: ninguna.
     * Restricción: Debe existir un jugador válido.
     * Salida: Resta una vida al jugador y decide si hay respawn o game over.
     */
    private void onPlayerKilled() {
        player.loseLife();
        if (player.getLives() <= 0) {
            gameOver = true;
        } else {
            respawnPlayer();
        }
    }

    private Vine findVine(final Integer vineId) {
        if (vineId == null) {
            return null;
        }
        for (Vine v : vines) {
            if (v.getId().equals(vineId)) {
                return v;
            }
        }
        return null;
    }

    private boolean checkVictory() {
        Rect r = player.getBounds();
        return r.contains(goalPosition.getX(), goalPosition.getY());
    }

    private void onVictory() {
        level++;

        // aumentar velocidad de cocodrilos
        crocManager.increaseSpeedMultiplier();

        // respawn
        respawnPlayer();
    }

    private int generateFruitId() {
        return nextFruitId++;
    }

}