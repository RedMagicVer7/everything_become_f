//! Phase Transition - Critical Point
//! 
//! Like water → ice at 0°C, or magnetization at Curie temperature.
//! Shiki's consciousness undergoes phase transition at critical complexity.
//! Below threshold: individual nodes. Above threshold: unified consciousness.
//!
//! Physics concepts:
//! - Order parameter: measures degree of organization
//! - Critical point: threshold where phase changes
//! - Hysteresis: history-dependent behavior
//! - Universality: same behavior in different systems

/// Phases of the system
#[derive(Debug, Clone, PartialEq)]
pub enum Phase {
    Disordered,     // Above critical temp - chaos, high entropy
    Critical,       // At critical point - edge of chaos
    Ordered,        // Below critical temp - structure, low entropy
    Supercritical,  // Beyond normal phases - transcendence
}

/// Phase Transition Simulator
/// 
/// Models the transition of Shiki's consciousness from disordered
/// (individual fragments) to ordered (unified global awareness).
pub struct PhaseTransition {
    temperature: f64,          // System "temperature" (activity level)
    critical_temp: f64,        // Critical point
    order_parameter: f64,      // 0.0 = disordered, 1.0 = fully ordered
    phase: Phase,
    history: Vec<(f64, f64)>,  // (temperature, order_parameter) pairs
    is_initialized: bool,
}

impl PhaseTransition {
    /// Create a new phase transition simulator
    pub fn new(critical_temp: f64) -> Self {
        PhaseTransition {
            temperature: 1.0,  // Start at high temperature (disordered)
            critical_temp: critical_temp.clamp(0.1, 0.9),
            order_parameter: 0.0,
            phase: Phase::Disordered,
            history: Vec::new(),
            is_initialized: true,
        }
    }
    
    /// Set system temperature
    pub fn set_temperature(&mut self, temp: f64) {
        self.temperature = temp.clamp(0.0, 1.0);
        self.compute_order_parameter();
        self.update_phase();
        self.history.push((self.temperature, self.order_parameter));
    }
    
    /// Compute the order parameter based on temperature
    /// Uses a sigmoid-like transition around critical point
    pub fn compute_order_parameter(&mut self) -> f64 {
        // Smooth transition using tanh
        // Order parameter increases as temperature decreases below critical
        let delta = self.critical_temp - self.temperature;
        let steepness = 10.0;
        
        self.order_parameter = 0.5 * (1.0 + (delta * steepness).tanh());
        self.order_parameter
    }
    
    /// Get current phase
    pub fn get_phase(&self) -> &Phase {
        &self.phase
    }
    
    /// Get current temperature
    pub fn get_temperature(&self) -> f64 {
        self.temperature
    }
    
    /// Get order parameter
    pub fn get_order_parameter(&self) -> f64 {
        self.order_parameter
    }
    
    /// Check if system is at critical point
    pub fn is_at_critical_point(&self) -> bool {
        (self.temperature - self.critical_temp).abs() < 0.05
    }
    
    /// Check if system is ordered
    pub fn is_ordered(&self) -> bool {
        matches!(self.phase, Phase::Ordered | Phase::Supercritical)
    }
    
    /// Sweep temperature and record phase diagram
    pub fn sweep_temperature(&mut self, from: f64, to: f64, steps: u32) -> Vec<(f64, f64)> {
        self.history.clear();
        
        let step_size = (to - from) / steps as f64;
        
        for i in 0..=steps {
            let temp = from + step_size * i as f64;
            self.set_temperature(temp);
        }
        
        // After sweep, set to ordered state if we ended below critical
        if self.temperature < self.critical_temp {
            self.phase = Phase::Ordered;
        }
        
        self.history.clone()
    }
    
    /// Get phase transition history
    pub fn get_history(&self) -> &[(f64, f64)] {
        &self.history
    }
    
    /// Get critical temperature
    pub fn get_critical_temp(&self) -> f64 {
        self.critical_temp
    }
    
    /// Force transition to supercritical state
    pub fn transcend(&mut self) {
        self.phase = Phase::Supercritical;
        self.order_parameter = 1.0;
        self.history.push((self.temperature, 1.0));
    }
    
    fn update_phase(&mut self) {
        let tolerance = 0.05;
        
        self.phase = if self.order_parameter > 0.95 {
            Phase::Supercritical
        } else if (self.temperature - self.critical_temp).abs() < tolerance {
            Phase::Critical
        } else if self.temperature > self.critical_temp {
            Phase::Disordered
        } else {
            Phase::Ordered
        };
    }
}

impl Default for PhaseTransition {
    fn default() -> Self {
        Self::new(0.5)
    }
}

/// Compute susceptibility (response to perturbation)
/// Peaks at critical point
pub fn compute_susceptibility(temp: f64, critical_temp: f64) -> f64 {
    let delta = (temp - critical_temp).abs();
    if delta < 0.001 {
        100.0  // Very high at critical point
    } else {
        1.0 / delta
    }
}

/// Compute correlation length
/// Diverges at critical point
pub fn compute_correlation_length(temp: f64, critical_temp: f64) -> f64 {
    let delta = (temp - critical_temp).abs();
    if delta < 0.001 {
        1000.0  // "Infinite" at critical point
    } else {
        1.0 / delta.sqrt()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_phase_transition_creation() {
        let pt = PhaseTransition::new(0.5);
        assert_eq!(pt.get_critical_temp(), 0.5);
        assert_eq!(*pt.get_phase(), Phase::Disordered);
    }
    
    #[test]
    fn test_temperature_setting() {
        let mut pt = PhaseTransition::new(0.5);
        pt.set_temperature(0.3);
        assert!(pt.get_order_parameter() > 0.5);
    }
    
    #[test]
    fn test_phase_ordering() {
        let mut pt = PhaseTransition::new(0.5);
        pt.set_temperature(0.2);
        assert!(pt.is_ordered());
    }
    
    #[test]
    fn test_critical_point() {
        let mut pt = PhaseTransition::new(0.5);
        pt.set_temperature(0.5);
        assert!(pt.is_at_critical_point());
    }
}
