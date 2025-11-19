public class Main {

    public static void main(String[] args) {

        // 1) Inicializar lógica del juego
        GameLogic game = new GameLogic();

        System.out.println("=== ESTADO INICIAL ===");
        printState(game);

        // 2) Admin: crear un cocodrilo rojo en la vine 0
        System.out.println("\n=== ADMIN: SPAWN RED CROC EN VINE 0 ===");
        boolean okRed = game.adminSpawnRedCroc(0, 10, 1);
        System.out.println("Spawn red croc en vine 0: " + okRed);
        printState(game);

        // 3) Admin: crear una fruta en (x=10, y=8) (por ejemplo sobre la misma vine)
        System.out.println("\n=== ADMIN: CREAR FRUTA EN (10, 8) ===");
        game.adminCreateFruit(10, 8, 300);
        printState(game);

        // 4) Tick 1: mover enemigos (croc) y simular que el jugador se mueve hacia la vine
        System.out.println("\n=== TICK 1: UPDATE ENEMIGOS + PLAYER -> (10, 15) ===");
        game.updateEnemies();
        game.updatePlayerFromClient(10, 15); // como si C hubiera enviado esta pos
        printState(game);

        // 5) Tick 2: mover enemigos + jugador sube (posible colisión con croc o fruta)
        System.out.println("\n=== TICK 2: UPDATE ENEMIGOS + PLAYER -> (10, 10) ===");
        game.updateEnemies();
        game.updatePlayerFromClient(10, 10);
        printState(game);

        // 6) Tick 3: jugador va a la fruta (10, 8)
        System.out.println("\n=== TICK 3: PLAYER -> (10, 8) (INTENTA TOMAR FRUTA) ===");
        game.updateEnemies();
        game.updatePlayerFromClient(10, 8);
        printState(game);

        // 7) Tick 4: jugador va hacia la meta (ej: 42,4 según GameConfig.isGoalPosition)
        System.out.println("\n=== TICK 4: PLAYER -> POSICIÓN GOAL (42, 4) ===");
        game.updateEnemies();
        game.updatePlayerFromClient(42, 4);
        printState(game);

        System.out.println("\n=== FIN DE PRUEBA ===");
    }

    private static void printState(GameLogic game) {
        Player p = game.getPlayer();

        System.out.println("Jugador:");
        System.out.println("  Pos     : (" + p.getX() + ", " + p.getY() + ")");
        System.out.println("  Vidas   : " + p.getLives());
        System.out.println("  Score   : " + p.getScore());
        System.out.println("  OnVine  : " + p.isOnVine());

        System.out.println("Nivel / Velocidad:");
        System.out.println("  Level   : " + game.getLevel());
        System.out.println("  SpeedMul: " + game.getSpeedMul());

        System.out.println("Cocodrilos:");
        int i = 0;
        for (Croc c : game.getCrocs()) {
            if (c != null && c.isAlive()) {
                System.out.println("  #" + i +
                        " tipo=" + c.getClass().getSimpleName() +
                        " vineId=" + c.getVine().getId() +
                        " pos=(" + c.getX() + "," + c.getY() + ")");
            }
            i++;
        }

        System.out.println("Frutas activas:");
        for (Fruit f : game.getFruits()) {
            if (f.isActive()) {
                System.out.println("  (" + f.getX() + "," + f.getY() + ") pts=" + f.getPoints());
            }
        }
    }
}
