public class Fruit {
    private final int x;
    private final int y;
    private final int points;
    private boolean active = true;

    public Fruit(int x, int y, int points) {
        this.x = x;
        this.y = y;
        this.points = points;
    }

    public int getX() { return x; }
    public int getY() { return y; }
    public int getPoints() { return points; }
    public boolean isActive() { return active; }

    public void collect() {
        active = false;
    }
}
