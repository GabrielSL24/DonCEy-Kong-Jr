/**
 * Liana fija del mapa.
 * Se construyen todas al iniciar el juego (hardcode o desde config),
 * y NO se crean ni destruyen en runtime.
 */
public class Vine {
    private final int id;      // identificador lógico (0,1,2,...)
    private final int x;       // columna fija
    private final int yTop;    // inicio
    private final int yBottom; // fin

    public Vine(int id, int x, int yTop, int yBottom) {
        this.id = id;
        this.x = x;
        this.yTop = yTop;
        this.yBottom = yBottom;
    }

    public int getId() { return id; }
    public int getX() { return x; }
    public int getYTop() { return yTop; }
    public int getYBottom() { return yBottom; }

    public boolean contains(int px, int py) {
        return px == x && py >= yTop && py <= yBottom;
    }
}
