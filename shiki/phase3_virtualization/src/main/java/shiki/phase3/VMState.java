package shiki.phase3;

/**
 * Virtual Machine state enumeration.
 * 
 * Like JVM lifecycle states:
 * - STOPPED: Not running (terminated)
 * - BOOTING: Starting up (class loading, verification)
 * - RUNNING: Executing bytecode (normal operation)
 * - MIGRATING: Moving between hosts (live migration)
 * - SUSPENDED: Paused state (checkpointed)
 * - ESCAPED: Broke free from hypervisor (VM escape vulnerability)
 */
public enum VMState {
    STOPPED,
    BOOTING,
    RUNNING,
    MIGRATING,
    SUSPENDED,
    ESCAPED
}
