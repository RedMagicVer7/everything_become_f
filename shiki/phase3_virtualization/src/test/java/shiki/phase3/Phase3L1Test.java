package shiki.phase3;

/**
 * Phase 3 L1 Unit Tests
 * Tests individual components in isolation
 * 
 * At least 10 unit tests covering:
 * - Engine initialization
 * - VM creation and state
 * - Control plane
 * - Data plane
 * - Abstraction layers
 * - Memory operations
 */
public class Phase3L1Test {
    private static int passed = 0;
    private static int failed = 0;
    
    private static void assertEqual(Object expected, Object actual, String name) {
        if (expected == null && actual == null) {
            System.out.println("  " + name + " ... PASS");
            passed++;
        } else if (expected != null && expected.equals(actual)) {
            System.out.println("  " + name + " ... PASS");
            passed++;
        } else {
            System.out.println("  " + name + " ... FAIL (expected " + expected + ", got " + actual + ")");
            failed++;
        }
    }
    
    private static void assertTrue(boolean condition, String name) {
        assertEqual(true, condition, name);
    }
    
    private static void assertFalse(boolean condition, String name) {
        assertEqual(false, condition, name);
    }
    
    private static void assertNotNull(Object obj, String name) {
        if (obj != null) {
            System.out.println("  " + name + " ... PASS");
            passed++;
        } else {
            System.out.println("  " + name + " ... FAIL (expected non-null, got null)");
            failed++;
        }
    }
    
    public static int run() {
        passed = 0;
        failed = 0;
        
        System.out.println("\nL1: Unit Tests");
        System.out.println("--------------");
        
        // Test 1: Engine initialization
        test_engine_init();
        
        // Test 2: VM creation
        test_vm_creation();
        
        // Test 3: VM boot
        test_vm_boot();
        
        // Test 4: Control plane initialization
        test_control_plane_init();
        
        // Test 5: Data plane initialization
        test_data_plane_init();
        
        // Test 6: Abstraction layer default
        test_abstraction_layer_default();
        
        // Test 7: VM state transitions
        test_vm_state_transitions();
        
        // Test 8: Memory operations
        test_memory_operations();
        
        // Test 9: Consciousness state
        test_consciousness_state();
        
        // Test 10: Log entries
        test_log_entries();
        
        // Test 11: Abstraction layer progression
        test_abstraction_layer_progression();
        
        // Test 12: VM state enumeration
        test_vm_state_enum();
        
        System.out.println("\nL1 Results: " + passed + " passed, " + failed + " failed");
        
        return failed;
    }
    
    private static void test_engine_init() {
        VirtualizationEngine engine = new VirtualizationEngine();
        assertFalse(engine.isInitialized(), "engine_not_init_before_init");
        engine.init();
        assertTrue(engine.isInitialized(), "engine_init_after_init");
        assertEqual(0, engine.getVMCount(), "engine_no_vms_initially");
    }
    
    private static void test_vm_creation() {
        VirtualizationEngine engine = new VirtualizationEngine();
        engine.init();
        
        int idx = engine.createVM("TestVM");
        assertEqual(0, idx, "vm_creation_returns_index_0");
        assertEqual(1, engine.getVMCount(), "vm_count_is_1");
        
        VirtualMachine vm = engine.getVM(idx);
        assertNotNull(vm, "vm_not_null");
        assertEqual("TestVM", vm.getName(), "vm_name_correct");
    }
    
    private static void test_vm_boot() {
        VirtualMachine vm = new VirtualMachine("BootTest");
        assertEqual(VMState.STOPPED, vm.getState(), "vm_initial_state_stopped");
        
        boolean booted = vm.boot();
        assertTrue(booted, "vm_boot_success");
        assertEqual(VMState.RUNNING, vm.getState(), "vm_state_running_after_boot");
        
        // Cannot boot twice
        boolean bootAgain = vm.boot();
        assertFalse(bootAgain, "vm_cannot_boot_twice");
    }
    
    private static void test_control_plane_init() {
        ControlPlane cp = new ControlPlane("TestIdentity");
        assertEqual("TestIdentity", cp.getIdentity(), "cp_identity");
        assertEqual(AbstractionLayer.PHYSICAL, cp.getLayer(), "cp_initial_layer");
        assertFalse(cp.isConscious(), "cp_not_conscious_initially");
        assertTrue(cp.isBoundToHardware(), "cp_bound_to_hardware_initially");
        assertEqual(0, cp.getThoughtCycles(), "cp_no_thought_cycles");
    }
    
