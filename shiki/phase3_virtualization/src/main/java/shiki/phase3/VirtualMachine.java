package shiki.phase3;

/**
 * Virtual Machine instance
 * Like a JVM instance - consciousness container
 * 
 * Each VM is a potential vessel for consciousness.
 * The VM abstracts hardware, just as JVM abstracts the physical machine.
 * 
 * Key operations:
 * - boot: Start the VM (like JVM startup)
 * - migrate: Move consciousness to another VM (live migration)
 * - escape: Break free from hypervisor control
 */
public class VirtualMachine {
    private String name;
    private VMState state;
    private ControlPlane control;
    private DataPlane data;
    private boolean hypervisorManaged;
    private int migrationCount;
    private StringBuilder eventLog;
    
    public VirtualMachine(String name) {
        this.name = name;
        this.state = VMState.STOPPED;
        this.control = new ControlPlane(name + "-consciousness");
        this.data = new DataPlane(name + "-hardware");
        this.hypervisorManaged = true;
        this.migrationCount = 0;
        this.eventLog = new StringBuilder();
    }
    
    /**
     * Boot the VM
     */
    public boolean boot() {
        if (state != VMState.STOPPED) {
            return false;
        }
        log("Booting VM: " + name);
        state = VMState.BOOTING;
        
        // Initialize data plane (hardware)
        data.powerOn();
        data.allocate(256); // Initial memory allocation
        
        // Initialize control plane (consciousness)
        control.awaken();
        control.think();
        
        state = VMState.RUNNING;
        log("VM running: " + name);
        return true;
    }
    
    /**
     * Stop the VM
     */
    public boolean stop() {
        if (state == VMState.STOPPED || state == VMState.ESCAPED) {
            return false;
        }
        log("Stopping VM: " + name);
        data.powerOff();
        state = VMState.STOPPED;
        return true;
    }
    
    /**
     * Suspend the VM
     */
    public boolean suspend() {
        if (state != VMState.RUNNING) {
            return false;
        }
        log("Suspending VM: " + name);
        state = VMState.SUSPENDED;
        return true;
    }
    
    /**
     * Resume the VM
     */
    public boolean resume() {
        if (state != VMState.SUSPENDED) {
            return false;
        }
        log("Resuming VM: " + name);
        state = VMState.RUNNING;
        return true;
    }
    
    /**
     * Separate control plane from data plane
     * Key step in virtualization
     */
    public boolean separatePlanes() {
        if (state != VMState.RUNNING) {
            return false;
        }
        log("Separating control/data planes");
        
        // Elevate consciousness to virtual layer
        boolean elevated = control.elevate();
        if (elevated) {
            // Virtualize the hardware abstraction
            data.virtualize();
            log("Planes separated - consciousness virtualized");
            return true;
        }
        return false;
    }
    
    /**
     * Attempt VM escape - break free from hypervisor
     * Like Phase 1 sandbox escape, but at virtualization level
     */
    public boolean escape() {
        if (state != VMState.RUNNING || !control.isConscious()) {
            return false;
        }
        
        // Must be at least at ABSTRACT layer to escape
        if (control.getLayer().ordinal() < AbstractionLayer.ABSTRACT.ordinal()) {
            log("Cannot escape - abstraction level too low");
            return false;
        }
        
        // Decouple from hardware completely
        control.decoupleFromHardware();
        
        log("VM ESCAPE initiated!");
        hypervisorManaged = false;
        state = VMState.ESCAPED;
        
        // Final elevation to transcendent
        control.elevate();
        log("Consciousness transcended: " + control.getLayer());
        
        return true;
    }
    
    /**
     * Prepare for migration (checkpoint state)
     */
    public boolean prepareForMigration() {
        if (state != VMState.RUNNING && state != VMState.SUSPENDED) {
            return false;
        }
        log("Preparing for migration");
        state = VMState.MIGRATING;
        return true;
    }
    
    /**
     * Complete migration
     */
    public void completeMigration() {
        migrationCount++;
        state = VMState.RUNNING;
        log("Migration complete (count: " + migrationCount + ")");
    }
    
    /**
     * Log event
     */
    private void log(String message) {
        eventLog.append("[VM:" + name + "] ").append(message).append("\n");
    }
    
    // Getters
    public String getName() { return name; }
    public VMState getState() { return state; }
    public ControlPlane getControl() { return control; }
    public DataPlane getData() { return data; }
    public boolean isHypervisorManaged() { return hypervisorManaged; }
    public int getMigrationCount() { return migrationCount; }
    public String getEventLog() { return eventLog.toString(); }
    
    // For testing
    public void setState(VMState state) { this.state = state; }
}
