
public class GameLogic {
    private float playerX = 200.0f;
    private float playerY = 300.0f;
    private String playerState = "STANDING";

    public void processInput(String inputType, String key) {
        System.out.println("Procesando input: " + inputType + " - " + key);
        
        if ("KEY_PRESSED".equals(inputType)) {
            switch (key) {
                case "LEFT":
                    playerX -= 5.0f;
                    playerState = "MOVING_LEFT";
                    break;
                case "RIGHT":
                    playerX += 5.0f;
                    playerState = "MOVING_RIGHT";
                    break;
                case "UP":
                    playerY -= 5.0f;
                    playerState = "CLIMBING";
                    break;
                case "DOWN":
                    playerY += 5.0f;
                    playerState = "FALLING";
                    break;
                case "JUMP":
                    playerY -= 10.0f;
                    playerState = "JUMPING";
                    break;
            }
        } else if ("KEY_RELEASED".equals(inputType)) {
            if ("LEFT".equals(key) || "RIGHT".equals(key)) {
                playerState = "STANDING";
            }
        }

        // Limites
        if (playerX < 0) playerX = 0;
        if (playerX > 800) playerX = 800;
        if (playerY < 0) playerY = 0;
        if (playerY > 600) playerY = 600;
        
        System.out.println("Nueva posicion: " + playerX + ", " + playerY);
    }

    // Solo getters para el estado - NO generación de JSON
    public float getPlayerX() { return playerX; }
    public float getPlayerY() { return playerY; }
    public String getPlayerState() { return playerState; }
}
