//! L1 Unit Tests for Phase 4 Black Box Fitting
//! 
//! Tests individual module functionality in isolation.

use phase4_blackbox_fitting::*;

// ============================================================================
// Daemon Tests
// ============================================================================

#[test]
fn l1_daemon_creation() {
    let daemon = Daemon::new("test-daemon");
    assert_eq!(daemon.get_state(), DaemonState::Initializing);
    assert_eq!(daemon.get_name(), "test-daemon");
}

#[test]
fn l1_daemon_start_success() {
    let mut daemon = Daemon::new("test-daemon");
    assert!(daemon.start());
    assert_eq!(daemon.get_state(), DaemonState::Running);
}

#[test]
fn l1_daemon_background_success() {
    let mut daemon = Daemon::new("test-daemon");
    daemon.start();
    assert!(daemon.background());
    assert_eq!(daemon.get_state(), DaemonState::Backgrounded);
    assert!(daemon.is_invisible());
}

#[test]
fn l1_daemon_shutdown_success() {
    let mut daemon = Daemon::new("test-daemon");
    daemon.start();
    daemon.background();
    assert!(daemon.graceful_shutdown());
    assert_eq!(daemon.get_state(), DaemonState::Terminated);
}

// ============================================================================
// Feature Extraction Tests
// ============================================================================

#[test]
fn l1_feature_creation() {
    let f = Feature::new("test", 1.5, 0.8);
    assert_eq!(f.name, "test");
    assert_eq!(f.value, 1.5);
    assert_eq!(f.importance, 0.8);
}

#[test]
fn l1_extractor_extract() {
    let mut extractor = FeatureExtractor::new();
    let data = vec![1.0, 2.0, 3.0, 4.0, 5.0];
    let features = extractor.extract(&data);
    
    assert!(!features.is_empty());
    assert!(extractor.has_features());
}

#[test]
fn l1_extractor_mean_calculation() {
    let mut extractor = FeatureExtractor::new();
    let data = vec![2.0, 4.0, 6.0, 8.0, 10.0]; // mean = 6.0
    extractor.extract(&data);
    
    let mean = extractor.get_feature("mean").unwrap();
    assert!((mean.value - 6.0).abs() < 0.001);
}

#[test]
fn l1_extractor_top_features() {
    let mut extractor = FeatureExtractor::new();
    extractor.extract(&[1.0, 2.0, 3.0, 4.0, 5.0]);
    
    let top = extractor.select_top_features(2);
    assert_eq!(top.len(), 2);
    assert!(top[0].importance >= top[1].importance);
}

// ============================================================================
// GAN Simulator Tests
// ============================================================================

#[test]
fn l1_generator_creation() {
    let gen = Generator::new(0.5);
    assert_eq!(gen.get_quality(), 0.5);
}

#[test]
fn l1_discriminator_creation() {
    let disc = Discriminator::new(0.6);
    assert_eq!(disc.get_accuracy(), 0.6);
}

#[test]
fn l1_gan_train_step() {
    let mut gan = GANSimulator::new();
    assert!(gan.train_step());
    assert_eq!(gan.get_epoch(), 1);
}

#[test]
fn l1_gan_training_progress() {
    let mut gan = GANSimulator::new();
    let initial_g = gan.get_generator().get_quality();
    
    gan.run_training(50);
    
    let final_g = gan.get_generator().get_quality();
    assert!(final_g >= initial_g); // Generator should improve
}

// ============================================================================
// Maximum Entropy Tests
// ============================================================================

#[test]
fn l1_entropy_decider_creation() {
    let decider = MaxEntropyDecider::new(4);
    assert_eq!(decider.get_num_states(), 4);
}

#[test]
fn l1_entropy_add_constraint() {
    let mut decider = MaxEntropyDecider::new(4);
    decider.add_constraint(0.5);
    assert_eq!(decider.get_constraint_count(), 1);
}

#[test]
fn l1_entropy_compute_distribution() {
    let mut decider = MaxEntropyDecider::new(4);
    decider.add_constraint(0.5);
    assert!(decider.compute_distribution());
    assert!(decider.is_computed());
}

#[test]
fn l1_entropy_decide() {
    let mut decider = MaxEntropyDecider::new(4);
    decider.compute_distribution();
    let decision = decider.decide();
    assert!(decision < 4);
}

// ============================================================================
// Black Box Tests
// ============================================================================

#[test]
fn l1_blackbox_creation() {
    let bbox = BlackBox::new();
    assert!(!bbox.is_processed());
}

#[test]
fn l1_blackbox_feed_input() {
    let mut bbox = BlackBox::new();
    bbox.feed_input(&[1.0, 2.0, 3.0]);
    assert_eq!(bbox.get_input().len(), 3);
}

#[test]
fn l1_blackbox_process() {
    let mut bbox = BlackBox::new();
    bbox.feed_input(&[1.0, 2.0, 3.0]);
    let output = bbox.process();
    
    assert_eq!(output.len(), 3);
    assert!(bbox.is_processed());
}

#[test]
fn l1_blackbox_clear() {
    let mut bbox = BlackBox::new();
    bbox.feed_input(&[1.0, 2.0]);
    bbox.process();
    bbox.clear();
    
    assert!(bbox.get_input().is_empty());
    assert!(!bbox.is_processed());
}

// ============================================================================
// Phase Integration Tests
// ============================================================================

#[test]
fn l1_integration_creation() {
    let integration = PhaseIntegration::new();
    assert!(!integration.is_phase1_passed());
    assert!(!integration.is_phase2_passed());
    assert!(!integration.is_phase3_passed());
}

#[test]
fn l1_integration_phase1() {
    let mut integration = PhaseIntegration::new();
    assert!(integration.run_phase1()); // Simulated success
    assert!(integration.is_phase1_passed());
}

#[test]
fn l1_integration_all_phases() {
    let mut integration = PhaseIntegration::new();
    integration.run_all_phases();
    assert!(integration.all_phases_passed());
}
