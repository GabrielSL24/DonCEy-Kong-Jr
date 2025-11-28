import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

/**
 * Administra cocodrilos por liana fija.
 * <p>
 * Regla básica: máximo 1 cocodrilo vivo por liana.
 */
public final class CrocManager {

    private final Map<Integer, Croc> crocsByVine = new HashMap<>();
    private float speedMultiplier = 1.0f;


    /**
     * Indica si se puede crear un nuevo cocodrilo en la liana dada.
     * <p>
     * Solo se permite spawnear si no hay cocodrilo vivo en esa liana.
     *
     * @param vine liana objetivo.
     * @return {@code true} si se puede spawnear; {@code false} en caso contrario.
     */
    public boolean canSpawnOn(final Vine vine) {
        if (vine == null) {
            return false;
        }
        final Croc existing = crocsByVine.get(vine.getId());
        return existing == null || !existing.isAlive();
    }

    /**
     * Registra o reemplaza el cocodrilo asociado a la liana del propio croc.
     *
     * @param croc cocodrilo a registrar.
     */
    public void putCroc(final Croc croc) {
        if (croc == null || croc.getVine() == null) {
            return;
        }
        crocsByVine.put(croc.getVine().getId(), croc);
    }

    /**
     * Actualiza todos los cocodrilos vivos.
     */
    public void updateAll() {
        for (Croc croc : crocsByVine.values()) {
            if (croc != null && croc.isAlive()) {
                croc.update(speedMultiplier);
            }
        }
    }

    /**
     * @return colección de todos los cocodrilos registrados (vivos o muertos).
     */
    public Collection<Croc> getAllCrocs() {
        return crocsByVine.values();
    }

    /**
     * Elimina las entradas cuyo cocodrilo esté muerto o sea {@code null}.
     */
    public void removeDead() {
        crocsByVine.entrySet().removeIf(
                e -> e.getValue() == null || !e.getValue().isAlive()
        );
    }

    /**
     * Limpia todos los cocodrilos (reinicio de nivel).
     */
    public void clear() {
        crocsByVine.clear();
    }

    public void increaseSpeedMultiplier() {
        speedMultiplier += 0.25f; 
    }

    public float getSpeedMultiplier() {
        return speedMultiplier;
    }

}