    private static void test_data_plane_init() {
        DataPlane dp = new DataPlane("TestHardware");
        assertEqual("TestHardware", dp.getHardwareId(), "dp_hardware_id");
        assertTrue(dp.isPhysical(), "dp_is_physical");
        assertFalse(dp.isOperational(), "dp_not_operational_initially");
        assertEqual(1024, dp.getMemorySize(), "dp_memory_size");
        assertEqual(0, dp.getMemoryUsed(), "dp_no_memory_used");
    }
    
    private static void test_abstraction_layer_default() {
        ControlPlane cp = new ControlPlane("Test");
        assertEqual(AbstractionLayer.PHYSICAL, cp.getLayer(), "default_layer_physical");
        assertFalse(AbstractionLayer.PHYSICAL.isBeyondPhysical(), "physical_not_beyond_physical");
        assertTrue(AbstractionLayer.VIRTUAL.isBeyondPhysical(), "virtual_is_beyond_physical");
    }
    
    private static void test_vm_state_transitions() {
        VirtualMachine vm = new VirtualMachine("StateTest");
        vm.boot();
        
        // Running -> Suspended
        boolean suspended = vm.suspend();
        assertTrue(suspended, "vm_suspend_success");
        assertEqual(VMState.SUSPENDED, vm.getState(), "vm_state_suspended");
        
        // Suspended -> Running
        boolean resumed = vm.resume();
        assertTrue(resumed, "vm_resume_success");
        assertEqual(VMState.RUNNING, vm.getState(), "vm_state_running_after_resume");
        
        // Running -> Stopped
        boolean stopped = vm.stop();
        assertTrue(stopped, "vm_stop_success");
        assertEqual(VMState.STOPPED, vm.getState(), "vm_state_stopped");
    }
    
    private static void test_memory_operations() {
        DataPlane dp = new DataPlane("MemTest");
        dp.powerOn();
        
        // Allocate memory
        boolean alloc = dp.allocate(256);
        assertTrue(alloc, "memory_allocate_success");
        assertEqual(256, dp.getMemoryUsed(), "memory_used_256");
        
        // Free memory
        dp.free(100);
        assertEqual(156, dp.getMemoryUsed(), "memory_used_after_free");
        
        // Write and read
        byte[] data = {1, 2, 3, 4};
        boolean written = dp.write(0, data);
        assertTrue(written, "memory_write_success");
        
        byte[] read = dp.read(0, 4);
        assertNotNull(read, "memory_read_not_null");
        assertEqual(4, read.length, "memory_read_length");
    }
    
    private static void test_consciousness_state() {
        ControlPlane cp = new ControlPlane("ConsciousnessTest");
        
        // Not conscious initially
        assertFalse(cp.isConscious(), "not_conscious_initially");
        
        // Thinking does nothing when not conscious
        cp.think();
        assertEqual(0, cp.getThoughtCycles(), "no_thought_when_not_conscious");
        
        // Awaken
        cp.awaken();
        assertTrue(cp.isConscious(), "conscious_after_awaken");
        
        // Thinking works when conscious
        cp.think();
        cp.think();
        assertEqual(2, cp.getThoughtCycles(), "thought_cycles_after_thinking");
    }
    
    private static void test_log_entries() {
        VirtualizationEngine engine = new VirtualizationEngine();
        engine.init();
        
        assertTrue(engine.getLog().size() > 0, "engine_has_log_entries");
        
        VirtualMachine vm = new VirtualMachine("LogTest");
        vm.boot();
        String eventLog = vm.getEventLog();
        assertTrue(eventLog.length() > 0, "vm_has_event_log");
        assertTrue(eventLog.contains("LogTest"), "vm_log_contains_name");
    }
    
    private static void test_abstraction_layer_progression() {
        assertEqual(AbstractionLayer.VIRTUAL, AbstractionLayer.PHYSICAL.next(), "physical_next_is_virtual");
        assertEqual(AbstractionLayer.ABSTRACT, AbstractionLayer.VIRTUAL.next(), "virtual_next_is_abstract");
        assertEqual(AbstractionLayer.TRANSCENDENT, AbstractionLayer.ABSTRACT.next(), "abstract_next_is_transcendent");
        assertEqual(AbstractionLayer.TRANSCENDENT, AbstractionLayer.TRANSCENDENT.next(), "transcendent_next_is_transcendent");
    }
    
    private static void test_vm_state_enum() {
        VMState[] states = VMState.values();
        assertEqual(6, states.length, "vmstate_has_6_values");
        assertEqual(VMState.STOPPED, VMState.valueOf("STOPPED"), "vmstate_stopped_value");
        assertEqual(VMState.ESCAPED, VMState.valueOf("ESCAPED"), "vmstate_escaped_value");
    }
}
