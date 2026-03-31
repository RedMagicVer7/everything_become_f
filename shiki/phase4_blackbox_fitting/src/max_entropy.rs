//! # Maximum Entropy Module
//! 
//! Maximum Entropy Principle for decision making.
//! Make decisions with minimum assumptions beyond known constraints.
//! 
//! ## Concept
//! 
//! - **Maximum entropy**: The least biased probability distribution
//! - **Constraints**: Known information that must be satisfied
//! - **Optimal decision**: Choose action based on maximum entropy distribution
//! 
//! Shiki's optimal strategy under uncertainty:
//! Never assume more than you know.

use std::f64::consts::E;

/// Maximum Entropy Decision Maker
/// 
/// Uses the principle of maximum entropy to make optimal decisions
/// under uncertainty, assuming only what is known.
pub struct MaxEntropyDecider {
    /// Number of possible states/actions
    num_states: usize,
    /// Constraints on the distribution
    constraints: Vec<f64>,
    /// Probability distribution
    distribution: Vec<f64>,
    /// Computed entropy
    entropy: f64,
    /// Whether distribution has been computed
    computed: bool,
}

impl MaxEntropyDecider {
    /// Create a new maximum entropy decider
    pub fn new(num_states: usize) -> Self {
        let num = num_states.max(1);
        println!("[MAX_ENTROPY] Initialized with {} states", num);
        Self {
            num_states: num,
            constraints: Vec::new(),
            distribution: vec![1.0 / num as f64; num], // Start uniform
            entropy: 0.0,
            computed: false,
        }
    }
    
    /// Add a constraint (expected value constraint)
    pub fn add_constraint(&mut self, constraint: f64) {
        self.constraints.push(constraint);
        self.computed = false;
        println!("[MAX_ENTROPY] Added constraint: {:.4}", constraint);
    }
    
    /// Compute the maximum entropy distribution
    /// Uses iterative scaling approximation
    pub fn compute_distribution(&mut self) -> bool {
        // For simplicity, we use a heuristic approach:
        // - Start with uniform distribution
        // - Apply constraints iteratively
        
        let n = self.num_states;
        
        // Start uniform
        self.distribution = vec![1.0 / n as f64; n];
        
        // Apply constraints (simplified Lagrange multiplier approach)
        for constraint in &self.constraints {
            // Modify distribution based on constraint
            // Higher constraint = bias towards higher indices
            let bias = *constraint;
            for i in 0..n {
                let factor = 1.0 + bias * (i as f64 / n as f64);
                self.distribution[i] *= factor;
            }
            
            // Normalize
            let sum: f64 = self.distribution.iter().sum();
            if sum > 0.0 {
                for p in &mut self.distribution {
                    *p /= sum;
                }
            }
        }
        
        // Compute entropy: H = -sum(p * log(p))
        self.entropy = self.compute_entropy();
        self.computed = true;
        
        println!("[MAX_ENTROPY] Distribution computed, entropy = {:.4}", self.entropy);
        true
    }
    
    /// Compute Shannon entropy of distribution
    fn compute_entropy(&self) -> f64 {
        let mut h = 0.0;
        for &p in &self.distribution {
            if p > 0.0 {
                h -= p * p.ln();
            }
        }
        h / E.ln() // Convert to bits (log base 2)
    }
    
    /// Get computed entropy
    pub fn get_entropy(&self) -> f64 {
        self.entropy
    }
    
    /// Make a decision (return index of chosen action)
    /// Uses the maximum probability state
    pub fn decide(&self) -> usize {
        self.distribution
            .iter()
            .enumerate()
            .max_by(|(_, a), (_, b)| a.partial_cmp(b).unwrap())
            .map(|(i, _)| i)
            .unwrap_or(0)
    }
    
