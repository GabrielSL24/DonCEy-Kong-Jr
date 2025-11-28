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
    public BlueCroc(final Vine vine) {
        super(vine, vine.getYTop());
        this.speed = 1;
    }

    @Override
    public void update() {
        update(1.0f);
    }

    @Override
    public void update(float speedMultiplier) {
        if (!alive) {
            return;
        }

        int iy = y;
        final int baseSpeed = speed;
        final int s = Math.max(1, Math.round(baseSpeed * speedMultiplier));
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
