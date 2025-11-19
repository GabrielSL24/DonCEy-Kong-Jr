// Ejemplo de definición fija (puede ir en otra clase)
public class MapConfig {
    public static Vine[] createVines() {
        return new Vine[] {
            new Vine(0, 10, 5, 20),
            new Vine(1, 20, 8, 22),
            // etc: todas las vines del nivel
        };
    }
}
