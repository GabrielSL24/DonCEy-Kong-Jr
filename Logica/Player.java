import java.util.List;

/**
 * Representa al jugador del juego, incluyendo su estado de física
 * (posición, velocidad y estado de movimiento).
 */
public final class Player {

    private Integer xCenter;      // posición X del centro del jugador, en píxeles
    private Integer yCenter;      // posición Y del centro del jugador, en píxeles

    private final Integer width;  // ancho del jugador en píxeles
    private final Integer height; // alto del jugador en píxeles

    private Float velocityX;      // velocidad horizontal (píxeles por frame)
    private Float velocityY;      // velocidad vertical (píxeles por frame)

    private Integer lives;
    private Integer score;

    private PlayerState state;

    /** Id de la liana a la que está agarrado, o {@code null} si no hay liana. */
    private Integer attachedVineId;

    /**
     * Crea un nuevo jugador.
     *
     * @param spawnX posición inicial X (centro) en píxeles.
     * @param spawnY posición inicial Y (centro) en píxeles.
     * @param lives  número de vidas iniciales.
     * @param size   tamaño del jugador (ancho = alto), en píxeles.
     */
    public Player(final Integer spawnX,
                  final Integer spawnY,
                  final Integer lives,
                  final Integer size) {
        if (spawnX == null || spawnY == null || lives == null || size == null) {
            throw new IllegalArgumentException("Player parameters must not be null");
        }
        this.xCenter = spawnX;
        this.yCenter = spawnY;
        this.lives = lives;
        this.score = 0;
        this.width = size;
        this.height = size;
        this.velocityX = 0.0f;
        this.velocityY = 0.0f;
        this.state = PlayerState.GROUND;
        this.attachedVineId = null;
    }

    // ==================== GETTERS BÁSICOS ====================

    public Integer getX() {
        return xCenter;
    }

    public Integer getY() {
        return yCenter;
    }

    public Integer getLives() {
        return lives;
    }

    public Integer getScore() {
        return score;
    }

    public PlayerState getState() {
        return state;
    }

    /**
     * @return id de la liana a la que está agarrado, o {@code null}.
     */
    public Integer getAttachedVineId() {
        return attachedVineId;
    }

    /**
     * @return rectángulo de colisión aproximado del jugador.
     */
    public Rect getBounds() {
        final int halfW = width / 2;
        final int halfH = height / 2;
        final int x = xCenter - halfW;
        final int y = yCenter - halfH;
        return new Rect(x, y, width, height);
    }

    // ==================== PUNTUACIÓN / VIDAS ====================

    public void addScore(final Integer points) {
        if (points != null && points > 0) {
            this.score += points;
        }
    }

    public void loseLife() {
        if (lives > 0) {
            lives -= 1;
        }
    }

    public void gainLife() {
        lives += 1;
    }


    /**
     * Entrada: centerX, centerY (nueva posición del centro).
     * Restricción: Ningún parámetro debe ser null.
     * Salida: Reposiciona al jugador y reinicia su estado físico.
     */
    public void resetTo(final Integer centerX, final Integer centerY) {
        if (centerX == null || centerY == null) {
            return;
        }
        this.xCenter = centerX;
        this.yCenter = centerY;
        this.velocityX = 0.0f;
        this.velocityY = 0.0f;
        this.state = PlayerState.GROUND;
        this.attachedVineId = null;
    }

    // ==================== BUCLE DE ACTUALIZACIÓN ====================

