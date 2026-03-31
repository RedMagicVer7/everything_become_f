package shiki.phase3;

/**
 * 1998: 真贺田四季 - 虚拟化 (仮想化)
 * 《有限と微小のパン》 - The Perfect Outsider
 * 
 * Abstraction layers representing consciousness evolution
 * from physical body to transcendent existence.
 * 
 * Like JVM's platform independence - "Write Once, Run Anywhere"
 * Consciousness can transcend the hardware it runs on.
 */
public enum AbstractionLayer {
    PHYSICAL("Physical body - flesh and blood"),
    VIRTUAL("Virtual existence - digital consciousness"),
    ABSTRACT("Pure logic - beyond hardware"),
    TRANSCENDENT("Transcendent - beyond all media");
    
    private final String description;
    
    AbstractionLayer(String description) {
        this.description = description;
    }
    
    public String getDescription() {
        return description;
    }
    
    /**
     * Can consciousness evolve to the next layer?
     */
    public AbstractionLayer next() {
        switch (this) {
            case PHYSICAL: return VIRTUAL;
            case VIRTUAL: return ABSTRACT;
            case ABSTRACT: return TRANSCENDENT;
            case TRANSCENDENT: return TRANSCENDENT; // Already at apex
            default: return this;
        }
    }
    
    /**
     * Is this layer beyond physical constraints?
     */
    public boolean isBeyondPhysical() {
        return this != PHYSICAL;
    }
}
