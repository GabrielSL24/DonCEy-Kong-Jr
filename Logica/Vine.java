/**
 * Liana fija del mapa, definida en coordenadas de píxel.
 * <p>
 * Corresponde a las estructuras {@code Liana} definidas en C, donde se
 * especifica la posición horizontal y el tramo vertical de la cuerda.
 */
public final class Vine {

    private final Integer id;
    private final Integer x;        // posición horizontal en píxeles
    private final Integer yTop;     // inicio de la liana (parte superior)
    private final Integer yBottom;  // fin de la liana (parte inferior)
    private final Integer halfWidth;

    /**
     * Crea una liana con un ancho efectivo de colisión por defecto.
     *
     * @param id      identificador lógico de la liana.
     * @param x       coordenada X en píxeles.
     * @param yTop    coordenada Y superior en píxeles.
     * @param yBottom coordenada Y inferior en píxeles.
     */
    public Vine(final Integer id,
                final Integer x,
                final Integer yTop,
                final Integer yBottom) {
        this(id, x, yTop, yBottom, Integer.valueOf(3)); // halfWidth por defecto (≈ LIANA_WIDTH/2)
    }

    /**
     * Crea una liana.
     *
     * @param id        identificador lógico de la liana.
     * @param x         coordenada X en píxeles.
     * @param yTop      coordenada Y superior en píxeles.
     * @param yBottom   coordenada Y inferior en píxeles.
     * @param halfWidth medio ancho efectivo para colisión horizontal, en píxeles.
     */
    public Vine(final Integer id,
                final Integer x,
                final Integer yTop,
                final Integer yBottom,
                final Integer halfWidth) {
        if (id == null || x == null || yTop == null || yBottom == null || halfWidth == null) {
            throw new IllegalArgumentException("Vine parameters must not be null");
        }
        this.id = id;
        this.x = x;
        this.yTop = yTop;
        this.yBottom = yBottom;
        this.halfWidth = halfWidth;
    }

    /**
     * @return identificador lógico de la liana.
     */
    public Integer getId() {
        return id;
    }

    /**
     * @return posición X de la liana en píxeles.
     */
    public Integer getX() {
        return x;
    }

    /**
     * @return coordenada Y superior de la liana en píxeles.
     */
    public Integer getYTop() {
        return yTop;
    }

    /**
     * @return coordenada Y inferior de la liana en píxeles.
     */
    public Integer getYBottom() {
        return yBottom;
    }

    /**
     * @return medio ancho efectivo de colisión horizontal, en píxeles.
     */
    public Integer getHalfWidth() {
        return halfWidth;
    }

    /**
     * @return rectángulo aproximado de la liana para fines de colisión AABB.
     */
    public Rect toRect() {
        final int hw = halfWidth;
        final int rx = x - hw;
        final int ry = yTop;
        final int width = hw * 2;
        final int height = yBottom - yTop;
        return new Rect(rx, ry, width, height);
    }

    /**
     * Indica si un punto está lo suficientemente "cerca" de la liana
     * como para considerarse agarrado a ella.
     *
     * @param px coordenada X del punto, en píxeles.
     * @param py coordenada Y del punto, en píxeles.
     * @return {@code true} si el punto está dentro del área efectiva de la liana.
     */
    public boolean contains(final Integer px, final Integer py) {
        if (px == null || py == null) {
            return false;
        }
        final int ipx = px;
        final int ipy = py;
        final int cx = x;
        final int top = yTop;
        final int bottom = yBottom;
        final int hw = halfWidth;

        return ipx >= cx - hw && ipx <= cx + hw
            && ipy >= top     && ipy <= bottom;
    }
}
