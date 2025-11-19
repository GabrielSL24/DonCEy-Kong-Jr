public abstract class Croc {
    protected final Vine vine; // siempre asociado a una vine fija
    protected int y;
    protected boolean alive = true;

    protected Croc(Vine vine, int initialY) {
        this.vine = vine;
        this.y = initialY;
    }

    public Vine getVine() { return vine; }
    public int getX() { return vine.getX(); }
    public int getY() { return y; }
    public boolean isAlive() { return alive; }

    public abstract void update();

    protected void kill() {
        this.alive = false;
    }
}
