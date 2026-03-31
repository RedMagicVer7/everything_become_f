//! # GAN Simulator Module
//! 
//! Generative Adversarial Network simulation.
//! Two networks in adversarial training:
//! - Generator: creates fake data (Shiki creating false identities)
//! - Discriminator: detects fakes (the world trying to find Shiki)
//! 
//! ## Concept
//! 
//! The adversarial relationship mirrors Shiki vs. the outside world:
//! - She generates convincing facades
//! - The world tries to see through them
//! - Both improve through competition

/// Generator network - creates synthetic data
/// Like Shiki creating false identities
#[derive(Debug, Clone)]
pub struct Generator {
    /// Quality of generated output (0.0 - 1.0)
    quality: f64,
    /// Number of generations performed
    generations: u32,
    /// Learning rate
    learning_rate: f64,
}

impl Generator {
    /// Create a new generator with initial quality
    pub fn new(initial_quality: f64) -> Self {
        Self {
            quality: initial_quality.clamp(0.0, 1.0),
            generations: 0,
            learning_rate: 0.01,
        }
    }
    
    /// Generate a sample
    pub fn generate(&mut self) -> f64 {
        self.generations += 1;
        // Return current quality as the "authenticity" of generated data
        self.quality
    }
    
    /// Improve based on discriminator feedback
    pub fn improve(&mut self, feedback: f64) {
        // Negative feedback means discriminator detected fake
        // Generator improves to fool discriminator
        let improvement = feedback * self.learning_rate;
        self.quality = (self.quality + improvement).clamp(0.0, 1.0);
    }
    
    /// Get current quality
    pub fn get_quality(&self) -> f64 {
        self.quality
    }
    
    /// Get generation count
    pub fn get_generations(&self) -> u32 {
        self.generations
    }
}

/// Discriminator network - detects fakes
/// Like the world trying to find Shiki
#[derive(Debug, Clone)]
pub struct Discriminator {
    /// Detection accuracy (0.0 - 1.0)
    accuracy: f64,
    /// Number of discriminations performed
    discriminations: u32,
    /// Learning rate
    learning_rate: f64,
}

impl Discriminator {
    /// Create a new discriminator with initial accuracy
    pub fn new(initial_accuracy: f64) -> Self {
        Self {
            accuracy: initial_accuracy.clamp(0.0, 1.0),
            discriminations: 0,
            learning_rate: 0.01,
        }
    }
    
    /// Discriminate a sample (returns true if detected as fake)
    pub fn discriminate(&mut self, sample_quality: f64) -> bool {
        self.discriminations += 1;
        // Higher accuracy = better at detecting fakes
        // Lower sample quality = easier to detect
        sample_quality < self.accuracy
    }
    
    /// Improve based on correct/incorrect detection
    pub fn improve(&mut self, was_correct: bool) {
        if was_correct {
            // Got it right - small improvement
            self.accuracy = (self.accuracy + self.learning_rate * 0.5).clamp(0.0, 1.0);
        } else {
            // Got it wrong - learn from mistake
            self.accuracy = (self.accuracy + self.learning_rate).clamp(0.0, 1.0);
        }
    }
    
    /// Get current accuracy
    pub fn get_accuracy(&self) -> f64 {
        self.accuracy
    }
    
    /// Get discrimination count
    pub fn get_discriminations(&self) -> u32 {
        self.discriminations
    }
}

/// GAN Simulator - orchestrates adversarial training
pub struct GANSimulator {
    generator: Generator,
    discriminator: Discriminator,
    epoch: u32,
    converged: bool,
    convergence_threshold: f64,
    log: Vec<String>,
}

impl GANSimulator {
    /// Create a new GAN simulator
    pub fn new() -> Self {
        println!("[GAN] GAN Simulator initialized");
        Self {
            generator: Generator::new(0.1),     // Start with low quality
            discriminator: Discriminator::new(0.5), // Start with 50% accuracy
            epoch: 0,
            converged: false,
            convergence_threshold: 0.1, // Converge when |G - D| < threshold
            log: Vec::new(),
        }
    }
    
