/**
 * Cocodrilo azul: desciende por la liana y muere cuando la abandona.
 */
public final class BlueCroc extends Croc {

    /**
     * Velocidad en píxeles por actualización (mínimo 1).
     */
    private final Integer speed;

    /**
     * Crea un cocodrilo azul.
     *
     * @param vine      liana a la que está asociado.
     * @param initialY  posición vertical inicial.
     * @param speed     velocidad de movimiento (se fuerza a mínimo 1).
     */
    public BlueCroc(final Vine vine,
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
        iy += s;

        final int bottom = vine.getYBottom();
        if (iy > bottom) {
            // Se salió de la vine, cae y muere
            kill();
        } else {
            y = iy;
        }
    }
}
