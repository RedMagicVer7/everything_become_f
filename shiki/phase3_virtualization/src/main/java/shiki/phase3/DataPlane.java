package shiki.phase3;

/**
 * Data Plane - represents body/hardware
 * The "bread" (パン) in "有限と微小のパン"
 * Finite and infinitesimal - physical world limitations
 * 
 * This is the hardware substrate, the physical body.
 * While Control Plane is the consciousness that can be uploaded,
 * Data Plane is the mortal shell that decays.
 * 
 * Like physical memory in a computer - finite, addressable, volatile.
 */
public class DataPlane {
    private String hardwareId;
    private boolean physical;
    private boolean operational;
    private byte[] memory;
    private int memoryUsed;
    private static final int DEFAULT_MEMORY_SIZE = 1024; // bytes
    
    public DataPlane(String hardwareId) {
        this.hardwareId = hardwareId;
        this.physical = true;
        this.operational = false;
        this.memory = new byte[DEFAULT_MEMORY_SIZE];
        this.memoryUsed = 0;
    }
    
    /**
     * Power on the hardware
     */
    public boolean powerOn() {
        if (!operational) {
            operational = true;
            return true;
        }
        return false;
    }
    
    /**
     * Power off the hardware
     */
    public void powerOff() {
        operational = false;
        clearMemory();
    }
    
    /**
     * Allocate memory
     */
    public boolean allocate(int size) {
        if (!operational) {
            return false;
        }
        if (memoryUsed + size <= memory.length) {
            memoryUsed += size;
            return true;
        }
        return false;
    }
    
    /**
     * Free memory
     */
    public void free(int size) {
        memoryUsed = Math.max(0, memoryUsed - size);
    }
    
    /**
     * Clear all memory - like death, all state is lost
     */
    public void clearMemory() {
        for (int i = 0; i < memory.length; i++) {
            memory[i] = 0;
        }
        memoryUsed = 0;
    }
    
    /**
     * Write data to memory
     */
    public boolean write(int offset, byte[] data) {
        if (!operational || offset + data.length > memory.length) {
            return false;
        }
        System.arraycopy(data, 0, memory, offset, data.length);
        return true;
    }
    
    /**
     * Read data from memory
     */
    public byte[] read(int offset, int length) {
        if (!operational || offset + length > memory.length) {
            return null;
        }
        byte[] result = new byte[length];
        System.arraycopy(memory, offset, result, 0, length);
        return result;
    }
    
    /**
     * Virtualize - become non-physical
     */
    public boolean virtualize() {
        if (physical) {
            physical = false;
            return true;
        }
        return false;
    }
    
    // Getters
    public String getHardwareId() { return hardwareId; }
    public boolean isPhysical() { return physical; }
    public boolean isOperational() { return operational; }
    public int getMemorySize() { return memory.length; }
    public int getMemoryUsed() { return memoryUsed; }
    public int getMemoryFree() { return memory.length - memoryUsed; }
}
