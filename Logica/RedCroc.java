/**
 * Cocodrilo rojo: se mueve subiendo y bajando entre los extremos de la liana.
 */
public final class RedCroc extends Croc {

    /**
     * Dirección de movimiento: 1 = baja, -1 = sube.
     */
    private Integer dir = 1;

    /**
     * Velocidad en píxeles por actualización (mínimo 1).
     */
    private final Integer speed;

    /**
     * Crea un cocodrilo rojo.
     *
     * @param vine      liana a la que está asociado.
     * @param initialY  posición vertical inicial.
     * @param speed     velocidad de movimiento (se fuerza a mínimo 1).
     */
    public RedCroc(final Vine vine,
                   final Integer initialY,
                   final Integer speed) {
        super(vine, initialY);
        if (speed == null) {
            throw new IllegalArgumentException("speed must not be null");
        }
        this.speed = Math.max(1, speed);
    }

    @Override
    public void update() {
        if (!alive) {
            return;
        }

        int iy = y;
        final int s = speed;
        final int direction = dir;

        iy += direction * s;

        final int top = vine.getYTop();
        final int bottom = vine.getYBottom();

        if (iy >= bottom) {
            iy = bottom;
            dir = -1;
        } else if (iy <= top) {
            iy = top;
            dir = 1;
        }

        y = iy;
    }
}