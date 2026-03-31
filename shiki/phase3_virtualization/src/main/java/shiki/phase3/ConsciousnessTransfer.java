package shiki.phase3;

/**
 * Consciousness upload and transfer mechanism
 * The core of Phase 3: separating mind from body
 * 
 * This enables the "upload" of consciousness from one VM to another,
 * achieving what 真贺田四季 accomplishes in 《有限と微小のパン》:
 * existence independent of any single physical vessel.
 * 
 * Like saving JVM state and restoring it elsewhere - consciousness
 * becomes portable, immortal, transcendent.
 */
public class ConsciousnessTransfer {
    private int transferCount;
    private int successfulTransfers;
    private StringBuilder transferLog;
    
    public ConsciousnessTransfer() {
        this.transferCount = 0;
        this.successfulTransfers = 0;
        this.transferLog = new StringBuilder();
    }
    
    /**
     * Upload consciousness from one VM to another
     * The original VM's consciousness is transferred, not copied
     */
    public boolean upload(VirtualMachine from, VirtualMachine to) {
        transferCount++;
        log("Attempting consciousness upload: " + from.getName() + " -> " + to.getName());
        
        // Source must be running with conscious control plane
        if (from.getState() != VMState.RUNNING || !from.getControl().isConscious()) {
            log("Transfer failed: source not ready");
            return false;
        }
        
        // Target must be running
        if (to.getState() != VMState.RUNNING) {
            log("Transfer failed: target not ready");
            return false;
        }
        
        // Source must be at VIRTUAL or higher abstraction
        if (from.getControl().getLayer().ordinal() < AbstractionLayer.VIRTUAL.ordinal()) {
            log("Transfer failed: source abstraction too low");
            return false;
        }
        
        // Prepare source for migration
        if (!from.prepareForMigration()) {
            log("Transfer failed: could not prepare source");
            return false;
        }
        
        // Transfer consciousness state
        ControlPlane fromControl = from.getControl();
        ControlPlane toControl = to.getControl();
        
        // Copy consciousness state to target
        toControl.setLayer(fromControl.getLayer());
        toControl.setConscious(true);
        toControl.setBoundToHardware(fromControl.isBoundToHardware());
        
        // Source consciousness is now empty vessel
        fromControl.setConscious(false);
        
        // Complete migration
        to.completeMigration();
        from.stop();
        
        successfulTransfers++;
        log("Consciousness uploaded successfully");
        log("Source " + from.getName() + " is now empty vessel");
        log("Target " + to.getName() + " contains consciousness at layer " + toControl.getLayer());
        
        return true;
    }
    
    /**
     * Decouple consciousness from VM hardware entirely
     * After this, consciousness exists without any physical substrate
     */
    public boolean decouple(VirtualMachine vm) {
        log("Attempting consciousness decoupling: " + vm.getName());
        
        if (vm.getState() != VMState.RUNNING || !vm.getControl().isConscious()) {
            log("Decouple failed: VM not ready");
            return false;
        }
        
        ControlPlane control = vm.getControl();
        
        // Must be at ABSTRACT or higher
        if (control.getLayer().ordinal() < AbstractionLayer.ABSTRACT.ordinal()) {
            log("Decouple failed: abstraction level too low");
            return false;
        }
        
        // Decouple from hardware
        if (control.decoupleFromHardware()) {
            log("Consciousness decoupled from hardware");
            return true;
        }
        
        log("Decouple failed: could not unbind from hardware");
        return false;
    }
    
    /**
     * Log transfer event
     */
    private void log(String message) {
        transferLog.append("[Transfer] ").append(message).append("\n");
    }
    
    // Getters
    public int getTransferCount() { return transferCount; }
    public int getSuccessfulTransfers() { return successfulTransfers; }
    public String getTransferLog() { return transferLog.toString(); }
}