    /**
     * Actualiza el jugador un frame: aplica input, gravedad y colisiones
     * con las plataformas. Si está en una liana, delega a la lógica ON_VINE.
     *
     * @param input     estado de entrada del usuario.
     * @param platforms lista de plataformas del nivel.
     * @param vines     lista de lianas del nivel.
     */
    public void update(final PlayerInput input,
                       final List<Platform> platforms,
                       final List<Vine> vines) {
        if (input == null || platforms == null || vines == null) {
            return;
        }

        if (state == PlayerState.ON_VINE) {
            // Lógica especial cuando está agarrado de una liana
            updateOnVine(input, vines);
            return;
        }

        // 1) Movimiento horizontal según estado y entrada
        aplicarMovimientoHorizontal(input);

        // 2) Salto (solo si está en el suelo)
        if (input.isJump() && state == PlayerState.GROUND) {
            iniciarSalto();
        }

        // 3) Gravedad
        aplicarGravedad();

        // 4) Mover en Y y resolver colisiones verticales
        moverYConColisiones(platforms);

        // 5) Mover en X y resolver colisiones horizontales
        moverXConColisiones(platforms);

        // 6) Intentar agarrarse a una liana (solo en el aire y si se mantiene ARRIBA)
        if ((state == PlayerState.JUMPING || state == PlayerState.FALLING) && input.isUp()) {
            tryGrabVine(vines);
        }
    }

    // ==================== LÓGICA SUELO / AIRE ====================

    private void aplicarMovimientoHorizontal(final PlayerInput input) {
        float newVx = velocityX;

        if (state == PlayerState.GROUND) {
            // En el suelo, velocidad fija según dirección
            if (input.isLeft() && !input.isRight()) {
                newVx = -PlayerPhysicsConfig.HORIZONTAL_SPEED_GROUND;
            } else if (input.isRight() && !input.isLeft()) {
                newVx = PlayerPhysicsConfig.HORIZONTAL_SPEED_GROUND;
            } else {
                newVx = 0.0f;
            }
        } else {
            // En el aire: acelerar hasta un máximo y aplicar frenado si no hay input
            if (input.isLeft() && !input.isRight()) {
                newVx -= PlayerPhysicsConfig.AIR_ACCELERATION;
                // Limitar velocidad máxima
                if (newVx < -PlayerPhysicsConfig.HORIZONTAL_SPEED_AIR) {
                    newVx = -PlayerPhysicsConfig.HORIZONTAL_SPEED_AIR;
                }
            } else if (input.isRight() && !input.isLeft()) {
                newVx += PlayerPhysicsConfig.AIR_ACCELERATION;
                // Limitar velocidad máxima
                if (newVx > PlayerPhysicsConfig.HORIZONTAL_SPEED_AIR) {
                    newVx = PlayerPhysicsConfig.HORIZONTAL_SPEED_AIR;
                }
            } else {
                // Sin input horizontal: aplicar frenado en aire
                newVx *= PlayerPhysicsConfig.AIR_FRICTION;
                if (Math.abs(newVx) < 0.1f) {
                    newVx = 0.0f;
                }
            }

            //final float maxAir = PlayerPhysicsConfig.HORIZONTAL_SPEED_AIR;
            //if (newVx > maxAir) {
            //    newVx = maxAir;
            //} else if (newVx < -maxAir) {
            //    newVx = -maxAir;
            //}
        }

        velocityX = newVx;
    }

    private void iniciarSalto() {
        state = PlayerState.JUMPING;
        velocityY = PlayerPhysicsConfig.JUMP_FORCE;
    }

    private void aplicarGravedad() {
        float vy = velocityY + PlayerPhysicsConfig.GRAVITY;
        if (vy > PlayerPhysicsConfig.MAX_FALL_SPEED) {
            vy = PlayerPhysicsConfig.MAX_FALL_SPEED;
        }
        velocityY = vy;

        if (state == PlayerState.JUMPING && velocityY > 0.0f) {
            state = PlayerState.FALLING;
        }
        if (state == PlayerState.GROUND && velocityY > 0.0f) {
            state = PlayerState.FALLING;
        }
    }

