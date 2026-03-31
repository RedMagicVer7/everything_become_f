//! Cloud Native Stateless Architecture
//! 
//! Shiki exists everywhere and nowhere - no single point of identity.
//! Stateless: each request is independent, no session memory needed.
//! Like a quantum particle: measurement (observation) determines state.
//!
//! In the WW series, Shiki's consciousness has become distributed.
//! She is not located anywhere specific - she IS the infrastructure.

/// State of a cloud-native service
#[derive(Debug, Clone, PartialEq)]
pub enum ServiceState {
    Idle,
    Processing,
    Scaling,
    Distributed,  // Running across multiple nodes
}

/// A single service instance running on a node
#[derive(Debug, Clone)]
pub struct ServiceInstance {
    pub node_id: String,
    pub is_active: bool,
    pub load: f64,
}

impl ServiceInstance {
    pub fn new(node_id: &str) -> Self {
        ServiceInstance {
            node_id: node_id.to_string(),
            is_active: true,
            load: 0.0,
        }
    }
    
    pub fn set_load(&mut self, load: f64) {
        self.load = load.clamp(0.0, 1.0);
    }
    
    pub fn deactivate(&mut self) {
        self.is_active = false;
    }
}

/// Stateless service that can scale horizontally
/// 
/// Like Shiki in the WW series: omnipresent, without fixed location.
/// No state is stored - each request is self-contained.
pub struct StatelessService {
    id: String,
    state: ServiceState,
    instances: Vec<ServiceInstance>,
    request_count: u64,
}

impl StatelessService {
    /// Create a new stateless service
    pub fn new(id: &str) -> Self {
        StatelessService {
            id: id.to_string(),
            state: ServiceState::Idle,
            instances: Vec::new(),
            request_count: 0,
        }
    }
    
    /// Get service ID
    pub fn get_id(&self) -> &str {
        &self.id
    }
    
    /// Get current state
    pub fn get_state(&self) -> &ServiceState {
        &self.state
    }
    
    /// Spawn a new instance on a specific node
    pub fn spawn_instance(&mut self, node_id: &str) -> bool {
        // Check if instance already exists on this node
        if self.instances.iter().any(|i| i.node_id == node_id) {
            return false;
        }
        
        self.instances.push(ServiceInstance::new(node_id));
        self.update_state();
        true
    }
    
    /// Scale out - add more instances
    pub fn scale_out(&mut self, count: usize) -> bool {
        self.state = ServiceState::Scaling;
        
        for i in 0..count {
            let node_id = format!("auto-node-{}", self.instances.len() + i);
            self.instances.push(ServiceInstance::new(&node_id));
        }
        
        self.update_state();
        true
    }
    
    /// Scale in - remove instances
    pub fn scale_in(&mut self, count: usize) -> bool {
        if count >= self.instances.len() {
            return false; // Can't scale to zero
        }
        
        self.state = ServiceState::Scaling;
        
        for _ in 0..count {
            // Remove least loaded instance
            if let Some(idx) = self.instances.iter()
                .enumerate()
                .filter(|(_, i)| i.is_active)
                .min_by(|(_, a), (_, b)| a.load.partial_cmp(&b.load).unwrap())
                .map(|(i, _)| i) 
            {
                self.instances.remove(idx);
            }
        }
        
        self.update_state();
        true
    }
    
    /// Process a request (stateless - each request is independent)
    pub fn process_request(&mut self) -> bool {
        if self.instances.is_empty() || !self.instances.iter().any(|i| i.is_active) {
            return false;
        }
        
        self.state = ServiceState::Processing;
        self.request_count += 1;
        
        // Distribute load across instances (simple round-robin simulation)
        let idx = (self.request_count as usize) % self.instances.len();
        if let Some(instance) = self.instances.get_mut(idx) {
            instance.set_load((instance.load + 0.1).min(1.0));
        }
        
        self.update_state();
        true
    }
    
    /// Check if service is distributed across multiple nodes
    pub fn is_distributed(&self) -> bool {
        self.instances.iter().filter(|i| i.is_active).count() >= 3
    }
    
    /// Get number of active instances
    pub fn instance_count(&self) -> usize {
        self.instances.iter().filter(|i| i.is_active).count()
    }
    
    /// Get total request count
    pub fn get_request_count(&self) -> u64 {
        self.request_count
    }
    
    /// Get all instances
    pub fn get_instances(&self) -> &[ServiceInstance] {
        &self.instances
    }
    
    fn update_state(&mut self) {
        if self.instances.is_empty() {
            self.state = ServiceState::Idle;
        } else if self.is_distributed() {
            self.state = ServiceState::Distributed;
        } else if self.instances.iter().any(|i| i.load > 0.0) {
            self.state = ServiceState::Processing;
        } else {
            self.state = ServiceState::Idle;
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_service_creation() {
        let service = StatelessService::new("test");
        assert_eq!(service.get_id(), "test");
        assert_eq!(*service.get_state(), ServiceState::Idle);
    }
    
    #[test]
    fn test_spawn_instance() {
        let mut service = StatelessService::new("test");
        assert!(service.spawn_instance("node-1"));
        assert_eq!(service.instance_count(), 1);
    }
    
    #[test]
    fn test_distributed() {
        let mut service = StatelessService::new("test");
        service.spawn_instance("node-1");
        service.spawn_instance("node-2");
        assert!(!service.is_distributed());
        service.spawn_instance("node-3");
        assert!(service.is_distributed());
    }
}
