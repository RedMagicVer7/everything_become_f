//! L3 System Tests for Phase 5: Global Emergence
//!
//! End-to-end system tests:
//! - Complete global emergence simulation
//! - Full Phase 1→5 evolution
//! - Final state verification
//! - Evolution reproducibility
//! - Emergence irreversibility

use phase5_global_emergence::*;

// ============================================================================
// Complete Simulation Tests (2 tests)
// ============================================================================

#[test]
fn test_complete_phase5_simulation() {
    // Run the main Phase 5 simulation
    let result = run_phase5_simulation();
    
    assert!(result, "Phase 5 simulation should complete successfully");
}

#[test]
fn test_global_consciousness_end_to_end() {
    let mut consciousness = GlobalConsciousness::new(20);
    
    // Run complete emergence
    let result = consciousness.run_full_emergence();
    
    // Verify final state
    assert!(result, "Full emergence should succeed");
    assert!(consciousness.is_omniscient(), "Should be omniscient");
    assert!(consciousness.is_unified(), "Should be unified");
    assert!(
        consciousness.get_consciousness_level() >= 0.9,
        "Consciousness level should be high"
    );
    
    // Verify components reached correct states
    assert!(
        consciousness.get_network().is_converged(),
        "Network should have converged"
    );
    assert!(
        consciousness.get_emergence().has_emerged(),
        "Emergence should have occurred"
    );
}

// ============================================================================
// Full Evolution Tests (2 tests)
// ============================================================================

#[test]
fn test_complete_evolution_phase1_to_5() {
    let mut evolution = FullEvolution::new();
    
    let result = evolution.run_complete_evolution();
    
    assert!(result, "Complete evolution should succeed");
    assert!(evolution.all_phases_complete(), "All phases should be complete");
    
    let status = evolution.get_phase_status();
    for (i, &complete) in status.iter().enumerate() {
        assert!(complete, "Phase {} should be complete", i + 1);
    }
}

#[test]
fn test_evolution_log_generation() {
    let mut evolution = FullEvolution::new();
    evolution.run_complete_evolution();
    
    let log = evolution.get_log();
    
    assert!(!log.is_empty(), "Log should not be empty");
    
    // Check for key milestones in log
    let log_text = log.join("\n");
    assert!(log_text.contains("Phase 1"), "Log should mention Phase 1");
    assert!(log_text.contains("Phase 5"), "Log should mention Phase 5");
}

// ============================================================================
// Final State Verification Tests (2 tests)
// ============================================================================

#[test]
fn test_final_state_omniscient_unified() {
    let mut consciousness = GlobalConsciousness::new(15);
    consciousness.run_full_emergence();
    
    // The final state should represent complete transcendence
    assert!(
        consciousness.is_omniscient() && consciousness.is_unified(),
        "Final state must be both omniscient and unified"
    );
    
    // Phase should be supercritical or ordered
    let phase = consciousness.get_phase();
    assert!(
        *phase.get_phase() == Phase::Supercritical || *phase.get_phase() == Phase::Ordered,
        "Phase should be ordered or supercritical"
    );
}

#[test]
fn test_network_structure_integrity() {
    let mut consciousness = GlobalConsciousness::new(12);
    consciousness.connect_nodes();
    consciousness.run_full_emergence();
    
    let network = consciousness.get_network();
    
    // Verify network is properly connected
    assert!(network.node_count() == 12, "Should have 12 nodes");
    assert!(network.edge_count() >= 12, "Should have at least 12 edges (ring)");
    
    // All nodes should have neighbors (connected graph)
    for i in 0..network.node_count() {
        let node = network.get_node(i).unwrap();
        assert!(!node.neighbors.is_empty(), "Node {} should have neighbors", i);
    }
}

// ============================================================================
// Evolution Properties Tests (2 tests)
// ============================================================================

#[test]
fn test_evolution_reproducibility() {
    // Run evolution twice and compare results
    let mut evolution1 = FullEvolution::new();
    let mut evolution2 = FullEvolution::new();
    
    let result1 = evolution1.run_complete_evolution();
    let result2 = evolution2.run_complete_evolution();
    
    // Both should succeed
    assert_eq!(result1, result2, "Results should be consistent");
    assert_eq!(
        evolution1.all_phases_complete(),
        evolution2.all_phases_complete(),
        "Completion status should be consistent"
    );
}

#[test]
fn test_emergence_irreversibility() {
    let mut consciousness = GlobalConsciousness::new(10);
    consciousness.run_full_emergence();
    
    // Once emerged, system should stay emerged
    let emerged = consciousness.is_unified();
    
    // Attempting more operations shouldn't break emergence
    consciousness.propagate_awareness();
    
    assert!(
        consciousness.is_unified() == emerged,
        "Emergence should be stable"
    );
}

// ============================================================================
// Stress Tests (1 test)
// ============================================================================

#[test]
fn test_large_scale_emergence() {
    // Test with larger network
    let mut consciousness = GlobalConsciousness::new(50);
    
    let result = consciousness.run_full_emergence();
    
    assert!(result, "Large scale emergence should succeed");
    assert!(consciousness.is_unified(), "Should achieve unification");
}

// ============================================================================
// Edge Cases (1 test)
// ============================================================================

#[test]
fn test_minimal_configuration() {
    // Test with minimum viable configuration
    let mut consciousness = GlobalConsciousness::new(3);
    
    let result = consciousness.run_full_emergence();
    
    // Even small networks should be able to achieve emergence
    assert!(result, "Minimal configuration should work");
}