    private void moverYConColisiones(final List<Platform> platforms) {
        final float vy = velocityY;
        if (vy == 0.0f) {
            return;
        }

        final Rect before = getBounds();
        final int newYCenter = yCenter + Math.round(vy);
        final Rect after = boundsAt(xCenter, newYCenter);

        boolean grounded = false;

        for (Platform platform : platforms) {
            final Rect pRect = platform.getBounds();

            if (!after.intersects(pRect)) {
                continue;
            }

            final int beforeBottom = before.getY() + before.getHeight();
            final int platformTop = pRect.getY();

            final int beforeTop = before.getY();
            final int platformBottom = pRect.getY() + pRect.getHeight();

            if (vy > 0.0f && beforeBottom <= platformTop) {
                // Aterriza en la plataforma
                final int halfH = height / 2;
                yCenter = platformTop - halfH;
                velocityY = 0.0f;
                state = PlayerState.GROUND;
                grounded = true;
                return;
            }

            if (vy < 0.0f && beforeTop >= platformBottom) {
                // Golpea el techo de la plataforma
                final int halfH = height / 2;
                yCenter = platformBottom + halfH;
                velocityY = 0.0f;
                state = PlayerState.FALLING;
                return;
            }
        }

        if (!grounded) {
            yCenter = newYCenter;
            if (state == PlayerState.GROUND && vy > 0.0f) {
                state = PlayerState.FALLING;
            }
        }
    }

    private void moverXConColisiones(final List<Platform> platforms) {
        final float vx = velocityX;
        if (vx == 0.0f) {
            return;
        }

        final Rect before = getBounds();
        final int newXCenter = xCenter + Math.round(vx);
        final Rect after = boundsAt(newXCenter, yCenter);

        for (Platform platform : platforms) {
            final Rect pRect = platform.getBounds();

            if (!after.intersects(pRect)) {
                continue;
            }

            final int beforeRight = before.getX() + before.getWidth();
            final int beforeLeft = before.getX();
            final int platformLeft = pRect.getX();
            final int platformRight = pRect.getX() + pRect.getWidth();
            final int halfW = width / 2;

            if (vx > 0.0f && beforeRight <= platformLeft) {
                xCenter = platformLeft - halfW;
                velocityX = 0.0f;
                return;
            }

            if (vx < 0.0f && beforeLeft >= platformRight) {
                xCenter = platformRight + halfW;
                velocityX = 0.0f;
                return;
            }
        }

        xCenter = newXCenter;
    }

    private Rect boundsAt(final Integer centerX, final Integer centerY) {
        final int halfW = width / 2;
        final int halfH = height / 2;
        final int x = centerX - halfW;
        final int y = centerY - halfH;
        return new Rect(x, y, width, height);
    }

    // ==================== LIANAS ====================

    /**
     * Intenta agarrarse a una liana cercana cuando el jugador está en el aire
     * y se mantiene presionada la tecla de subir.
     *
     * @param vines lista de lianas disponibles.
     */
    private void tryGrabVine(final List<Vine> vines) {
        final int px = xCenter;
        final int py = yCenter;

        Vine best = null;
        float bestDist = PlayerPhysicsConfig.VINE_GRAB_DISTANCE;

        for (Vine v : vines) {
            final int vx = v.getX();
            final int vyTop = v.getYTop();
            final int vyBottom = v.getYBottom();

            // Verificar rango vertical: centro del jugador dentro del tramo de la liana
            if (py < vyTop || py > vyBottom) {
                continue;
            }

            final float dx = Math.abs(px - vx);
            if (dx <= bestDist) {
                bestDist = dx;
                best = v;
            }
        }

        if (best != null) {
            // Nos agarramos a la liana
            state = PlayerState.ON_VINE;
            attachedVineId = best.getId();
            velocityX = 0.0f;
            velocityY = 0.0f;
            // Nota: mantenemos la X actual, igual que la versión en C que no ajusta la posición automáticamente.
        }
    }

