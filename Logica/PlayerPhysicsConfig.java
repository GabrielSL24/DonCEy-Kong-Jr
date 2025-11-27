/**
 * Parámetros de física del jugador.
 * <p>
 * Los valores numéricos se basan en config.h de la versión en C.
 */
public final class PlayerPhysicsConfig {

    private PlayerPhysicsConfig() {
        // Evita instanciación
    }

    // ---------------- FÍSICA GENERAL ----------------

    /** Gravedad aplicada por frame al jugador (GRAVEDAD en C). */
    public static final Float GRAVITY = 0.6f;

    /** Velocidad inicial vertical del salto (FUERZA_SALTO en C, negativa = hacia arriba). */
    public static final Float JUMP_FORCE = -10.0f;

    /** Velocidad horizontal máxima en el aire (VELOCIDAD_HORIZONTAL_AIRE en C). */
    public static final Float HORIZONTAL_SPEED_AIR = 5.5f;

    /** Velocidad horizontal en el suelo (VELOCIDAD_HORIZONTAL_SUELO en C). */
    public static final Float HORIZONTAL_SPEED_GROUND = 3.5f;

    /** Factor de frenado en el aire cuando no hay input (FRENADO_AIRE en C). */
    public static final Float AIR_FRICTION = 0.7f;

    /** Velocidad máxima de caída (VELOCIDAD_MAXIMA_CAIDA en C). */
    public static final Float MAX_FALL_SPEED = 8.0f;

    /** Incremento de velocidad horizontal al acelerar en el aire (0.5 en C). */
    public static final Float AIR_ACCELERATION = 0.3f;

    // ---------------- LIANAS ----------------

    /** Velocidad de trepado en liana (VELOCIDAD_TREPADO en C). */
    public static final Float CLIMB_SPEED = 3.0f;

    /** Distancia horizontal máxima para poder agarrar una liana (DISTANCIA_AGARRE_LIANA en C). */
    public static final Float VINE_GRAB_DISTANCE = 40.0f;

    /** Distancia máxima horizontal para cambiar de liana (DISTANCIA_SALTO en C). */
    public static final Float VINE_HOP_MAX_DISTANCE = 60.0f;

    /** Tolerancia vertical al cambiar de liana (no está en C, decisión de diseño). */
    public static final Float VINE_VERTICAL_TOLERANCE = 30.0f;
}
