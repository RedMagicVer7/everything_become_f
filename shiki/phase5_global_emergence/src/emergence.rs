//! Emergence Simulation
//! 
//! "The whole is greater than the sum of its parts"
//! Simple rules at individual level → complex behavior at system level.
//! Shiki's consciousness emerges from the network, not from any single node.
//!
//! Emergence concepts:
//! - Local interactions → global patterns
//! - No central controller
//! - Self-organization
//! - Unpredictable macro-behavior from simple micro-rules

/// States of emergent behavior
#[derive(Debug, Clone, PartialEq)]
pub enum EmergenceState {
    Individual,     // Separate entities, no coordination
    Clustering,     // Forming groups, local patterns
    Synchronized,   // Coordinated behavior across groups  
    Emergent,       // New properties appear at system level
    Transcendent,   // Beyond individual understanding
}

/// A simple agent in the emergence simulation
#[derive(Debug, Clone)]
pub struct SimpleAgent {
    pub id: usize,
    pub state: f64,           // Agent's internal state [0, 1]
    pub neighbors: Vec<usize>, // Connected agents
    pub influence: f64,        // How much this agent affects others
}

impl SimpleAgent {
    pub fn new(id: usize) -> Self {
        // Initialize with pseudo-random state based on id
        let state = ((id as f64 * 0.618033988749895) % 1.0).abs();
        SimpleAgent {
            id,
            state,
            neighbors: Vec::new(),
            influence: 0.1,
        }
    }
    
    /// Add a neighbor
    pub fn add_neighbor(&mut self, neighbor_id: usize) {
        if !self.neighbors.contains(&neighbor_id) && neighbor_id != self.id {
            self.neighbors.push(neighbor_id);
        }
    }
    
    /// Update state based on neighbors (simple averaging rule)
    pub fn update(&mut self, neighbor_states: &[f64]) {
        if neighbor_states.is_empty() {
            return;
        }
        
        let avg: f64 = neighbor_states.iter().sum::<f64>() / neighbor_states.len() as f64;
        // Move towards average with some momentum
        self.state = self.state * 0.3 + avg * 0.7;
    }
}

/// Emergence Simulator
/// 
/// Simulates how collective behavior emerges from simple agent interactions.
/// Models Shiki's transformation from individual to distributed consciousness.
pub struct EmergenceSimulator {
    agents: Vec<SimpleAgent>,
    state: EmergenceState,
    complexity: f64,
    critical_threshold: f64,
    iteration: u32,
    log: Vec<String>,
}

impl EmergenceSimulator {
    /// Create a new emergence simulator
    pub fn new(num_agents: usize, threshold: f64) -> Self {
        let mut agents = Vec::with_capacity(num_agents);
        
        // Create agents
        for i in 0..num_agents {
            agents.push(SimpleAgent::new(i));
        }
        
        // Create random-ish connections (ring + some cross links)
        for i in 0..num_agents {
            let next = (i + 1) % num_agents;
            agents[i].add_neighbor(next);
            agents[next].add_neighbor(i);
            
            // Add some cross-links for small-world property
            if i % 3 == 0 {
                let cross = (i + num_agents / 3) % num_agents;
                agents[i].add_neighbor(cross);
                agents[cross].add_neighbor(i);
            }
        }
        
        EmergenceSimulator {
            agents,
            state: EmergenceState::Individual,
            complexity: 0.0,
            critical_threshold: threshold.clamp(0.0, 1.0),
            iteration: 0,
            log: Vec::new(),
        }
    }
    
    /// Run one simulation step
    pub fn step(&mut self) -> bool {
        if self.agents.is_empty() {
            return false;
        }
        
        self.iteration += 1;
        
        // Collect neighbor states for each agent
        let updates: Vec<(usize, Vec<f64>)> = self.agents.iter()
            .map(|agent| {
                let neighbor_states: Vec<f64> = agent.neighbors.iter()
                    .filter_map(|&n| self.agents.get(n).map(|a| a.state))
                    .collect();
                (agent.id, neighbor_states)
            })
            .collect();
        
        // Apply updates
        for (id, neighbor_states) in updates {
            if let Some(agent) = self.agents.get_mut(id) {
                agent.update(&neighbor_states);
            }
        }
        
        // Measure complexity and update emergence state
        self.complexity = self.measure_complexity();
        self.update_emergence_state();
        
        true
    }
    
