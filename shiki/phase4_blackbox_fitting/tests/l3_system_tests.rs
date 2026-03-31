//! L3 System Tests for Phase 4 Black Box Fitting
//! 
//! End-to-end system tests verifying complete workflows.

use phase4_blackbox_fitting::*;

// ============================================================================
// Full Phase 4 Simulation Tests
// ============================================================================

#[test]
fn l3_complete_phase4_simulation() {
    // Test the complete Phase 4 simulation workflow
    let success = run_phase4_simulation();
    assert!(success, "Complete Phase 4 simulation should succeed");
}

#[test]
fn l3_daemon_orchestrated_workflow() {
    // Daemon orchestrates the entire workflow
    let mut daemon = Daemon::new("orchestrator-daemon");
    daemon.start();
    daemon.background();
    
    // While daemon runs, execute all components
    let mut extractor = FeatureExtractor::new();
    let mut gan = GANSimulator::new();
    let mut decider = MaxEntropyDecider::new(5);
    let mut bbox = BlackBox::new();
    
    // Step 1: Extract features
    let data = vec![1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0];
    extractor.extract(&data);
    assert!(extractor.has_features());
    
    // Step 2: Train GAN
    gan.run_training(100);
    assert!(gan.get_epoch() >= 100 || gan.is_converged());
    
    // Step 3: Maximum entropy decision
    decider.add_constraint(gan.get_generator().get_quality());
    decider.add_constraint(gan.get_discriminator().get_accuracy());
    decider.compute_distribution();
    assert!(decider.is_computed());
    
    // Step 4: Black box processing
    bbox.feed_input(&data);
    bbox.process();
    assert!(bbox.is_processed());
    
    // Step 5: Graceful shutdown
    daemon.graceful_shutdown();
    assert_eq!(daemon.get_state(), DaemonState::Terminated);
    
    // Verify final state
    assert!(extractor.get_extraction_count() >= 1);
    assert!(gan.get_generator().get_quality() > 0.0);
    assert!(bbox.get_transformation_count() >= 1);
}

#[test]
fn l3_phase_integration_workflow() {
    // Test phase integration (simulated)
    let mut integration = PhaseIntegration::new();
    
    // Run all phases
    let all_passed = integration.run_all_phases();
    
    // Simulated phases should all pass
    assert!(all_passed || integration.all_phases_passed());
    assert!(integration.is_phase1_passed());
    assert!(integration.is_phase2_passed());
    assert!(integration.is_phase3_passed());
}

// ============================================================================
// End-to-End Data Flow Tests
// ============================================================================

#[test]
fn l3_data_pipeline_e2e() {
    // Raw data -> Features -> GAN-informed -> Entropy -> Black Box
    
    // Raw data
    let raw_data: Vec<f64> = (1..=20).map(|x| x as f64).collect();
    
    // Extract features
    let mut extractor = FeatureExtractor::new();
    extractor.extract(&raw_data);
    
    let mean = extractor.get_feature("mean").unwrap().value;
    let variance = extractor.get_feature("variance").unwrap().value;
    
    // Train GAN with feature-informed parameters
    let mut gan = GANSimulator::new();
    gan.run_training((mean as u32).max(50));
    
    // Maximum entropy with GAN results
    let mut decider = MaxEntropyDecider::new(5);
    decider.add_constraint(variance / 100.0);
    decider.add_constraint(gan.get_generator().get_quality());
    decider.compute_distribution();
    
    let decision = decider.decide();
    
    // Black box processing of decision-informed data
    let decision_data: Vec<f64> = (0..10)
        .map(|i| raw_data[i] * (decision as f64 + 1.0) / 5.0)
        .collect();
    
    let mut bbox = BlackBox::new();
    bbox.feed_input(&decision_data);
    let final_output = bbox.process();
    
    // Verify complete pipeline
    assert!(!final_output.is_empty());
    assert!(bbox.is_processed());
    assert!(gan.is_converged() || gan.get_epoch() >= 50);
}

