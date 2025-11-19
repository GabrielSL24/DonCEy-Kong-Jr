public class BlueCroc extends Croc {

    private final int speed;

    public BlueCroc(Vine vine, int initialY, int speed) {
        super(vine, initialY);
        this.speed = Math.max(1, speed);
    }

    @Override
    public void update() {
        if (!alive) return;

        y += speed;

        // Si se salió de la vine, cae y muere
        if (y > vine.getYBottom()) {
            kill();
        }
    }
}