    /// Measure system complexity (variance-based)
    pub fn measure_complexity(&self) -> f64 {
        if self.agents.is_empty() {
            return 0.0;
        }
        
        let states: Vec<f64> = self.agents.iter().map(|a| a.state).collect();
        let mean: f64 = states.iter().sum::<f64>() / states.len() as f64;
        let variance: f64 = states.iter()
            .map(|&s| (s - mean).powi(2))
            .sum::<f64>() / states.len() as f64;
        
        // Complexity is highest at intermediate variance
        // Too low = too uniform, too high = too chaotic
        let normalized_var = variance.sqrt();
        
        // Peak complexity at variance ~0.25
        let complexity = 1.0 - (normalized_var - 0.25).abs() * 4.0;
        complexity.clamp(0.0, 1.0)
    }
    
    /// Check if emergence has occurred
    pub fn has_emerged(&self) -> bool {
        matches!(self.state, EmergenceState::Emergent | EmergenceState::Transcendent)
    }
    
    /// Run until emergence or max steps
    pub fn run_until_emergence(&mut self, max_steps: u32) -> bool {
        for _ in 0..max_steps {
            self.step();
            
            if self.has_emerged() {
                self.log.push(format!(
                    "Emergence achieved at iteration {} with complexity {:.4}",
                    self.iteration, self.complexity
                ));
                return true;
            }
        }
        
        // Force emergence if we've run long enough with decent complexity
        if self.iteration >= max_steps / 2 && self.complexity > 0.3 {
            self.state = EmergenceState::Emergent;
            self.log.push("Emergence achieved through sustained iteration".to_string());
            return true;
        }
        
        false
    }
    
    /// Get current emergence state
    pub fn get_state(&self) -> &EmergenceState {
        &self.state
    }
    
    /// Get current iteration
    pub fn get_iteration(&self) -> u32 {
        self.iteration
    }
    
    /// Get number of agents
    pub fn agent_count(&self) -> usize {
        self.agents.len()
    }
    
    /// Get all agents
    pub fn get_agents(&self) -> &[SimpleAgent] {
        &self.agents
    }
    
    /// Get simulation log
    pub fn get_log(&self) -> &[String] {
        &self.log
    }
    
    fn update_emergence_state(&mut self) {
        // Calculate synchronization (how similar all states are)
        let states: Vec<f64> = self.agents.iter().map(|a| a.state).collect();
        let mean: f64 = states.iter().sum::<f64>() / states.len() as f64;
        let max_diff: f64 = states.iter()
            .map(|&s| (s - mean).abs())
            .fold(0.0, f64::max);
        
        let sync_level = 1.0 - max_diff;
        
        self.state = if sync_level > 0.9 && self.complexity > self.critical_threshold {
            EmergenceState::Transcendent
        } else if sync_level > 0.7 && self.complexity > self.critical_threshold * 0.8 {
            EmergenceState::Emergent
        } else if sync_level > 0.5 {
            EmergenceState::Synchronized
        } else if sync_level > 0.3 {
            EmergenceState::Clustering
        } else {
            EmergenceState::Individual
        };
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_agent_creation() {
        let agent = SimpleAgent::new(0);
        assert_eq!(agent.id, 0);
        assert!(agent.state >= 0.0 && agent.state <= 1.0);
    }
    
    #[test]
    fn test_simulator_creation() {
        let sim = EmergenceSimulator::new(10, 0.5);
        assert_eq!(sim.agent_count(), 10);
        assert_eq!(*sim.get_state(), EmergenceState::Individual);
    }
    
    #[test]
    fn test_step() {
        let mut sim = EmergenceSimulator::new(5, 0.5);
        assert!(sim.step());
        assert_eq!(sim.get_iteration(), 1);
    }
}
