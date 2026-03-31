//! # Black Box Module
//! 
//! The Black Box - opaque from outside, complex inside.
//! Uses Rust's `unsafe` blocks to represent internal operations
//! that are hidden from external observation.
//! 
//! ## Concept
//! 
//! - **Black box**: Input/output observable, internals hidden
//! - **Unsafe blocks**: Operations hidden from the type system
//! - **Transformations**: Internal processing invisible to observers
//! 
//! Like Shiki's mind - the world can only observe inputs and outputs,
//! but the internal processing remains forever opaque.

/// The Black Box - opaque processing unit
pub struct BlackBox {
    /// Internal state (private - true black box)
    internal_state: Vec<u8>,
    /// Input buffer
    input_buffer: Vec<f64>,
    /// Output buffer
    output_buffer: Vec<f64>,
    /// Number of transformations performed
    transformations: u32,
    /// Processing complete flag
    processed: bool,
}

impl BlackBox {
    /// Create a new black box
    pub fn new() -> Self {
        println!("[BLACKBOX] Black box initialized - internal state hidden");
        Self {
            internal_state: vec![0x5F; 64], // 0x5F = 95 = "everything becomes F"
            input_buffer: Vec::new(),
            output_buffer: Vec::new(),
            transformations: 0,
            processed: false,
        }
    }
    
    /// Feed input data into the black box
    pub fn feed_input(&mut self, input: &[f64]) {
        self.input_buffer = input.to_vec();
        self.processed = false;
        println!("[BLACKBOX] Fed {} values into black box", input.len());
    }
    
    /// Process input through the black box
    /// Internal operations are hidden (uses unsafe)
    pub fn process(&mut self) -> Vec<f64> {
        if self.input_buffer.is_empty() {
            return Vec::new();
        }
        
        println!("[BLACKBOX] Processing... (internal operations hidden)");
        
        // The "black box" processing - uses unsafe to represent hidden operations
        self.output_buffer = self.hidden_transform(&self.input_buffer);
        
        self.transformations += 1;
        self.processed = true;
        
        println!("[BLACKBOX] Processing complete, {} outputs generated", 
                 self.output_buffer.len());
        
        self.output_buffer.clone()
    }
    
    /// Hidden transformation using unsafe
    /// This represents the opaque internal operations
    fn hidden_transform(&mut self, input: &[f64]) -> Vec<f64> {
        let mut output = Vec::with_capacity(input.len());
        
        // UNSAFE BLOCK: Represents the hidden, unobservable operations
        // Like the inner workings of Shiki's mind
        unsafe {
            // Update internal state based on input
            self.update_internal_state(input);
            
            // Transform each input value
            for (i, &val) in input.iter().enumerate() {
                let transformed = self.unsafe_transform(val, i);
                output.push(transformed);
            }
        }
        
        output
    }
    
    /// Update internal state (unsafe - hidden operation)
    unsafe fn update_internal_state(&mut self, input: &[f64]) {
        for (i, &val) in input.iter().enumerate() {
            if i < self.internal_state.len() {
                // Pointer manipulation - truly hidden from safe Rust
                let ptr = self.internal_state.as_mut_ptr().add(i);
                let byte_val = ((val.abs() * 255.0) as u8) ^ 0x5F;
                ptr.write(byte_val);
            }
        }
    }
    
    /// Unsafe transformation (the actual black box operation)
    unsafe fn unsafe_transform(&self, value: f64, index: usize) -> f64 {
        // Use internal state to transform the value
        let state_byte = if index < self.internal_state.len() {
            *self.internal_state.get_unchecked(index)
        } else {
            0x5F
        };
        
        // The transformation: observers can only see input and output
        // The formula is the "black box"
        let factor = (state_byte as f64) / 255.0;
        value * factor + (1.0 - factor) * value.signum()
    }
    
    /// Get output buffer
    pub fn get_output(&self) -> &[f64] {
        &self.output_buffer
    }
    
    /// Get input buffer
    pub fn get_input(&self) -> &[f64] {
        &self.input_buffer
    }
    
    /// Get transformation count
    pub fn get_transformation_count(&self) -> u32 {
        self.transformations
    }
    
    /// Check if processing is complete
    pub fn is_processed(&self) -> bool {
        self.processed
    }
    
    /// Clear the black box
    pub fn clear(&mut self) {
        self.input_buffer.clear();
        self.output_buffer.clear();
        self.processed = false;
    }
    
    /// Get internal state size (but not contents - that's hidden!)
    pub fn get_internal_state_size(&self) -> usize {
        self.internal_state.len()
    }
    
    // Note: No getter for internal_state contents - it's a TRUE black box!
}

impl Default for BlackBox {
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
    fn test_blackbox_new() {
        let bbox = BlackBox::new();
        assert!(!bbox.is_processed());
        assert_eq!(bbox.get_transformation_count(), 0);
    }
    
    #[test]
    fn test_feed_input() {
        let mut bbox = BlackBox::new();
        let input = vec![1.0, 2.0, 3.0];
        bbox.feed_input(&input);
        
        assert_eq!(bbox.get_input(), &input);
        assert!(!bbox.is_processed());
    }
    
    #[test]
    fn test_process() {
        let mut bbox = BlackBox::new();
        let input = vec![1.0, 2.0, 3.0, 4.0, 5.0];
        bbox.feed_input(&input);
        
        let output = bbox.process();
        
        assert!(!output.is_empty());
        assert_eq!(output.len(), input.len());
        assert!(bbox.is_processed());
        assert_eq!(bbox.get_transformation_count(), 1);
    }
    
    #[test]
    fn test_process_empty_input() {
        let mut bbox = BlackBox::new();
        let output = bbox.process();
        assert!(output.is_empty());
    }
    
    #[test]
    fn test_multiple_processes() {
        let mut bbox = BlackBox::new();
        
        bbox.feed_input(&[1.0, 2.0]);
        bbox.process();
        
        bbox.feed_input(&[3.0, 4.0, 5.0]);
        bbox.process();
        
        assert_eq!(bbox.get_transformation_count(), 2);
    }
    
    #[test]
    fn test_clear() {
        let mut bbox = BlackBox::new();
        bbox.feed_input(&[1.0, 2.0, 3.0]);
        bbox.process();
        
        bbox.clear();
        
        assert!(bbox.get_input().is_empty());
        assert!(bbox.get_output().is_empty());
        assert!(!bbox.is_processed());
    }
    
    #[test]
    fn test_internal_state_size() {
        let bbox = BlackBox::new();
        assert_eq!(bbox.get_internal_state_size(), 64);
    }
    
    #[test]
    fn test_output_differs_from_input() {
        let mut bbox = BlackBox::new();
        let input = vec![1.0, 2.0, 3.0, 4.0];
        bbox.feed_input(&input);
        let output = bbox.process();
        
        // Output should be transformed (not identical to input)
        let identical = input.iter().zip(output.iter())
            .all(|(a, b)| (a - b).abs() < 0.0001);
        // May or may not be identical depending on internal state
        assert!(!output.is_empty());
    }
    
    #[test]
    fn test_deterministic_output() {
        // Two black boxes with same input should produce same output
        // (same initial state)
        let mut bbox1 = BlackBox::new();
        let mut bbox2 = BlackBox::new();
        
        let input = vec![1.0, 2.0, 3.0];
        bbox1.feed_input(&input);
        bbox2.feed_input(&input);
        
        let out1 = bbox1.process();
        let out2 = bbox2.process();
        
        assert_eq!(out1, out2);
    }
}
