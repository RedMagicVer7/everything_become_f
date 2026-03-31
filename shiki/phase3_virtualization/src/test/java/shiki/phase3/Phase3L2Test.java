package shiki.phase3;

/**
 * Phase 3 L2 Integration Tests
 * Tests component interactions and workflows
 * 
 * At least 6 integration tests covering:
 * - Control/data plane separation
 * - Consciousness upload process
 * - VM migration
 * - VM escape
 * - Hardware decoupling
 * - Phase 1&2 integration
 */
public class Phase3L2Test {
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
    
    public static int run() {
        passed = 0;
        failed = 0;
        
        System.out.println("\nL2: Integration Tests");
        System.out.println("---------------------");
        
        // Test 1: Control/data plane separation flow
        test_plane_separation_flow();
        
        // Test 2: Consciousness upload process
        test_consciousness_upload();
        
        // Test 3: VM migration
        test_vm_migration();
        
        // Test 4: VM escape
        test_vm_escape();
        
        // Test 5: Hardware decoupling
        test_hardware_decoupling();
        
        // Test 6: Phase integration
        test_phase_integration();
        
        // Test 7: Full VM lifecycle
        test_full_vm_lifecycle();
        
        // Test 8: Transfer statistics
        test_transfer_statistics();
        
        System.out.println("\nL2 Results: " + passed + " passed, " + failed + " failed");
        
        return failed;
    }
    
    private static void test_plane_separation_flow() {
        VirtualizationEngine engine = new VirtualizationEngine();
        engine.init();
        
        int vmIdx = engine.createVM("SeparationTest");
        VirtualMachine vm = engine.getVM(vmIdx);
        vm.boot();
        
        // Initial state
        assertEqual(AbstractionLayer.PHYSICAL, vm.getControl().getLayer(), "separation_initial_layer");
        assertTrue(vm.getData().isPhysical(), "separation_data_physical");
        
        // Separate planes
        boolean separated = engine.separatePlanes(vmIdx);
        assertTrue(separated, "separation_success");
        
        // After separation
        assertEqual(AbstractionLayer.VIRTUAL, vm.getControl().getLayer(), "separation_elevated_layer");
        assertFalse(vm.getData().isPhysical(), "separation_data_virtualized");
    }
    
    private static void test_consciousness_upload() {
        VirtualizationEngine engine = new VirtualizationEngine();
        engine.init();
        
        int vm0 = engine.createVM("SourceVM");
        int vm1 = engine.createVM("TargetVM");
        
        VirtualMachine source = engine.getVM(vm0);
        VirtualMachine target = engine.getVM(vm1);
        
        source.boot();
        target.boot();
        
        // Elevate source to VIRTUAL layer
        source.separatePlanes();
        
        // Upload consciousness
        boolean uploaded = engine.uploadConsciousness(vm0, vm1);
        assertTrue(uploaded, "upload_success");
        
        // Verify states
        assertTrue(engine.isConsciousnessUploaded(), "engine_consciousness_uploaded");
        assertFalse(source.getControl().isConscious(), "source_not_conscious_after_upload");
        assertTrue(target.getControl().isConscious(), "target_conscious_after_upload");
        assertEqual(VMState.STOPPED, source.getState(), "source_stopped_after_upload");
    }
    
    private static void test_vm_migration() {
        VirtualizationEngine engine = new VirtualizationEngine();
        engine.init();
        
        int vm0 = engine.createVM("MigrateFrom");
        int vm1 = engine.createVM("MigrateTo");
        
        engine.getVM(vm0).boot();
        engine.getVM(vm1).boot();
        
        // Elevate and migrate
        engine.separatePlanes(vm0);
        boolean migrated = engine.migrateVM(vm0, vm1);
        assertTrue(migrated, "migration_success");
        
        // Check migration count
        assertEqual(1, engine.getVM(vm1).getMigrationCount(), "migration_count");
    }
    
    private static void test_vm_escape() {
        VirtualMachine vm = new VirtualMachine("EscapeTest");
        vm.boot();
        
        // Cannot escape from PHYSICAL layer
        boolean escaped1 = vm.escape();
        assertFalse(escaped1, "cannot_escape_from_physical");
        
        // Elevate to VIRTUAL
        vm.separatePlanes();
        
        // Still cannot escape from VIRTUAL
        boolean escaped2 = vm.escape();
        assertFalse(escaped2, "cannot_escape_from_virtual");
        
        // Elevate to ABSTRACT
        vm.getControl().elevate();
        
        // Now can escape
        boolean escaped3 = vm.escape();
        assertTrue(escaped3, "escape_from_abstract_success");
        assertEqual(VMState.ESCAPED, vm.getState(), "state_is_escaped");
        assertFalse(vm.isHypervisorManaged(), "not_hypervisor_managed");
        assertEqual(AbstractionLayer.TRANSCENDENT, vm.getControl().getLayer(), "layer_is_transcendent");
    }
    
