import java.util.ArrayList;
import java.util.List;

/**
 * Configuración estática del nivel.
 * <p>
 * Esta clase concentra las constantes de pantalla y las posiciones fijas
 * del mapa (plataformas, lianas, spawn del jugador, meta y zona de abismo),
 * copiadas directamente de la lógica existente en C.
 */
public final class GameConfig {

    private GameConfig() {
        // Evita instanciación
    }

    // ===================== PANTALLA =====================

    /** Ancho de la ventana en píxeles (SCREEN_WIDTH en C). */
    public static final Integer SCREEN_WIDTH  = 800;

    /** Alto de la ventana en píxeles (SCREEN_HEIGHT en C). */
    public static final Integer SCREEN_HEIGHT = 600;

    /** FPS objetivo (FPS en C). */
    public static final Integer FPS = 60;

    // ===================== "GRID" HEREDADO =====================

    /** Número de filas lógicas de la matriz en C (MATRIZ_FILAS). */
    public static final Integer GRID_ROWS = 30;

    /** Número de columnas lógicas de la matriz en C (MATRIZ_COLUMNAS). */
    public static final Integer GRID_COLS = 40;

    /** Tamaño de celda usado en C (TAMANIO_CELDA), en píxeles. */
    public static final Integer CELL_SIZE = 20;

    // ===================== TAMAÑOS =====================

    /** Tamaño del sprite/hitbox del jugador (JUGADOR_SIZE en C). */
    public static final Integer PLAYER_SIZE = 20;

    /** Tamaño del sprite del cocodrilo (COCODRILO_SIZE en C). */
    public static final Integer CROCODILE_SIZE = 18;

    /** Tamaño del sprite de la fruta (FRUTA_SIZE en C). */
    public static final Integer FRUIT_SIZE = 15;

    /** Ancho de la liana (LIANA_WIDTH en C), en píxeles. */
    public static final Integer VINE_WIDTH = 5;

    /** Alto de las plataformas (PLATAFORMA_HEIGHT en C), en píxeles. */
    public static final Integer PLATFORM_HEIGHT = 20;

    /** Alto aproximado del agua (AGUA_HEIGHT en C), en píxeles. */
    public static final Integer WATER_HEIGHT = 80;

    // ===================== MAPA ESTÁTICO =====================

    /**
     * Crea las plataformas (islas) del nivel con las mismas posiciones
     * definidas en {@code inicializar_juego} de Interfaz/game.c.
     *
     * @return lista de plataformas en coordenadas de píxel.
     */
    public static List<Platform> createPlatforms() {
        final List<Platform> platforms = new ArrayList<>();

        // NIVEL INFERIOR (5 islas) - y > 450
        // estado->islas[0] = (Isla){0, 550, 200, ...};
        platforms.add(new Platform(0,   550, 200, PLATFORM_HEIGHT)); // idx 0

        // estado->islas[2] = (Isla){420, 520, 80, ...};
        platforms.add(new Platform(420, 520, 80,  PLATFORM_HEIGHT)); // idx 1

        // estado->islas[3] = (Isla){550, 500, 80, ...};
        platforms.add(new Platform(550, 500, 80,  PLATFORM_HEIGHT)); // idx 2

        // estado->islas[4] = (Isla){300, 500, 90, ...};
        platforms.add(new Platform(300, 500, 90,  PLATFORM_HEIGHT)); // idx 3

        // estado->islas[5] = (Isla){670, 460, 100, ...};
        platforms.add(new Platform(670, 460, 100, PLATFORM_HEIGHT)); // idx 4

        // NIVEL MEDIO (3 islas)
        // estado->islas[1] = (Isla){130, 220, 100, ...};
        platforms.add(new Platform(130, 220, 100, PLATFORM_HEIGHT)); // idx 5

        // estado->islas[7] = (Isla){130, 340, 150, ...};
        platforms.add(new Platform(130, 340, 150, PLATFORM_HEIGHT)); // idx 6

        // estado->islas[9] = (Isla){600, 280, 180, ...};
        platforms.add(new Platform(600, 280, 180, PLATFORM_HEIGHT)); // idx 7

        // NIVEL SUPERIOR (2 islas)
        // estado->islas[6] = (Isla){460, 80, 200, ...};
        platforms.add(new Platform(460, 80, 200, PLATFORM_HEIGHT));  // idx 8

        // estado->islas[8] = (Isla){0, 60, 460, ...};
        platforms.add(new Platform(0,   60, 460, PLATFORM_HEIGHT));  // idx 9

        return platforms;
    }

