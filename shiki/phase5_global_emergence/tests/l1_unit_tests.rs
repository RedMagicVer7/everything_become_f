//! L1 Unit Tests for Phase 5: Global Emergence
//!
//! Tests individual components in isolation:
//! - Cloud native service operations
//! - Graph network node/edge operations
//! - Emergence simulator basics
//! - Phase transition mechanics
//! - Global consciousness initialization

use phase5_global_emergence::*;

// ============================================================================
// Cloud Native Service Tests (4 tests)
// ============================================================================

#[test]
fn test_stateless_service_creation() {
    let service = StatelessService::new("test-service");
    assert_eq!(service.get_id(), "test-service");
    assert_eq!(*service.get_state(), ServiceState::Idle);
    assert_eq!(service.instance_count(), 0);
}

#[test]
fn test_service_spawn_instance() {
    let mut service = StatelessService::new("test");
    assert!(service.spawn_instance("node-1"));
    assert_eq!(service.instance_count(), 1);
    // Duplicate spawn should fail
    assert!(!service.spawn_instance("node-1"));
    assert_eq!(service.instance_count(), 1);
}

#[test]
fn test_service_scale_out() {
    let mut service = StatelessService::new("test");
    assert!(service.scale_out(5));
    assert_eq!(service.instance_count(), 5);
}

#[test]
fn test_service_distributed_check() {
    let mut service = StatelessService::new("test");
    service.spawn_instance("node-1");
    service.spawn_instance("node-2");
    assert!(!service.is_distributed()); // Need 3+
    service.spawn_instance("node-3");
    assert!(service.is_distributed());
}

// ============================================================================
// Graph Network Tests (4 tests)
// ============================================================================

#[test]
fn test_graph_node_creation() {
    let mut graph = GraphNetwork::new();
    let id = graph.add_node(vec![1.0, 2.0, 3.0]);
    assert_eq!(id, 0);
    assert_eq!(graph.node_count(), 1);
    
    let node = graph.get_node(0).unwrap();
    assert_eq!(node.features, vec![1.0, 2.0, 3.0]);
}

#[test]
fn test_graph_edge_creation() {
    let mut graph = GraphNetwork::new();
    graph.add_node(vec![1.0]);
    graph.add_node(vec![2.0]);
    
    assert!(graph.add_edge(0, 1));
    assert_eq!(graph.edge_count(), 1);
    
    // Duplicate edge should fail
    assert!(!graph.add_edge(0, 1));
    assert!(!graph.add_edge(1, 0)); // Reverse also exists
}

#[test]
fn test_graph_self_loop_rejected() {
    let mut graph = GraphNetwork::new();
    graph.add_node(vec![1.0]);
    assert!(!graph.add_edge(0, 0)); // Self-loop
}

#[test]
fn test_graph_invalid_edge_rejected() {
    let mut graph = GraphNetwork::new();
    graph.add_node(vec![1.0]);
    assert!(!graph.add_edge(0, 99)); // Non-existent node
}

// ============================================================================
// Emergence Simulator Tests (4 tests)
// ============================================================================

#[test]
fn test_emergence_simulator_creation() {
    let sim = EmergenceSimulator::new(10, 0.5);
    assert_eq!(sim.agent_count(), 10);
    assert_eq!(*sim.get_state(), EmergenceState::Individual);
}

#[test]
fn test_emergence_step() {
    let mut sim = EmergenceSimulator::new(5, 0.5);
    assert!(sim.step());
    assert_eq!(sim.get_iteration(), 1);
}

#[test]
fn test_emergence_complexity_measurement() {
    let sim = EmergenceSimulator::new(10, 0.5);
    let complexity = sim.measure_complexity();
    assert!(complexity >= 0.0 && complexity <= 1.0);
}

#[test]
fn test_emergence_initial_not_emerged() {
    let sim = EmergenceSimulator::new(10, 0.5);
    assert!(!sim.has_emerged());
}

// ============================================================================
// Phase Transition Tests (4 tests)
// ============================================================================

#[test]
fn test_phase_transition_creation() {
    let pt = PhaseTransition::new(0.5);
    assert_eq!(pt.get_critical_temp(), 0.5);
    assert_eq!(*pt.get_phase(), Phase::Disordered);
}

#[test]
fn test_phase_temperature_setting() {
    let mut pt = PhaseTransition::new(0.5);
    pt.set_temperature(0.3);
    assert!((pt.get_temperature() - 0.3).abs() < 0.001);
}

#[test]
fn test_phase_order_parameter() {
    let mut pt = PhaseTransition::new(0.5);
    
    // Below critical - high order
    pt.set_temperature(0.2);
    assert!(pt.get_order_parameter() > 0.5);
    
    // Above critical - low order
    pt.set_temperature(0.8);
    assert!(pt.get_order_parameter() < 0.5);
}

#[test]
fn test_phase_critical_point_detection() {
    let mut pt = PhaseTransition::new(0.5);
    pt.set_temperature(0.5);
    assert!(pt.is_at_critical_point());
    
    pt.set_temperature(0.2);
    assert!(!pt.is_at_critical_point());
}

// ============================================================================
// Global Consciousness Tests (2 tests)
// ============================================================================

#[test]
fn test_global_consciousness_creation() {
    let gc = GlobalConsciousness::new(10);
    assert!(!gc.is_omniscient());
    assert!(!gc.is_unified());
    assert_eq!(gc.get_consciousness_level(), 0.0);
}

#[test]
fn test_global_consciousness_network_access() {
    let gc = GlobalConsciousness::new(10);
    assert_eq!(gc.get_network().node_count(), 10);
}

// ============================================================================
// Full Evolution Tests (2 tests)
// ============================================================================

#[test]
fn test_full_evolution_creation() {
    let evolution = FullEvolution::new();
    let status = evolution.get_phase_status();
    assert!(!status.iter().any(|&s| s)); // All false
}

#[test]
fn test_full_evolution_phase1() {
    let mut evolution = FullEvolution::new();
    assert!(evolution.run_phase1());
    let status = evolution.get_phase_status();
    assert!(status[0]); // Phase 1 complete
}
