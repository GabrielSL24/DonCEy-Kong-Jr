/**
 * Fruta generada por el administrador del juego.
 * <p>
 * La fruta tiene una posición fija en el mapa, otorga una cantidad de puntos
 * al ser recogida y luego pasa a estado inactivo.
 */
public final class Fruit {
    private final Integer id; 
    private final Integer x;
    private final Integer y;
    private final Integer points;
    private boolean active = true;

    /**
     * Crea una fruta.
     *
     * @param x      coordenada X del centro de la fruta, en píxeles.
     * @param y      coordenada Y del centro de la fruta, en píxeles.
     * @param points puntos que otorga al ser recogida.
     */
    public Fruit(final Integer id,
                 final Integer x,
                 final Integer y,
                 final Integer points) {
        if (x == null || y == null || points == null) {
            throw new IllegalArgumentException("Fruit parameters must not be null");
        }
        this.x = x;
        this.y = y;
        this.points = points;
        this.id = id;
    }

    /**
     * @return coordenada X del centro, en píxeles.
     */
    public Integer getX() {
        return x;
    }

    /**
     * @return coordenada Y del centro, en píxeles.
     */
    public Integer getY() {
        return y;
    }

    /**
     * @return puntos que otorga esta fruta.
     */
    public Integer getPoints() {
        return points;
    }

    /**
     * @return {@code true} si la fruta sigue disponible; {@code false} si ya fue recogida.
     */
    public boolean isActive() {
        return active;
    }

    /**
     * Marca la fruta como recogida. No cambia la puntuación del jugador,
     * esa responsabilidad es de la lógica de juego.
     */
    public void collect() {
        active = false;
    }

    public Integer getId() {
        return id;
    }

    /**
     * Devuelve el rectángulo de colisión aproximado de la fruta.
     * <p>
     * Usa {@link GameConfig#FRUIT_SIZE} como tamaño cuadrado.
     *
     * @return rectángulo AABB centrado en (x, y).
     */
    public Rect getBounds() {
        final int size = GameConfig.FRUIT_SIZE;
        final int half = size / 2;
        final int bx = x - half;
        final int by = y - half;
        return new Rect(bx, by, size, size);
    }
}