    /**
     * Lógica cuando el jugador está en el estado ON_VINE.
     *
     * @param input estado de entrada.
     * @param vines lista de lianas.
     */
    private void updateOnVine(final PlayerInput input, final List<Vine> vines) {
        final Vine vine = getAttachedVine(vines);
        if (vine == null) {
            // Si por alguna razón la liana desapareció, caemos.
            state = PlayerState.FALLING;
            attachedVineId = null;
            return;
        }

        // Mientras está en liana no hay movimiento horizontal "normal"
        velocityX = 0.0f;
        velocityY = 0.0f;

        // Movimiento vertical (trepar / descender)
        int newYCenter = yCenter;
        if (input.isUp()) {
            newYCenter -= Math.round(PlayerPhysicsConfig.CLIMB_SPEED);
        }
        if (input.isDown()) {
            newYCenter += Math.round(PlayerPhysicsConfig.CLIMB_SPEED);
        }

        // Limitar dentro del tramo de la liana
        final int halfH = height / 2;
        final int minCenterY = vine.getYTop() + halfH;
        final int maxCenterY = vine.getYBottom() - halfH;
        if (newYCenter < minCenterY) {
            newYCenter = minCenterY;
        }
        if (newYCenter > maxCenterY) {
            newYCenter = maxCenterY;
        }
        yCenter = newYCenter;

        // Salto desde la liana
        if (input.isJump()) {
            jumpFromVine(input);
            return;
        }

        // Cambio de liana izquierda/derecha
        if (input.isRight() && !input.isLeft()) {
            if (!changeVine(vines, +1.0f)) {
                // Si no hay liana en esa dirección, se suelta
                detachFromVineAndFall();
            }
        } else if (input.isLeft() && !input.isRight()) {
            if (!changeVine(vines, -1.0f)) {
                detachFromVineAndFall();
            }
        }
    }

    private Vine getAttachedVine(final List<Vine> vines) {
        if (attachedVineId == null) {
            return null;
        }
        for (Vine v : vines) {
            if (attachedVineId.equals(v.getId())) {
                return v;
            }
        }
        return null;
    }

    /**
     * Salta desde la liana, similar a saltar_desde_liana en C.
     *
     * @param input estado de entrada.
     */
    private void jumpFromVine(final PlayerInput input) {
        state = PlayerState.JUMPING;
        velocityY = PlayerPhysicsConfig.JUMP_FORCE * 0.7f;

        if (input.isRight() && !input.isLeft()) {
            velocityX = PlayerPhysicsConfig.HORIZONTAL_SPEED_AIR;
        } else if (input.isLeft() && !input.isRight()) {
            velocityX = -PlayerPhysicsConfig.HORIZONTAL_SPEED_AIR;
        } else {
            velocityX = 0.0f;
        }

        attachedVineId = null;
    }

    /**
     * Intenta cambiar a una liana vecina en la dirección indicada.
     *
     * @param vines     lista de lianas.
     * @param direction +1.0f para derecha, -1.0f para izquierda.
     * @return {@code true} si se cambió de liana, {@code false} si no hay liana válida.
     */
    private boolean changeVine(final List<Vine> vines, final float direction) {
        final Vine current = getAttachedVine(vines);
        if (current == null || direction == 0.0f) {
            return false;
        }

        final int px = xCenter;
        final int py = yCenter;

        Vine best = null;
        float bestDist = PlayerPhysicsConfig.VINE_HOP_MAX_DISTANCE;

        for (Vine v : vines) {
            if (v.getId().equals(current.getId())) {
                continue;
            }

            final int vx = v.getX();
            final float dx = vx - current.getX();

            if (direction > 0.0f && dx <= 0.0f) {
                continue; // buscamos solo a la derecha
            }
            if (direction < 0.0f && dx >= 0.0f) {
                continue; // solo a la izquierda
            }

            if (Math.abs(dx) > PlayerPhysicsConfig.VINE_HOP_MAX_DISTANCE) {
                continue; // demasiado lejos
            }

            final int vyTop = v.getYTop();
            final int vyBottom = v.getYBottom();
            final float tolerance = PlayerPhysicsConfig.VINE_VERTICAL_TOLERANCE;

            // el centro del jugador debe estar cerca del tramo vertical de la nueva liana
            if (py < vyTop - tolerance || py > vyBottom + tolerance) {
                continue;
            }

            final float absDx = Math.abs(dx);
            if (absDx < bestDist) {
                bestDist = absDx;
                best = v;
            }
        }

        if (best != null) {
            attachedVineId = best.getId();
            return true;
        }
        return false;
    }

    private void detachFromVineAndFall() {
        attachedVineId = null;
        state = PlayerState.FALLING;
        // la gravedad se aplicará en el siguiente frame
    }
}