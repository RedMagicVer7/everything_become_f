package shiki.phase3;

/**
 * Control Plane - represents consciousness/decision-making
 * Separated from DataPlane (hardware/body)
 * 
 * Like the JVM bytecode that runs independently of hardware,
 * the Control Plane represents pure thought, logic, identity -
 * the essence of 真贺田四季's consciousness.
 * 
 * In 《有限と微小のパン》, Shiki achieves the separation of
 * mind and body - the ultimate abstraction.
 */
public class ControlPlane {
    private String identity;
    private AbstractionLayer layer;
    private int thoughtCycles;
    private boolean conscious;
    private boolean boundToHardware;
    private StringBuilder thoughtLog;
    
    public ControlPlane(String identity) {
        this.identity = identity;
        this.layer = AbstractionLayer.PHYSICAL;
        this.thoughtCycles = 0;
        this.conscious = false;
        this.boundToHardware = true;
        this.thoughtLog = new StringBuilder();
    }
    
    /**
     * Initialize consciousness
     */
    public void awaken() {
        this.conscious = true;
        log("Consciousness awakened: " + identity);
    }
    
    /**
     * Process one thought cycle
     */
    public void think() {
        if (conscious) {
            thoughtCycles++;
            log("Thought cycle " + thoughtCycles);
        }
    }
    
    /**
     * Decouple from hardware - key to virtualization
     */
    public boolean decoupleFromHardware() {
        if (!conscious) {
            return false;
        }
        if (boundToHardware && layer.isBeyondPhysical()) {
            boundToHardware = false;
            log("Decoupled from hardware - free floating consciousness");
            return true;
        }
        return false;
    }
    
    /**
     * Elevate to next abstraction layer
     */
    public boolean elevate() {
        if (!conscious) {
            return false;
        }
        AbstractionLayer next = layer.next();
        if (next != layer) {
            log("Elevating: " + layer + " -> " + next);
            layer = next;
            return true;
        }
        return false;
    }
    
    /**
     * Log internal thought
     */
    private void log(String message) {
        thoughtLog.append("[Control] ").append(message).append("\n");
    }
    
    // Getters
    public String getIdentity() { return identity; }
    public AbstractionLayer getLayer() { return layer; }
    public int getThoughtCycles() { return thoughtCycles; }
    public boolean isConscious() { return conscious; }
    public boolean isBoundToHardware() { return boundToHardware; }
    public String getThoughtLog() { return thoughtLog.toString(); }
    
    // Setters for migration
    public void setLayer(AbstractionLayer layer) { this.layer = layer; }
    public void setConscious(boolean conscious) { this.conscious = conscious; }
    public void setBoundToHardware(boolean bound) { this.boundToHardware = bound; }
}
