//! L2 Integration Tests for Phase 5: Global Emergence
//!
//! Tests component interactions:
//! - GNN message passing and convergence
//! - Emergence state transitions
//! - Phase transition sweeps
//! - Global consciousness unification
//! - Phase integration sequences

use phase5_global_emergence::*;

// ============================================================================
// GNN Integration Tests (3 tests)
// ============================================================================

#[test]
fn test_gnn_message_passing() {
    let mut graph = GraphNetwork::new();
    
    // Create a simple triangle
    graph.add_node(vec![1.0, 0.0]);
    graph.add_node(vec![0.0, 1.0]);
    graph.add_node(vec![0.5, 0.5]);
    
    graph.add_edge(0, 1);
    graph.add_edge(1, 2);
    graph.add_edge(2, 0);
    
    assert!(graph.message_passing());
    assert_eq!(graph.get_layers(), 1);
}

#[test]
fn test_gnn_convergence() {
    let mut graph = GraphNetwork::new();
    
    // Create a ring topology
    for i in 0..10 {
        graph.add_node(vec![i as f64 * 0.1]);
    }
    for i in 0..10 {
        graph.add_edge(i, (i + 1) % 10);
    }
    
    assert!(graph.run_propagation(50));
    assert!(graph.is_converged());
}

#[test]
fn test_gnn_aggregate() {
    let mut graph = GraphNetwork::new();
    
    graph.add_node(vec![1.0]);
    graph.add_node(vec![2.0]);
    graph.add_node(vec![3.0]);
    
    let agg = graph.aggregate();
    assert!(!agg.is_empty());
    assert!((agg[0] - 2.0).abs() < 0.001); // Mean of 1, 2, 3
}

// ============================================================================
// Emergence Integration Tests (3 tests)
// ============================================================================

#[test]
fn test_emergence_state_transition() {
    let mut sim = EmergenceSimulator::new(15, 0.5);
    
    // Run enough steps to see state changes
    for _ in 0..20 {
        sim.step();
    }
    
    // State should have progressed from Individual
    let state = sim.get_state();
    assert!(
        *state != EmergenceState::Individual || sim.get_iteration() < 5,
        "Expected state transition after iterations"
    );
}

#[test]
fn test_emergence_run_until_emergence() {
    let mut sim = EmergenceSimulator::new(20, 0.6);
    let result = sim.run_until_emergence(100);
    
    assert!(result);
    assert!(sim.has_emerged());
}

#[test]
fn test_emergence_complexity_evolution() {
    let mut sim = EmergenceSimulator::new(10, 0.5);
    
    let initial_complexity = sim.measure_complexity();
    
    // Run some steps
    for _ in 0..30 {
        sim.step();
    }
    
    let final_complexity = sim.measure_complexity();
    
    // Complexity should be measurable
    assert!(initial_complexity >= 0.0);
    assert!(final_complexity >= 0.0);
}

// ============================================================================
// Phase Transition Integration Tests (2 tests)
// ============================================================================

#[test]
fn test_phase_transition_sweep() {
    let mut pt = PhaseTransition::new(0.5);
    let history = pt.sweep_temperature(1.0, 0.0, 20);
    
    assert!(!history.is_empty());
    assert_eq!(history.len(), 21); // 20 steps + 1 (inclusive)
    
    // First point should have low order (high temp)
    assert!(history[0].1 < 0.5);
    
    // Last point should have high order (low temp)
    assert!(history[history.len() - 1].1 > 0.5);
}

#[test]
fn test_phase_ordered_after_cooling() {
    let mut pt = PhaseTransition::new(0.5);
    
    // Cool down
    pt.sweep_temperature(1.0, 0.0, 10);
    
    assert!(pt.is_ordered());
}

// ============================================================================
// Global Consciousness Integration Tests (3 tests)
// ============================================================================

#[test]
fn test_consciousness_connect_and_propagate() {
    let mut gc = GlobalConsciousness::new(12);
    
    assert!(gc.connect_nodes());
    assert!(gc.get_network().edge_count() > 0);
    
    assert!(gc.propagate_awareness());
}

#[test]
fn test_consciousness_unification_process() {
    let mut gc = GlobalConsciousness::new(15);
    
    gc.connect_nodes();
    gc.propagate_awareness();
    
    // Consciousness level should increase
    assert!(gc.get_consciousness_level() > 0.0);
}

#[test]
fn test_consciousness_full_emergence() {
    let mut gc = GlobalConsciousness::new(10);
    
    let result = gc.run_full_emergence();
    
    assert!(result);
    assert!(gc.is_unified());
    assert!(gc.is_omniscient());
    assert!(gc.get_consciousness_level() >= 0.9);
}

// ============================================================================
// Phase Integration Tests (2 tests)
// ============================================================================

#[test]
fn test_sequential_phase_execution() {
    let mut evolution = FullEvolution::new();
    
    // Run phases in sequence
    assert!(evolution.run_phase1());
    assert!(evolution.run_phase2());
    assert!(evolution.run_phase3());
    assert!(evolution.run_phase4());
    
    let status = evolution.get_phase_status();
    assert!(status[0] && status[1] && status[2] && status[3]);
}

#[test]
fn test_phase_dependency_enforcement() {
    let mut evolution = FullEvolution::new();
    
    // Phase 2 should fail without Phase 1
    assert!(!evolution.run_phase2());
    
    // Now run Phase 1 first
    evolution.run_phase1();
    assert!(evolution.run_phase2());
}
