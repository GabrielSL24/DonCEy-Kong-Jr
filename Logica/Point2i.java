/**
 * Punto 2D inmutable en coordenadas de píxel.
 * <p>
 * Se usa para representar posiciones lógicas en el mapa
 * (por ejemplo, spawn del jugador, posición de la meta, etc.).
 */
public final class Point2i {

    private final Integer x;
    private final Integer y;

    /**
     * Crea un punto 2D.
     *
     * @param x coordenada X en píxeles. No debe ser {@code null}.
     * @param y coordenada Y en píxeles. No debe ser {@code null}.
     * @throws IllegalArgumentException si {@code x} o {@code y} son {@code null}.
     */
    public Point2i(final Integer x, final Integer y) {
        if (x == null || y == null) {
            throw new IllegalArgumentException("x and y must not be null");
        }
        this.x = x;
        this.y = y;
    }

    /**
     * @return coordenada X en píxeles.
     */
    public Integer getX() {
        return x;
    }

    /**
     * @return coordenada Y en píxeles.
     */
    public Integer getY() {
        return y;
    }
}
