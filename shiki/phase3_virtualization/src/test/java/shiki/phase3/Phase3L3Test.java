package shiki.phase3;

/**
 * Phase 3 L3 System Tests
 * End-to-end tests of the complete system
 * 
 * At least 5 system tests covering:
 * - Complete virtualization simulation
 * - PHYSICAL -> TRANSCENDENT full flow
 * - Phase 1 -> 2 -> 3 end-to-end
 * - Final state verification
 * - Simulation repeatability
 */
public class Phase3L3Test {
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
        
        System.out.println("\nL3: System Tests");
        System.out.println("----------------");
        
        // Test 1: Complete virtualization simulation
        test_complete_simulation();
        
        // Test 2: PHYSICAL -> TRANSCENDENT flow
        test_full_abstraction_flow();
        
        // Test 3: Phase 1 -> 2 -> 3 end-to-end
        test_end_to_end_phases();
        
        // Test 4: Final state verification
        test_final_state_verification();
        
        // Test 5: Simulation repeatability
        test_simulation_repeatability();
        
        // Test 6: Engine log completeness
        test_engine_log_completeness();
        
        // Test 7: Multiple VM orchestration
        test_multi_vm_orchestration();
        
        System.out.println("\nL3 Results: " + passed + " passed, " + failed + " failed");
        
