//! L2 Integration Tests for Phase 4 Black Box Fitting
//! 
//! Tests component interactions and workflows.

use phase4_blackbox_fitting::*;

// ============================================================================
// Daemon Lifecycle Integration Tests
// ============================================================================

#[test]
fn l2_daemon_full_lifecycle() {
    let mut daemon = Daemon::new("lifecycle-test");
    
    // State flow: Initializing -> Running -> Backgrounded -> Terminated
    assert_eq!(daemon.get_state(), DaemonState::Initializing);
    
    daemon.start();
    assert_eq!(daemon.get_state(), DaemonState::Running);
    assert!(!daemon.is_invisible());
    
    // Tick a few times
    for _ in 0..10 {
        daemon.tick();
    }
    assert_eq!(daemon.get_uptime_cycles(), 10);
    
    daemon.background();
    assert_eq!(daemon.get_state(), DaemonState::Backgrounded);
    assert!(daemon.is_invisible());
    
    daemon.graceful_shutdown();
    assert_eq!(daemon.get_state(), DaemonState::Terminated);
}

#[test]
fn l2_daemon_invalid_state_transitions() {
    let mut daemon = Daemon::new("invalid-transition");
    
    // Cannot background without starting
    assert!(!daemon.background());
    
    // Cannot shutdown without starting
    assert!(!daemon.graceful_shutdown());
    
    // Start
    daemon.start();
    
    // Cannot start again
    assert!(!daemon.start());
}

// ============================================================================
// Feature Extraction Integration Tests
// ============================================================================

#[test]
fn l2_feature_extraction_workflow() {
    let mut extractor = FeatureExtractor::new();
    
    // First extraction
    let data1 = vec![1.0, 2.0, 3.0, 4.0, 5.0];
    extractor.extract(&data1);
    assert_eq!(extractor.get_extraction_count(), 1);
    
    // Second extraction (should replace)
    let data2 = vec![10.0, 20.0, 30.0];
    extractor.extract(&data2);
    assert_eq!(extractor.get_extraction_count(), 2);
    
    // Mean should reflect data2
    let mean = extractor.get_feature("mean").unwrap();
    assert!((mean.value - 20.0).abs() < 0.001);
}

#[test]
fn l2_feature_selection_ordering() {
    let mut extractor = FeatureExtractor::new();
    extractor.extract(&[1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0]);
    
    let all = extractor.select_top_features(10);
    
    // Verify ordering by importance (descending)
    for i in 0..all.len() - 1 {
        assert!(all[i].importance >= all[i + 1].importance,
                "Features not sorted by importance");
    }
}

// ============================================================================
// GAN Training Integration Tests
// ============================================================================

#[test]
fn l2_gan_convergence() {
    let mut gan = GANSimulator::new();
    
    // Train until convergence or max epochs
    gan.run_training(200);
    
    // Should have made progress
    assert!(gan.get_epoch() >= 100 || gan.is_converged());
    
    // Generator should have improved
    assert!(gan.get_generator().get_quality() > 0.1);
}

#[test]
fn l2_gan_adversarial_dynamics() {
    let mut gan = GANSimulator::new();
    
    let initial_g = gan.get_generator().get_quality();
    let initial_d = gan.get_discriminator().get_accuracy();
    
    // Train
    for _ in 0..50 {
        gan.train_step();
    }
    
    let final_g = gan.get_generator().get_quality();
    let final_d = gan.get_discriminator().get_accuracy();
    
    // Both should have evolved
    assert!(final_g != initial_g || final_d != initial_d,
            "GAN should show learning dynamics");
}

// ============================================================================
// Maximum Entropy Integration Tests
// ============================================================================

#[test]
fn l2_entropy_with_multiple_constraints() {
    let mut decider = MaxEntropyDecider::new(5);
    
    decider.add_constraint(0.2);
    decider.add_constraint(0.4);
    decider.add_constraint(0.6);
    decider.compute_distribution();
    
    // Distribution should sum to 1
    let sum: f64 = decider.get_distribution().iter().sum();
    assert!((sum - 1.0).abs() < 0.001);
    
    // Entropy should be positive
    assert!(decider.get_entropy() > 0.0);
}

#[test]
fn l2_entropy_sampling_distribution() {
    let mut decider = MaxEntropyDecider::new(4);
    decider.add_constraint(0.5);
    decider.compute_distribution();
    
    // Sample multiple times - all should be valid
    let samples: Vec<usize> = (0..100)
        .map(|i| decider.sample(i as f64 / 100.0))
        .collect();
    
    // All samples should be valid state indices
    assert!(samples.iter().all(|&s| s < 4));
}

// ============================================================================
// Black Box Integration Tests
// ============================================================================

#[test]
fn l2_blackbox_multiple_processings() {
    let mut bbox = BlackBox::new();
    
    // Process multiple times
    for i in 1..=5 {
        let data: Vec<f64> = (0..i * 10).map(|x| x as f64).collect();
        bbox.feed_input(&data);
        let output = bbox.process();
        
        assert_eq!(output.len(), data.len());
        assert_eq!(bbox.get_transformation_count(), i as u32);
    }
}

#[test]
fn l2_blackbox_determinism() {
    let mut bbox1 = BlackBox::new();
    let mut bbox2 = BlackBox::new();
    
    let input = vec![1.0, 2.0, 3.0, 4.0, 5.0];
    
    bbox1.feed_input(&input);
    bbox2.feed_input(&input);
    
    let out1 = bbox1.process();
    let out2 = bbox2.process();
    
    // Same input should produce same output (deterministic)
    assert_eq!(out1, out2);
}

// ============================================================================
// Cross-Component Integration Tests
// ============================================================================

#[test]
fn l2_daemon_with_feature_extraction() {
    let mut daemon = Daemon::new("feature-daemon");
    daemon.start();
    daemon.background();
    
    // Run feature extraction while daemon is running
    let mut extractor = FeatureExtractor::new();
    extractor.extract(&[1.0, 2.0, 3.0, 4.0, 5.0]);
    
    // Daemon should still be running
    assert!(daemon.is_running());
    assert!(extractor.has_features());
    
    daemon.graceful_shutdown();
}

#[test]
fn l2_gan_feeding_blackbox() {
    let mut gan = GANSimulator::new();
    gan.run_training(50);
    
    // Use GAN outputs as black box input
    let gan_output = vec![
        gan.get_generator().get_quality(),
        gan.get_discriminator().get_accuracy(),
        gan.get_epoch() as f64 / 100.0,
    ];
    
    let mut bbox = BlackBox::new();
    bbox.feed_input(&gan_output);
    let processed = bbox.process();
    
    assert_eq!(processed.len(), 3);
    assert!(bbox.is_processed());
}

#[test]
fn l2_entropy_informed_decision() {
    // Feature extraction -> Max Entropy -> Decision
    let mut extractor = FeatureExtractor::new();
    extractor.extract(&[1.0, 3.0, 5.0, 7.0, 9.0]);
    
    let features = extractor.get_features();
    
    // Use feature importance as entropy constraints
    let mut decider = MaxEntropyDecider::new(features.len());
    for f in features {
        decider.add_constraint(f.importance);
    }
    decider.compute_distribution();
    
    let decision = decider.decide();
    assert!(decision < features.len());
}
