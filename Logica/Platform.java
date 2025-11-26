/**
 * Plataforma (isla) fija del nivel, representada como un rectángulo.
 * <p>
 * Las coordenadas están en píxeles y corresponden directamente a las
 * posiciones que ya existen en la interfaz C.
 */
public final class Platform {

    private final Rect bounds;

    /**
     * Crea una plataforma rectangular.
     *
     * @param x      coordenada X de la esquina superior izquierda, en píxeles.
     * @param y      coordenada Y de la esquina superior izquierda, en píxeles.
     * @param width  ancho de la plataforma en píxeles.
     * @param height alto de la plataforma en píxeles.
     */
    public Platform(final Integer x,
                    final Integer y,
                    final Integer width,
                    final Integer height) {
        this.bounds = new Rect(x, y, width, height);
    }

    /**
     * @return rectángulo de colisión de la plataforma.
     */
    public Rect getBounds() {
        return bounds;
    }

    /**
     * @return coordenada X en píxeles.
     */
    public Integer getX() {
        return bounds.getX();
    }

    /**
     * @return coordenada Y en píxeles.
     */
    public Integer getY() {
        return bounds.getY();
    }

    /**
     * @return ancho de la plataforma en píxeles.
     */
    public Integer getWidth() {
        return bounds.getWidth();
    }

    /**
     * @return alto de la plataforma en píxeles.
     */
    public Integer getHeight() {
        return bounds.getHeight();
    }
}
