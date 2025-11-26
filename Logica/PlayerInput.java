/**
 * Estado de entrada del jugador en un frame.
 * <p>
 * Normalmente se construye a partir de la información enviada por el cliente C.
 */
public final class PlayerInput {

    private final boolean left;
    private final boolean right;
    private final boolean up;
    private final boolean down;
    private final boolean jump;

    /**
     * Crea un estado de entrada.
     *
     * @param left  {@code true} si está presionada la tecla de moverse a la izquierda.
     * @param right {@code true} si está presionada la tecla de moverse a la derecha.
     * @param up    {@code true} si está presionada la tecla de moverse hacia arriba.
     * @param down  {@code true} si está presionada la tecla de moverse hacia abajo.
     * @param jump  {@code true} si está presionada la tecla de salto.
     */
    public PlayerInput(final boolean left,
                       final boolean right,
                       final boolean up,
                       final boolean down,
                       final boolean jump) {
        this.left = left;
        this.right = right;
        this.up = up;
        this.down = down;
        this.jump = jump;
    }

    public boolean isLeft() {
        return left;
    }

    public boolean isRight() {
        return right;
    }

    public boolean isUp() {
        return up;
    }

    public boolean isDown() {
        return down;
    }

    public boolean isJump() {
        return jump;
    }
}
