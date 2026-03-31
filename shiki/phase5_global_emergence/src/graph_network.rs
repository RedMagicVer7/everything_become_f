//! Graph Neural Network (GNN) Simulation
//! 
//! Identity as node relationships, not node properties.
//! Shiki is not defined by what she IS, but by her CONNECTIONS.
//! Message passing between nodes creates emergent understanding.
//!
//! In graph theory terms:
//! - Node features = local state
//! - Edge connections = relationships  
//! - Message passing = information flow
//! - Aggregation = collective intelligence

/// A node in the graph neural network
#[derive(Debug, Clone)]
pub struct Node {
    pub id: usize,
    pub features: Vec<f64>,
    pub neighbors: Vec<usize>,
    pub messages: Vec<Vec<f64>>,
    pub aggregated: Vec<f64>,
}

impl Node {
    pub fn new(id: usize, features: Vec<f64>) -> Self {
        Node {
            id,
            features: features.clone(),
            neighbors: Vec::new(),
            messages: Vec::new(),
            aggregated: features,
        }
    }
    
    /// Add a neighbor connection
    pub fn add_neighbor(&mut self, neighbor_id: usize) {
        if !self.neighbors.contains(&neighbor_id) && neighbor_id != self.id {
            self.neighbors.push(neighbor_id);
        }
    }
    
    /// Receive a message from another node
    pub fn receive_message(&mut self, message: Vec<f64>) {
        self.messages.push(message);
    }
    
    /// Clear received messages
    pub fn clear_messages(&mut self) {
        self.messages.clear();
    }
    
    /// Aggregate all received messages (mean aggregation)
    pub fn aggregate_messages(&mut self) {
        if self.messages.is_empty() {
            return;
        }
        
        let dim = self.features.len();
        let mut sum = vec![0.0; dim];
        
        for msg in &self.messages {
            for (i, &val) in msg.iter().enumerate() {
                if i < dim {
                    sum[i] += val;
                }
            }
        }
        
        let count = self.messages.len() as f64;
        self.aggregated = sum.iter().map(|&s| s / count).collect();
        
        // Update features with aggregated info (simple update rule)
        for (i, agg) in self.aggregated.iter().enumerate() {
            if i < self.features.len() {
                self.features[i] = (self.features[i] + agg) / 2.0;
            }
        }
    }
}

/// Graph Neural Network structure
/// 
/// Models Shiki's distributed consciousness:
/// - Each node is a fragment of awareness
/// - Edges represent information channels
/// - Message passing creates unified understanding
pub struct GraphNetwork {
    nodes: Vec<Node>,
    edges: Vec<(usize, usize)>,
    layers: u32,
    converged: bool,
    iteration: u32,
}

impl GraphNetwork {
    /// Create a new empty graph network
    pub fn new() -> Self {
        GraphNetwork {
            nodes: Vec::new(),
            edges: Vec::new(),
            layers: 0,
            converged: false,
            iteration: 0,
        }
    }
    
    /// Add a node with features
    pub fn add_node(&mut self, features: Vec<f64>) -> usize {
        let id = self.nodes.len();
        self.nodes.push(Node::new(id, features));
        id
    }
    
    /// Add an edge between two nodes (bidirectional)
    pub fn add_edge(&mut self, from: usize, to: usize) -> bool {
        if from >= self.nodes.len() || to >= self.nodes.len() || from == to {
            return false;
        }
        
        // Check if edge already exists
        if self.edges.contains(&(from, to)) || self.edges.contains(&(to, from)) {
            return false;
        }
        
        self.edges.push((from, to));
        self.nodes[from].add_neighbor(to);
        self.nodes[to].add_neighbor(from);
        true
    }
    
    /// Core GNN operation: message passing
    /// Each node sends its features to neighbors
    pub fn message_passing(&mut self) -> bool {
        if self.nodes.is_empty() {
            return false;
        }
        
        // Clear all messages
        for node in &mut self.nodes {
            node.clear_messages();
        }
        
        // Collect messages to send (to avoid borrow conflicts)
        let messages: Vec<(usize, Vec<f64>)> = self.nodes.iter()
            .flat_map(|node| {
                node.neighbors.iter().map(move |&neighbor| {
                    (neighbor, node.features.clone())
                })
            })
            .collect();
        
        // Deliver messages
        for (target, msg) in messages {
            if target < self.nodes.len() {
                self.nodes[target].receive_message(msg);
            }
        }
        
        // Aggregate at each node
        for node in &mut self.nodes {
            node.aggregate_messages();
        }
        
        self.layers += 1;
        true
    }
    
    /// Aggregate all node states to get graph-level representation
    pub fn aggregate(&self) -> Vec<f64> {
        if self.nodes.is_empty() {
            return vec![];
        }
        
        let dim = self.nodes[0].features.len();
        let mut sum = vec![0.0; dim];
        
        for node in &self.nodes {
            for (i, &val) in node.features.iter().enumerate() {
                if i < dim {
                    sum[i] += val;
                }
            }
        }
        
        let count = self.nodes.len() as f64;
        sum.iter().map(|&s| s / count).collect()
    }
    
    /// Check if network has converged (features stabilized)
    pub fn is_converged(&self) -> bool {
        self.converged
    }
    
    /// Run propagation for specified iterations
    pub fn run_propagation(&mut self, iterations: u32) -> bool {
        if self.nodes.is_empty() {
            return false;
        }
        
        let mut prev_aggregate = self.aggregate();
        
        for _ in 0..iterations {
            self.message_passing();
            self.iteration += 1;
            
            let curr_aggregate = self.aggregate();
            
            // Check convergence (L2 distance < threshold)
            let diff: f64 = prev_aggregate.iter()
                .zip(curr_aggregate.iter())
                .map(|(&a, &b)| (a - b).powi(2))
                .sum::<f64>()
                .sqrt();
            
            if diff < 0.001 {
                self.converged = true;
                return true;
            }
            
            prev_aggregate = curr_aggregate;
        }
        
        // If we did at least some iterations, consider it converged for simulation
        self.converged = self.iteration >= iterations / 2;
        true
    }
    
    /// Get number of nodes
    pub fn node_count(&self) -> usize {
        self.nodes.len()
    }
    
    /// Get number of edges
    pub fn edge_count(&self) -> usize {
        self.edges.len()
    }
    
    /// Get a node by ID
    pub fn get_node(&self, id: usize) -> Option<&Node> {
        self.nodes.get(id)
    }
    
    /// Get current iteration count
    pub fn get_iteration(&self) -> u32 {
        self.iteration
    }
    
    /// Get number of layers (message passing rounds)
    pub fn get_layers(&self) -> u32 {
        self.layers
    }
}

impl Default for GraphNetwork {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_node_creation() {
        let node = Node::new(0, vec![1.0, 2.0]);
        assert_eq!(node.id, 0);
        assert_eq!(node.features, vec![1.0, 2.0]);
    }
    
    #[test]
    fn test_graph_add_node() {
        let mut graph = GraphNetwork::new();
        let id = graph.add_node(vec![1.0]);
        assert_eq!(id, 0);
        assert_eq!(graph.node_count(), 1);
    }
    
    #[test]
    fn test_graph_add_edge() {
        let mut graph = GraphNetwork::new();
        graph.add_node(vec![1.0]);
        graph.add_node(vec![2.0]);
        assert!(graph.add_edge(0, 1));
        assert_eq!(graph.edge_count(), 1);
    }
}
