/**
 * Configuración fija del mapa:
 * - Vines disponibles (id, x, yTop, yBottom)
 * - Zona de meta (GOAL) donde el jugador gana.
 */
public class GameConfig {

    // Ejemplo: define tus lianas reales aquí.
    public static Vine[] createVines() {
        return new Vine[] {
            new Vine(0, 4, 2, 13),   // Columna 4, de fila 2 a 13
            new Vine(1, 8, 2, 13),   // Columna 8, de fila 2 a 13  
            new Vine(2, 12, 2, 13),  // Columna 12, de fila 2 a 13
            new Vine(3, 16, 2, 13)   // Columna 16, de fila 2 a 13
        };
    }

    // Ejemplo de zona de victoria (puede ser una coordenada o rango).
    public static boolean isGoalPosition(int x, int y) {
        // Meta en la plataforma superior (fila 0-1)
        boolean esMeta = (x >= 9 && x <= 11 && y <= 1);
        System.out.println("🎯 GameConfig.isGoalPosition(" + x + "," + y + ") = " + esMeta);
        return esMeta;
    }

    // Ejemplo: abismo si y es mayor a cierto límite
    public static boolean isAbyss(int y) {
        // Abismo en las últimas filas (agua) - filas 13-14
        boolean esAbismo = (y >= 13);
        System.out.println("🌊 GameConfig.isAbyss(" + y + ") = " + esAbismo);
        return esAbismo;
    }
}
