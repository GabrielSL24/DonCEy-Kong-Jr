public class Player {
    private int x;
    private int y;
    private int lives;
    private int score;
    private boolean onVine;

    public Player(int x, int y, int lives) {
        this.x = x;
        this.y = y;
        this.lives = lives;
        this.score = 0;
        this.onVine = false;
    }

    public int getX() { return x; }
    public int getY() { return y; }
    public int getLives() { return lives; }
    public int getScore() { return score; }
    public boolean isOnVine() { return onVine; }

    public void setPosition(int x, int y) {
        this.x = x;
        this.y = y;
    }

    public void setOnVine(boolean onVine) {
        this.onVine = onVine;
    }

    public void addScore(int points) {
        this.score += points;
    }

    public void loseLife() {
        if (lives > 0) lives--;
    }

    public void gainLife() {
        lives++;
    }
}
