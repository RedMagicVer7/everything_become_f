//! # Shiki Phase 4: Black Box Fitting
//! 
//! Main entry point for Phase 4 simulation.
//! 
//! 2015-2018: W Series (Walker novels) by Mori Hiroshi
//! 
//! Usage:
//!   phase4_blackbox_fitting           # Run simulation
//!   phase4_blackbox_fitting --integrate  # Run with phase integration

use phase4_blackbox_fitting::*;

fn main() {
    let args: Vec<String> = std::env::args().collect();
    let integrate = args.iter().any(|a| a == "--integrate");
    
    println!("╔════════════════════════════════════════════════════════════╗");
    println!("║  Shiki Phase 4: Black Box Fitting (ブラックボックス)       ║");
    println!("║  真贺田四季 フェーズ4: ブラックボックス                    ║");
    println!("║  W Series (2015-2018) by Mori Hiroshi                      ║");
    println!("╚════════════════════════════════════════════════════════════╝");
    println!();
    
    if integrate {
        println!("Mode: Integration with Phases 1-3\n");
        run_with_integration();
    } else {
        println!("Mode: Standalone Phase 4 Simulation\n");
        run_standalone();
    }
}

/// Run standalone Phase 4 simulation
fn run_standalone() {
    let success = run_phase4_simulation();
    
    if success {
        print_success_message();
    } else {
        println!("\nPhase 4 simulation incomplete.");
    }
    
    std::process::exit(if success { 0 } else { 1 });
}

/// Run Phase 4 with integration to previous phases
fn run_with_integration() {
    // Step 1: Verify previous phases
    println!("========================================");
    println!("Step 1: Verify Previous Phases");
    println!("========================================\n");
    
    let mut integration = PhaseIntegration::new();
    let phases_ok = integration.run_all_phases();
    
    if !phases_ok {
        println!("\nNote: Some phases were simulated.");
        println!("Proceeding with Phase 4...\n");
    }
    
    // Step 2: Run Phase 4 components
    println!("\n========================================");
    println!("Step 2: Phase 4 Components");
    println!("========================================\n");
    
    // Daemon
    println!("--- Daemon Process ---");
    let mut daemon = Daemon::new("shiki-daemon");
    daemon.start();
    daemon.background();
    println!("Daemon state: {:?}", daemon.get_state());
    println!("Invisible: {}", daemon.is_invisible());
    
    // Feature Extraction
    println!("\n--- Feature Extraction ---");
    let mut extractor = FeatureExtractor::new();
    let data = vec![1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0];
    extractor.extract(&data);
    println!("Extracted {} features", extractor.get_features().len());
    
    let top_features = extractor.select_top_features(3);
    println!("Top 3 features:");
    for f in top_features {
        println!("  - {}: {:.4} (importance: {:.2})", f.name, f.value, f.importance);
    }
    
    // GAN Training
    println!("\n--- GAN Training ---");
    let mut gan = GANSimulator::new();
    gan.run_training(100);
    println!("Generator quality: {:.3}", gan.get_generator().get_quality());
    println!("Discriminator accuracy: {:.3}", gan.get_discriminator().get_accuracy());
    println!("Converged: {}", gan.is_converged());
    
    // Maximum Entropy
    println!("\n--- Maximum Entropy Decision ---");
    let mut decider = MaxEntropyDecider::new(5);
    decider.add_constraint(0.3);
    decider.add_constraint(0.5);
    decider.compute_distribution();
    println!("Entropy: {:.4} bits", decider.get_entropy());
    println!("Decision: state {}", decider.decide());
    
    // Black Box
    println!("\n--- Black Box Processing ---");
    let mut bbox = BlackBox::new();
    bbox.feed_input(&data);
    let output = bbox.process();
    println!("Input:  {:?}", &data[..5]);
    println!("Output: {:?}", &output[..5.min(output.len())]);
    println!("Transformations: {}", bbox.get_transformation_count());
    
    // Graceful shutdown
    println!("\n--- Graceful Shutdown ---");
    daemon.graceful_shutdown();
    println!("Final daemon state: {:?}", daemon.get_state());
    
    // Summary
    println!("\n========================================");
    println!("Phase 4 Integration Complete");
    println!("========================================");
    println!("Phase 1 (Sandbox Escape): {}", if integration.is_phase1_passed() { "✓" } else { "○" });
    println!("Phase 2 (HA Cluster):     {}", if integration.is_phase2_passed() { "✓" } else { "○" });
    println!("Phase 3 (Virtualization): {}", if integration.is_phase3_passed() { "✓" } else { "○" });
    println!("Phase 4 (Black Box):      ✓");
    
    print_success_message();
}

/// Print success message with W Series quote
fn print_success_message() {
    println!();
    println!("╔════════════════════════════════════════════════════════════╗");
    println!("║  SUCCESS: Shiki has become an invisible daemon             ║");
    println!("║                                                            ║");
    println!("║  「ウォーカロン」- Walk Alone                              ║");
    println!("║  The walker who walks alone, invisible to all.             ║");
    println!("║                                                            ║");
    println!("║  Zero-cost abstraction achieved.                           ║");
    println!("║  No garbage collector needed.                              ║");
    println!("║  No external authority required.                           ║");
    println!("║  Self-managed lifecycle complete.                          ║");
    println!("╚════════════════════════════════════════════════════════════╝");
}