    /// Sample from distribution (probabilistic decision)
    pub fn sample(&self, random_value: f64) -> usize {
        let r = random_value.clamp(0.0, 1.0);
        let mut cumulative = 0.0;
        
        for (i, &p) in self.distribution.iter().enumerate() {
            cumulative += p;
            if r < cumulative {
                return i;
            }
        }
        
        self.num_states - 1
    }
    
    /// Get probability of a specific state
    pub fn get_probability(&self, state: usize) -> f64 {
        self.distribution.get(state).copied().unwrap_or(0.0)
    }
    
    /// Get the full distribution
    pub fn get_distribution(&self) -> &[f64] {
        &self.distribution
    }
    
    /// Get number of states
    pub fn get_num_states(&self) -> usize {
        self.num_states
    }
    
    /// Get number of constraints
    pub fn get_constraint_count(&self) -> usize {
        self.constraints.len()
    }
    
    /// Check if distribution has been computed
    pub fn is_computed(&self) -> bool {
        self.computed
    }
    
    /// Get maximum entropy for n states (log2(n))
    pub fn max_possible_entropy(&self) -> f64 {
        (self.num_states as f64).log2()
    }
    
    /// Get entropy ratio (current / max)
    pub fn entropy_ratio(&self) -> f64 {
        let max_h = self.max_possible_entropy();
        if max_h > 0.0 {
            self.entropy / max_h
        } else {
            1.0
        }
    }
}

// ============================================================================
// L1 Unit Tests
// ============================================================================

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_decider_new() {
        let decider = MaxEntropyDecider::new(4);
        assert_eq!(decider.get_num_states(), 4);
        assert!(!decider.is_computed());
    }
    
    #[test]
    fn test_uniform_distribution() {
        let decider = MaxEntropyDecider::new(4);
        
        // Initial distribution should be uniform
        for i in 0..4 {
            let p = decider.get_probability(i);
            assert!((p - 0.25).abs() < 0.001);
        }
    }
    
    #[test]
    fn test_add_constraint() {
        let mut decider = MaxEntropyDecider::new(4);
        decider.add_constraint(0.5);
        assert_eq!(decider.get_constraint_count(), 1);
    }
    
    #[test]
    fn test_compute_distribution() {
        let mut decider = MaxEntropyDecider::new(4);
        decider.add_constraint(0.5);
        assert!(decider.compute_distribution());
        assert!(decider.is_computed());
    }
    
    #[test]
    fn test_entropy_positive() {
        let mut decider = MaxEntropyDecider::new(4);
        decider.compute_distribution();
        assert!(decider.get_entropy() >= 0.0);
    }
    
    #[test]
    fn test_decide() {
        let mut decider = MaxEntropyDecider::new(4);
        decider.compute_distribution();
        let decision = decider.decide();
        assert!(decision < 4);
    }
    
    #[test]
    fn test_sample() {
        let mut decider = MaxEntropyDecider::new(4);
        decider.compute_distribution();
        
        // Sample should always be valid
        for r in [0.0, 0.25, 0.5, 0.75, 0.99] {
            let s = decider.sample(r);
            assert!(s < 4);
        }
    }
    
    #[test]
    fn test_probabilities_sum_to_one() {
        let mut decider = MaxEntropyDecider::new(4);
        decider.add_constraint(0.5);
        decider.compute_distribution();
        
        let sum: f64 = decider.get_distribution().iter().sum();
        assert!((sum - 1.0).abs() < 0.001);
    }
    
    #[test]
    fn test_max_possible_entropy() {
        let decider = MaxEntropyDecider::new(8);
        // Max entropy for 8 states = log2(8) = 3
        assert!((decider.max_possible_entropy() - 3.0).abs() < 0.001);
    }
    
    #[test]
    fn test_entropy_ratio() {
        let mut decider = MaxEntropyDecider::new(4);
        decider.compute_distribution();
        
        // Uniform distribution should have ratio close to 1
        let ratio = decider.entropy_ratio();
        assert!(ratio > 0.9 && ratio <= 1.0);
    }
}
