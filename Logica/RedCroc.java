public class RedCroc extends Croc {

    private int dir = 1; // 1 = baja, -1 = sube
    private final int speed;

    public RedCroc(Vine vine, int initialY, int speed) {
        super(vine, initialY);
        this.speed = Math.max(1, speed);
    }

    @Override
    public void update() {
        if (!alive) return;

        y += dir * speed;

        if (y >= vine.getYBottom()) {
            y = vine.getYBottom();
            dir = -1;
        } else if (y <= vine.getYTop()) {
            y = vine.getYTop();
            dir = 1;
        }
    }
}
