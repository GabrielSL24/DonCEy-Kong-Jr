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
    private final Rect abyssZone;

    private final Player player;

    private final CrocManager crocManager;
    private final List<Fruit> fruits;

    /**
     * Crea un nuevo estado de juego con el nivel por defecto.
     */
    public GameLogic() {
        this.platforms = GameConfig.createPlatforms();
        this.vines = GameConfig.createVines();
        this.goalPosition = GameConfig.createGoalPosition();
        this.abyssZone = GameConfig.getAbyssZone();

        final Point2i spawn = GameConfig.createPlayerSpawn(platforms);
        this.player = new Player(
                spawn.getX(),
                spawn.getY(),
                Integer.valueOf(3),          // vidas iniciales
                GameConfig.PLAYER_SIZE       // tamaño del jugador
        );

        this.crocManager = new CrocManager();
        this.fruits = new ArrayList<>();
    }

    // ==================== BUCLE PRINCIPAL ====================

    /**
     * Actualiza la lógica del juego para un frame.
     *
     * @param input estado de entrada del jugador recibido desde el cliente.
     */
    public void update(final PlayerInput input) {
        // 1) Actualizar movimiento del jugador con colisión contra plataformas
        player.update(input, platforms, vines);

        // 2) Actualizar cocodrilos
        crocManager.updateAll();
        crocManager.removeDead();

        // 3) Próximos pasos:
        //    - colisión jugador/cocodrilos
        //    - colisión jugador/frutas
        //    - colisión con meta
        //    - colisión con abismo
    }

    // ==================== GETTERS DE ESTADO ====================

    public Player getPlayer() {
        return player;
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

    public Rect getAbyssZone() {
        return abyssZone;
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
    public boolean adminSpawnRedCroc(final Integer vineId,
                                     final Integer initialY,
                                     final Integer speed) {
        if (vineId == null || initialY == null || speed == null) {
            return false;
        }
        final Vine vine = findVine(vineId);
        if (vine == null) {
            return false;
        }
        if (!crocManager.canSpawnOn(vine)) {
            return false;
        }
        final Croc croc = new RedCroc(vine, initialY, speed);
        crocManager.putCroc(croc);
        return true;
    }

    /**
     * Crea un cocodrilo azul en la liana indicada, si las reglas lo permiten.
     */
    public boolean adminSpawnBlueCroc(final Integer vineId,
                                      final Integer initialY,
                                      final Integer speed) {
        if (vineId == null || initialY == null || speed == null) {
            return false;
        }
        final Vine vine = findVine(vineId);
        if (vine == null) {
            return false;
        }
        if (!crocManager.canSpawnOn(vine)) {
            return false;
        }
        final Croc croc = new BlueCroc(vine, initialY, speed);
        crocManager.putCroc(croc);
        return true;
    }

    /**
     * Crea una fruta en la posición indicada.
     *
     * @param x      coordenada X del centro, en píxeles.
     * @param y      coordenada Y del centro, en píxeles.
     * @param points puntos que otorgará al ser recogida.
     * @return la fruta creada, o {@code null} si los parámetros son inválidos.
     */
    public Fruit adminSpawnFruit(final Integer x,
                                 final Integer y,
                                 final Integer points) {
        if (x == null || y == null || points == null) {
            return null;
        }
        final Fruit fruit = new Fruit(x, y, points);
        fruits.add(fruit);
        return fruit;
    }

    /**
     * Elimina todas las frutas y cocodrilos.
     */
    public void adminClearEntities() {
        crocManager.clear();
        fruits.clear();
    }

    // ==================== HELPERS PRIVADOS ====================

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
}