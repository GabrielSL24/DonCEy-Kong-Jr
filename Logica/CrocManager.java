import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

/**
 * Administra cocodrilos por vine fija.
 * Regla: máximo 1 cocodrilo vivo por vine.
 */
public class CrocManager {

    private final Map<Integer, Croc> crocsByVine = new HashMap<>();

    /** Devuelve true si se puede crear un nuevo cocodrilo en la vine. */
    public boolean canSpawnOn(Vine vine) {
        Croc existing = crocsByVine.get(vine.getId());
        return existing == null || !existing.isAlive();
    }

    /** Crea un cocodrilo rojo si la vine está libre. */
    public Croc spawnRed(Vine vine, int initialY, int speed) {
        if (!canSpawnOn(vine)) return null;
        Croc c = new RedCroc(vine, initialY, speed);
        crocsByVine.put(vine.getId(), c);
        return c;
    }

    /** Crea un cocodrilo azul si la vine está libre. */
    public Croc spawnBlue(Vine vine, int initialY, int speed) {
        if (!canSpawnOn(vine)) return null;
        Croc c = new BlueCroc(vine, initialY, speed);
        crocsByVine.put(vine.getId(), c);
        return c;
    }

    /** Actualiza el movimiento de todos los cocodrilos activos. */
    public void updateAll() {
        for (Croc c : crocsByVine.values()) {
            if (c != null && c.isAlive()) {
                c.update();
            }
        }
    }

    /** Devuelve todos los cocodrilos (vivos o muertos). */
    public Collection<Croc> getAllCrocs() {
        return crocsByVine.values();
    }

    /** Elimina cocodrilos muertos (opcional, para limpieza). */
    public void removeDead() {
        crocsByVine.entrySet().removeIf(e -> e.getValue() == null || !e.getValue().isAlive());
    }

    /** Limpia todo (reinicia nivel). */
    public void clear() {
        crocsByVine.clear();
    }
}