    /// Perform one training step
    pub fn train_step(&mut self) -> bool {
        self.epoch += 1;
        
        // Generator generates
        let sample = self.generator.generate();
        
        // Discriminator tries to detect
        let detected = self.discriminator.discriminate(sample);
        
        // Both learn from the outcome
        if detected {
            // Discriminator won - generator needs to improve more
            self.generator.improve(0.1);
            self.discriminator.improve(true);
        } else {
            // Generator fooled discriminator
            self.generator.improve(0.05);
            self.discriminator.improve(false);
        }
        
        // Check convergence (Nash equilibrium-like state)
        let diff = (self.generator.get_quality() - self.discriminator.get_accuracy()).abs();
        if diff < self.convergence_threshold && self.generator.get_quality() > 0.4 {
            self.converged = true;
        }
        
        true
    }
    
    /// Run training for specified epochs
    pub fn run_training(&mut self, epochs: u32) -> bool {
        self.log(&format!("Starting training for {} epochs", epochs));
        
        for _ in 0..epochs {
            self.train_step();
            
            if self.converged {
                self.log(&format!(
                    "Converged at epoch {}! G={:.3}, D={:.3}",
                    self.epoch,
                    self.generator.get_quality(),
                    self.discriminator.get_accuracy()
                ));
                break;
            }
        }
        
        if !self.converged {
            self.log(&format!(
                "Training complete. G={:.3}, D={:.3}",
                self.generator.get_quality(),
                self.discriminator.get_accuracy()
            ));
        }
        
        self.converged
    }
    
    /// Check if GAN has converged
    pub fn is_converged(&self) -> bool {
        self.converged
    }
    
    /// Get current epoch
    pub fn get_epoch(&self) -> u32 {
        self.epoch
    }
    
    /// Get generator
    pub fn get_generator(&self) -> &Generator {
        &self.generator
    }
    
    /// Get discriminator
    pub fn get_discriminator(&self) -> &Discriminator {
        &self.discriminator
    }
    
    /// Log message
    fn log(&mut self, message: &str) {
        self.log.push(message.to_string());
        println!("[GAN] {}", message);
    }
    
    /// Get log
    pub fn get_log(&self) -> &[String] {
        &self.log
    }
}

impl Default for GANSimulator {
    fn default() -> Self {
        Self::new()
    }
}

// ============================================================================
// L1 Unit Tests
// ============================================================================

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_generator_new() {
        let gen = Generator::new(0.5);
        assert_eq!(gen.get_quality(), 0.5);
        assert_eq!(gen.get_generations(), 0);
    }
    
    #[test]
    fn test_generator_generate() {
        let mut gen = Generator::new(0.5);
        let sample = gen.generate();
        assert_eq!(sample, 0.5);
        assert_eq!(gen.get_generations(), 1);
    }
    
    #[test]
    fn test_generator_improve() {
        let mut gen = Generator::new(0.5);
        gen.improve(1.0);
        assert!(gen.get_quality() > 0.5);
    }
    
    #[test]
    fn test_discriminator_new() {
        let disc = Discriminator::new(0.5);
        assert_eq!(disc.get_accuracy(), 0.5);
        assert_eq!(disc.get_discriminations(), 0);
    }
    
    #[test]
    fn test_discriminator_discriminate() {
        let mut disc = Discriminator::new(0.5);
        
        // Low quality should be detected
        let detected = disc.discriminate(0.3);
        assert!(detected);
        
        // High quality should pass
        let detected2 = disc.discriminate(0.8);
        assert!(!detected2);
    }
    
    #[test]
    fn test_gan_new() {
        let gan = GANSimulator::new();
        assert_eq!(gan.get_epoch(), 0);
        assert!(!gan.is_converged());
    }
    
    #[test]
    fn test_gan_train_step() {
        let mut gan = GANSimulator::new();
        gan.train_step();
        assert_eq!(gan.get_epoch(), 1);
    }
    
    #[test]
    fn test_gan_run_training() {
        let mut gan = GANSimulator::new();
        gan.run_training(50);
        assert!(gan.get_epoch() >= 50 || gan.is_converged());
    }
    
    #[test]
    fn test_gan_convergence() {
        let mut gan = GANSimulator::new();
        gan.run_training(200);
        // After enough training, should converge
        assert!(gan.is_converged() || gan.get_generator().get_quality() > 0.3);
    }
    
    #[test]
    fn test_generator_quality_clamped() {
        let gen = Generator::new(2.0);
        assert_eq!(gen.get_quality(), 1.0);
        
        let gen2 = Generator::new(-1.0);
        assert_eq!(gen2.get_quality(), 0.0);
    }
}