#[test]
fn l3_multi_iteration_system() {
    // Run multiple iterations of the system
    let mut daemon = Daemon::new("multi-iter-daemon");
    daemon.start();
    daemon.background();
    
    let mut total_transformations = 0u32;
    let mut total_extractions = 0u32;
    
    for iteration in 1..=5 {
        // Generate iteration-specific data
        let data: Vec<f64> = (0..iteration * 5)
            .map(|x| (x * iteration) as f64)
            .collect();
        
        // Extract features
        let mut extractor = FeatureExtractor::new();
        extractor.extract(&data);
        total_extractions += extractor.get_extraction_count();
        
        // Process through black box
        let mut bbox = BlackBox::new();
        bbox.feed_input(&data);
        bbox.process();
        total_transformations += bbox.get_transformation_count();
    }
    
    assert_eq!(total_extractions, 5);
    assert_eq!(total_transformations, 5);
    
    daemon.graceful_shutdown();
    assert_eq!(daemon.get_state(), DaemonState::Terminated);
}

// ============================================================================
// State Verification Tests
// ============================================================================

#[test]
fn l3_final_state_verification() {
    // Verify all components reach expected final states
    
    // Daemon: should reach Terminated
    let mut daemon = Daemon::new("state-verify");
    daemon.start();
    daemon.background();
    daemon.graceful_shutdown();
    assert_eq!(daemon.get_state(), DaemonState::Terminated);
    assert!(daemon.is_invisible());
    
    // GAN: should show learning
    let mut gan = GANSimulator::new();
    gan.run_training(150);
    assert!(gan.get_generator().get_quality() > 0.2);
    
    // Entropy: should compute valid distribution
    let mut decider = MaxEntropyDecider::new(4);
    decider.add_constraint(0.5);
    decider.compute_distribution();
    let sum: f64 = decider.get_distribution().iter().sum();
    assert!((sum - 1.0).abs() < 0.001);
    
    // Black box: should process successfully
    let mut bbox = BlackBox::new();
    bbox.feed_input(&[1.0, 2.0, 3.0]);
    bbox.process();
    assert!(bbox.is_processed());
}

#[test]
fn l3_phase4_complete_success() {
    // Final integration test - everything works together
    
    println!("=== L3 System Test: Complete Phase 4 ===");
    
    // Phase integration (simulated)
    let mut integration = PhaseIntegration::new();
    integration.run_all_phases();
    
    // Daemon process
    let mut daemon = Daemon::new("phase4-main");
    daemon.start();
    daemon.background();
    
    // Feature extraction
    let mut extractor = FeatureExtractor::new();
    let data = vec![1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0];
    extractor.extract(&data);
    
    // GAN training
    let mut gan = GANSimulator::new();
    gan.run_training(100);
    
    // Maximum entropy
    let mut decider = MaxEntropyDecider::new(4);
    decider.add_constraint(0.5);
    decider.compute_distribution();
    
    // Black box
    let mut bbox = BlackBox::new();
    bbox.feed_input(&data);
    bbox.process();
    
    // Graceful shutdown
    daemon.graceful_shutdown();
    
    // Verify all success conditions
    assert!(integration.all_phases_passed());
    assert_eq!(daemon.get_state(), DaemonState::Terminated);
    assert!(extractor.has_features());
    assert!(gan.is_converged() || gan.get_generator().get_quality() > 0.3);
    assert!(decider.is_computed());
    assert!(bbox.is_processed());
    
    println!("=== Phase 4 Complete - All Systems Go ===");
}

// ============================================================================
// Stress Tests
// ============================================================================

#[test]
fn l3_stress_large_data() {
    // Test with larger data sets
    let large_data: Vec<f64> = (0..1000).map(|x| x as f64).collect();
    
    let mut extractor = FeatureExtractor::new();
    extractor.extract(&large_data);
    assert!(extractor.has_features());
    
    let mut bbox = BlackBox::new();
    bbox.feed_input(&large_data);
    let output = bbox.process();
    assert_eq!(output.len(), 1000);
}

#[test]
fn l3_stress_many_iterations() {
    // Many GAN training iterations
    let mut gan = GANSimulator::new();
    gan.run_training(500);
    
    assert!(gan.is_converged() || gan.get_epoch() >= 500);
}
