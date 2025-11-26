/**
 * Cocodrilo abstracto asociado siempre a una liana fija.
 * <p>
 * La posición X del cocodrilo viene dada por la liana. Solo se mueve en Y.
 */
public abstract class Croc {

    protected final Vine vine;   // siempre asociado a una vine fija
    protected Integer y;         // posición vertical del centro, en píxeles
    protected boolean alive = true;

    /**
     * Crea un cocodrilo sobre una liana.
     *
     * @param vine      liana a la que está asociado.
     * @param initialY  posición vertical inicial en píxeles.
     */
    protected Croc(final Vine vine,
                   final Integer initialY) {
        if (vine == null || initialY == null) {
            throw new IllegalArgumentException("Croc parameters must not be null");
        }
        this.vine = vine;
        this.y = initialY;
    }

    /**
     * @return liana asociada al cocodrilo.
     */
    public Vine getVine() {
        return vine;
    }

    /**
     * @return posición X del centro del cocodrilo (la X de la liana).
     */
    public Integer getX() {
        return vine.getX();
    }

    /**
     * @return posición Y del centro del cocodrilo.
     */
    public Integer getY() {
        return y;
    }

    /**
     * @return {@code true} si el cocodrilo sigue vivo.
     */
    public boolean isAlive() {
        return alive;
    }

    /**
     * @return rectángulo de colisión aproximado del cocodrilo.
     */
    public Rect getBounds() {
        final int size = GameConfig.CROCODILE_SIZE;
        final int half = size / 2;
        final int bx = vine.getX() - half;
        final int by = y - half;
        return new Rect(bx, by, size, size);
    }

    /**
     * Actualiza el estado del cocodrilo (movimiento, ciclo, etc.).
     * No realiza colisión con el jugador; eso es responsabilidad de GameLogic.
     */
    public abstract void update();

    /**
     * Marca el cocodrilo como muerto.
     */
    protected void kill() {
        this.alive = false;
    }
}