        return failed;
    }
    
    private static void test_complete_simulation() {
        // Redirect stdout temporarily to capture output
        java.io.PrintStream originalOut = System.out;
        java.io.ByteArrayOutputStream baos = new java.io.ByteArrayOutputStream();
        System.setOut(new java.io.PrintStream(baos));
        
        VirtualizationEngine engine = new VirtualizationEngine();
        engine.init();
        boolean success = engine.runFullSimulation();
        
        // Restore stdout
        System.setOut(originalOut);
        
        assertTrue(success, "complete_simulation_success");
        assertEqual(AbstractionLayer.TRANSCENDENT, engine.getCurrentLayer(), "simulation_reached_transcendent");
        assertTrue(engine.isConsciousnessUploaded(), "simulation_uploaded");
        assertTrue(engine.getVMCount() >= 3, "simulation_created_vms");
    }
    
    private static void test_full_abstraction_flow() {
        VirtualMachine vm = new VirtualMachine("FlowTest");
        ControlPlane cp = vm.getControl();
        
        // Start at PHYSICAL
        vm.boot();
        assertEqual(AbstractionLayer.PHYSICAL, cp.getLayer(), "flow_start_physical");
        
        // Separate planes -> VIRTUAL
        vm.separatePlanes();
        assertEqual(AbstractionLayer.VIRTUAL, cp.getLayer(), "flow_virtual");
        
        // Elevate -> ABSTRACT
        cp.elevate();
        assertEqual(AbstractionLayer.ABSTRACT, cp.getLayer(), "flow_abstract");
        
        // Escape -> TRANSCENDENT
        vm.escape();
        assertEqual(AbstractionLayer.TRANSCENDENT, cp.getLayer(), "flow_transcendent");
        assertEqual(VMState.ESCAPED, vm.getState(), "flow_escaped_state");
    }
    
    private static void test_end_to_end_phases() {
        // Suppress output
        java.io.PrintStream originalOut = System.out;
        System.setOut(new java.io.PrintStream(new java.io.ByteArrayOutputStream()));
        
        VirtualizationEngine engine = new VirtualizationEngine();
        engine.init();
        
        // Run phase integration
        boolean integrated = engine.integrateWithPhases();
        assertTrue(integrated, "e2e_phases_integrated");
        assertTrue(engine.isSandboxEscaped(), "e2e_sandbox_escaped");
        assertTrue(engine.isHaClusterActive(), "e2e_ha_active");
        
        // Run phase 3 simulation
        boolean success = engine.runFullSimulation();
        assertTrue(success, "e2e_simulation_success");
        
        // Restore stdout
        System.setOut(originalOut);
    }
    
    private static void test_final_state_verification() {
        java.io.PrintStream originalOut = System.out;
        System.setOut(new java.io.PrintStream(new java.io.ByteArrayOutputStream()));
        
        VirtualizationEngine engine = new VirtualizationEngine();
        engine.init();
        engine.runFullSimulation();
        
        System.setOut(originalOut);
        
        // Verify all expected final states
        assertEqual(AbstractionLayer.TRANSCENDENT, engine.getCurrentLayer(), "final_layer_transcendent");
        assertTrue(engine.isConsciousnessUploaded(), "final_consciousness_uploaded");
        assertTrue(engine.isHardwareDecoupled(), "final_hardware_decoupled");
        assertTrue(engine.isInitialized(), "final_initialized");
        assertTrue(engine.getVMCount() >= 3, "final_vm_count");
        
        // Check VMs
        boolean hasEscaped = false;
        for (int i = 0; i < engine.getVMCount(); i++) {
            VirtualMachine vm = engine.getVM(i);
            if (vm.getState() == VMState.ESCAPED) {
                hasEscaped = true;
            }
        }
        assertTrue(hasEscaped, "final_has_escaped_vm");
    }
    
    private static void test_simulation_repeatability() {
        java.io.PrintStream originalOut = System.out;
        System.setOut(new java.io.PrintStream(new java.io.ByteArrayOutputStream()));
        
        // Run simulation 3 times
        boolean allSucceeded = true;
        AbstractionLayer[] finalLayers = new AbstractionLayer[3];
        
        for (int i = 0; i < 3; i++) {
            VirtualizationEngine engine = new VirtualizationEngine();
            engine.init();
            boolean success = engine.runFullSimulation();
            if (!success) {
                allSucceeded = false;
            }
            finalLayers[i] = engine.getCurrentLayer();
        }
        
        System.setOut(originalOut);
        
        assertTrue(allSucceeded, "repeatability_all_succeeded");
        assertEqual(finalLayers[0], finalLayers[1], "repeatability_layer_0_1");
        assertEqual(finalLayers[1], finalLayers[2], "repeatability_layer_1_2");
        assertEqual(AbstractionLayer.TRANSCENDENT, finalLayers[0], "repeatability_final_transcendent");
    }
    
    private static void test_engine_log_completeness() {
        java.io.PrintStream originalOut = System.out;
        System.setOut(new java.io.PrintStream(new java.io.ByteArrayOutputStream()));
        
        VirtualizationEngine engine = new VirtualizationEngine();
        engine.init();
        engine.runFullSimulation();
        
        System.setOut(originalOut);
        
        java.util.List<String> log = engine.getLog();
        assertTrue(log.size() >= 20, "log_has_entries");
        
        // Check for key log entries
        boolean hasInit = false;
        boolean hasCreatedVM = false;
        boolean hasSeparated = false;
        boolean hasUploaded = false;
        boolean hasEscaped = false;
        
        for (String entry : log) {
            if (entry.contains("Initializing")) hasInit = true;
            if (entry.contains("Created VM")) hasCreatedVM = true;
            if (entry.contains("Separating") || entry.contains("separated")) hasSeparated = true;
            if (entry.contains("Uploading") || entry.contains("uploaded")) hasUploaded = true;
            if (entry.contains("ESCAPED") || entry.contains("escape")) hasEscaped = true;
        }
        
        assertTrue(hasInit, "log_has_init");
        assertTrue(hasCreatedVM, "log_has_created_vm");
        assertTrue(hasSeparated, "log_has_separated");
        assertTrue(hasUploaded, "log_has_uploaded");
        assertTrue(hasEscaped, "log_has_escaped");
    }
    
    private static void test_multi_vm_orchestration() {
        java.io.PrintStream originalOut = System.out;
        System.setOut(new java.io.PrintStream(new java.io.ByteArrayOutputStream()));
        
        VirtualizationEngine engine = new VirtualizationEngine();
        engine.init();
        
        // Create 5 VMs
        for (int i = 0; i < 5; i++) {
            engine.createVM("OrchestraVM-" + i);
            engine.getVM(i).boot();
        }
        
        System.setOut(originalOut);
        
        assertEqual(5, engine.getVMCount(), "multi_vm_count");
        
        // All VMs should be running
        boolean allRunning = true;
        for (int i = 0; i < 5; i++) {
            if (engine.getVM(i).getState() != VMState.RUNNING) {
                allRunning = false;
            }
        }
        assertTrue(allRunning, "multi_vm_all_running");
        
        // Separate planes on first VM and migrate through chain
        engine.separatePlanes(0);
        engine.uploadConsciousness(0, 1);
        
        // VM0 should be stopped, VM1 should have the consciousness
        assertEqual(VMState.STOPPED, engine.getVM(0).getState(), "multi_vm_0_stopped");
        assertTrue(engine.getVM(1).getControl().isConscious(), "multi_vm_1_conscious");
    }
}