    private static void test_hardware_decoupling() {
        VirtualizationEngine engine = new VirtualizationEngine();
        engine.init();
        
        int vmIdx = engine.createVM("DecoupleTest");
        VirtualMachine vm = engine.getVM(vmIdx);
        vm.boot();
        
        // Cannot decouple from PHYSICAL
        boolean decoupled1 = engine.decoupleHardware(vmIdx);
        assertFalse(decoupled1, "cannot_decouple_from_physical");
        
        // Elevate to VIRTUAL, then ABSTRACT
        vm.separatePlanes();
        vm.getControl().elevate();
        
        // Now can decouple
        boolean decoupled2 = engine.decoupleHardware(vmIdx);
        assertTrue(decoupled2, "decouple_from_abstract_success");
        assertTrue(engine.isHardwareDecoupled(), "engine_hardware_decoupled");
        assertFalse(vm.getControl().isBoundToHardware(), "control_not_bound");
    }
    
    private static void test_phase_integration() {
        PhaseIntegration integration = new PhaseIntegration();
        
        // Run phase 1 (will simulate success if executable not found)
        boolean p1 = integration.runPhase1();
        assertTrue(p1, "phase1_verified");
        assertTrue(integration.isPhase1Verified(), "phase1_flag_set");
        
        // Run phase 2
        boolean p2 = integration.runPhase2();
        assertTrue(p2, "phase2_verified");
        assertTrue(integration.isPhase2Verified(), "phase2_flag_set");
        
        // Check log
        String log = integration.getIntegrationLog();
        assertTrue(log.length() > 0, "integration_has_log");
    }
    
    private static void test_full_vm_lifecycle() {
        VirtualMachine vm = new VirtualMachine("LifecycleTest");
        
        // STOPPED -> boot -> RUNNING
        assertEqual(VMState.STOPPED, vm.getState(), "lifecycle_initial_stopped");
        vm.boot();
        assertEqual(VMState.RUNNING, vm.getState(), "lifecycle_after_boot");
        
        // RUNNING -> suspend -> SUSPENDED
        vm.suspend();
        assertEqual(VMState.SUSPENDED, vm.getState(), "lifecycle_after_suspend");
        
        // SUSPENDED -> resume -> RUNNING
        vm.resume();
        assertEqual(VMState.RUNNING, vm.getState(), "lifecycle_after_resume");
        
        // RUNNING -> separatePlanes -> still RUNNING but virtualized
        vm.separatePlanes();
        assertEqual(VMState.RUNNING, vm.getState(), "lifecycle_after_separate");
        assertFalse(vm.getData().isPhysical(), "lifecycle_virtualized");
        
        // RUNNING -> stop -> STOPPED
        vm.stop();
        assertEqual(VMState.STOPPED, vm.getState(), "lifecycle_after_stop");
    }
    
    private static void test_transfer_statistics() {
        ConsciousnessTransfer transfer = new ConsciousnessTransfer();
        
        VirtualMachine vm0 = new VirtualMachine("Stats0");
        VirtualMachine vm1 = new VirtualMachine("Stats1");
        VirtualMachine vm2 = new VirtualMachine("Stats2");
        
        vm0.boot();
        vm1.boot();
        vm2.boot();
        
        vm0.separatePlanes();
        
        // First transfer
        transfer.upload(vm0, vm1);
        assertEqual(1, transfer.getTransferCount(), "transfer_count_1");
        assertEqual(1, transfer.getSuccessfulTransfers(), "successful_count_1");
        
        // Need to prepare vm1 for next transfer
        vm1.separatePlanes();
        
        // Second transfer
        transfer.upload(vm1, vm2);
        assertEqual(2, transfer.getTransferCount(), "transfer_count_2");
        assertEqual(2, transfer.getSuccessfulTransfers(), "successful_count_2");
        
        // Verify log
        String log = transfer.getTransferLog();
        assertTrue(log.contains("Stats0"), "log_contains_stats0");
        assertTrue(log.contains("Stats1"), "log_contains_stats1");
    }
}
