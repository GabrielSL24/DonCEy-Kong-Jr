/**
 * Estados posibles del jugador desde el punto de vista de la física.
 */
public enum PlayerState {
    /**
     * El jugador está sobre una plataforma (en el suelo).
     */
    GROUND,

    /**
     * El jugador está ascendiendo tras iniciar un salto.
     */
    JUMPING,

    /**
     * El jugador está cayendo (por gravedad) y no está sobre una plataforma.
     */
    FALLING,

    /**
     * El jugador está agarrado de una liana.
     * <p>
     * La lógica asociada a este estado se implementará en un paso posterior.
     */
    ON_VINE
}
