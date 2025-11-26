/**
 * Rectángulo alineado a ejes (AABB) en coordenadas de píxel.
 * <p>
 * Se utiliza como base para detección de colisiones y para definir
 * áreas como plataformas, agua, etc.
 */
public final class Rect {

    private final Integer x;
    private final Integer y;
    private final Integer width;
    private final Integer height;

    /**
     * Crea un rectángulo.
     *
     * @param x      coordenada X de la esquina superior izquierda, en píxeles.
     * @param y      coordenada Y de la esquina superior izquierda, en píxeles.
     * @param width  ancho del rectángulo en píxeles.
     * @param height alto del rectángulo en píxeles.
     * @throws IllegalArgumentException si algún parámetro es {@code null}.
     */
    public Rect(final Integer x,
                final Integer y,
                final Integer width,
                final Integer height) {
        if (x == null || y == null || width == null || height == null) {
            throw new IllegalArgumentException("Rect parameters must not be null");
        }
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
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

    /**
     * @return ancho del rectángulo en píxeles.
     */
    public Integer getWidth() {
        return width;
    }

    /**
     * @return alto del rectángulo en píxeles.
     */
    public Integer getHeight() {
        return height;
    }

    /**
     * Indica si el punto dado está dentro del rectángulo (bordes incluidos).
     *
     * @param px coordenada X del punto, en píxeles.
     * @param py coordenada Y del punto, en píxeles.
     * @return {@code true} si el punto está dentro; {@code false} en caso contrario
     *         o si alguno de los parámetros es {@code null}.
     */
    public boolean contains(final Integer px, final Integer py) {
        if (px == null || py == null) {
            return false;
        }
        final int ix = x;
        final int iy = y;
        final int iw = width;
        final int ih = height;
        final int ipx = px;
        final int ipy = py;

        return ipx >= ix && ipx <= ix + iw
            && ipy >= iy && ipy <= iy + ih;
    }

    /**
     * Determina si este rectángulo se intersecta con otro usando AABB.
     *
     * @param other otro rectángulo. Si es {@code null}, devuelve {@code false}.
     * @return {@code true} si hay intersección entre ambos rectángulos.
     */
    public boolean intersects(final Rect other) {
        if (other == null) {
            return false;
        }
        final int ix = x;
        final int iy = y;
        final int iw = width;
        final int ih = height;

        final int ox = other.x;
        final int oy = other.y;
        final int ow = other.width;
        final int oh = other.height;

        return ix < ox + ow &&
               ix + iw > ox &&
               iy < oy + oh &&
               iy + ih > oy;
    }
}