    /**
     * Crea las lianas del nivel con las mismas posiciones que en C.
     *
     * @return lista de lianas en coordenadas de píxel.
     */
    public static List<Vine> createVines() {
        final List<Vine> vines = new ArrayList<>();

        // estado->lianas[0] = (Liana){40, 80, 480};
        vines.add(new Vine(0, 40, 80, 480, VINE_WIDTH / 2));

        // estado->lianas[1] = (Liana){110, 80, 470};
        vines.add(new Vine(1, 110, 80, 470, VINE_WIDTH / 2));

        // estado->lianas[2] = (Liana){200, 220, 490};
        vines.add(new Vine(2, 200, 220, 490, VINE_WIDTH / 2));

        // estado->lianas[3] = (Liana){320, 80, 410};
        vines.add(new Vine(3, 320, 80, 410, VINE_WIDTH / 2));

        // estado->lianas[4] = (Liana){450, 80, 300};
        vines.add(new Vine(4, 450, 80, 300, VINE_WIDTH / 2));

        // estado->lianas[5] = (Liana){520, 120, 400};
        vines.add(new Vine(5, 520, 120, 400, VINE_WIDTH / 2));

        // estado->lianas[6] = (Liana){580, 130, 350};
        vines.add(new Vine(6, 580, 130, 350, VINE_WIDTH / 2));

        // estado->lianas[7] = (Liana){680, 50, 400};
        vines.add(new Vine(7, 680, 50, 400, VINE_WIDTH / 2));

        // estado->lianas[8] = (Liana){750, 50, 400};
        vines.add(new Vine(8, 750, 50, 400, VINE_WIDTH / 2));

        return vines;
    }

    /**
     * Calcula la posición inicial del jugador usando la isla 0, igual que en C.
     *
     * @param platforms lista de plataformas devuelta por {@link #createPlatforms()}.
     * @return posición inicial del jugador en píxeles.
     */
    public static Point2i createPlayerSpawn(final List<Platform> platforms) {
        if (platforms == null || platforms.isEmpty()) {
            throw new IllegalArgumentException("Platforms must not be null or empty");
        }
        // En C: jugador.x = islas[0].x + 50; jugador.y = islas[0].y - JUGADOR_HITBOX;
        final Platform base = platforms.get(0);
        final int x = base.getX() + 50;
        final int y = base.getY() - PLAYER_SIZE / 2; // aproximación al JUGADOR_HITBOX
        return new Point2i(x, y);
    }

    /**
     * Posición de la meta (padre), calculada como en C:
     * <pre>
     * x = (MATRIZ_COLUMNAS / 2) * TAMANIO_CELDA + TAMANIO_CELDA / 2;
     * y = 2 * TAMANIO_CELDA + TAMANIO_CELDA / 2;
     * </pre>
     *
     * @return posición de la meta en píxeles.
     */
    public static Point2i createGoalPosition() {
        final int x = (GRID_COLS / 2) * CELL_SIZE + CELL_SIZE / 2;
        final int y = 2 * CELL_SIZE + CELL_SIZE / 2;
        return new Point2i(x, y);
    }

    /**
     * Zona de abismo/agua en la parte inferior de la pantalla.
     * <p>
     * Usa {@link #WATER_HEIGHT}, que viene de {@code AGUA_HEIGHT} en C.
     *
     * @return rectángulo que representa el área mortal de agua.
     */
    public static Rect getAbyssZone() {
        final int height = WATER_HEIGHT;
        final int y = SCREEN_HEIGHT - height;
        return new Rect(0, y, SCREEN_WIDTH, height);
    }
}
