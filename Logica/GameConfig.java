/**
 * Configuración fija del mapa:
 * - Vines disponibles (id, x, yTop, yBottom)
 * - Zona de meta (GOAL) donde el jugador gana.
 */
public class GameConfig {

    // Ejemplo: define tus lianas reales aquí.
    public static Vine[] createVines() {
            return new Vine[] {
            new Vine(0, 2, 4, 24),
            new Vine(1, 5, 4, 23),
            new Vine(2, 10, 11, 24),
            new Vine(3, 16, 4, 20),
            new Vine(4, 22, 4, 15),
            new Vine(5, 26, 6, 20),
            new Vine(6, 29, 6, 17),
            new Vine(7, 34, 2, 20),
            new Vine(8, 37, 2, 20)
        };
    }

    // Ejemplo de zona de victoria (puede ser una coordenada o rango).
    public static boolean isGoalPosition(int x, int y) {
        // Meta en la plataforma superior
        boolean esMeta = (x >= 0 && x <= 23 && y <= 4);
        System.out.println("🎯 GameConfig.isGoalPosition(" + x + "," + y + ") = " + esMeta);
        return esMeta;
    }

    // Ejemplo: abismo si y es mayor a cierto límite
    public static boolean isAbyss(int y) {
        // Abismo en las últimas filas (agua)
        boolean esAbismo = (y >= 26);
        System.out.println("🌊 GameConfig.isAbyss(" + y + ") = " + esAbismo);
        return esAbismo;
    }
}
