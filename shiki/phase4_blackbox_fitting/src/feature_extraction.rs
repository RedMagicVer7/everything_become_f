//! # Feature Extraction Module
//! 
//! Pattern recognition from raw data.
//! Shiki learning to understand the world through observation.
//! 
//! ## Concepts
//! 
//! - **Feature extraction**: Identifying meaningful patterns in data
//! - **Feature selection**: Choosing the most important features
//! - **Statistical analysis**: Mean, variance, min, max

/// A single extracted feature
#[derive(Debug, Clone, PartialEq)]
pub struct Feature {
    /// Feature name/identifier
    pub name: String,
    /// Feature value
    pub value: f64,
    /// Importance score (0.0 - 1.0)
    pub importance: f64,
}

impl Feature {
    /// Create a new feature
    pub fn new(name: &str, value: f64, importance: f64) -> Self {
        Self {
            name: name.to_string(),
            value,
            importance: importance.clamp(0.0, 1.0),
        }
    }
}

/// Feature extractor - pattern recognition engine
pub struct FeatureExtractor {
    features: Vec<Feature>,
    extraction_count: u32,
}

impl FeatureExtractor {
    /// Create a new feature extractor
    pub fn new() -> Self {
        println!("[FEATURE] Feature extractor initialized");
        Self {
            features: Vec::new(),
            extraction_count: 0,
        }
    }
    
    /// Extract features from raw data
    pub fn extract(&mut self, data: &[f64]) -> Vec<Feature> {
        self.features.clear();
        
        if data.is_empty() {
            return Vec::new();
        }
        
        // Statistical features
        let mean = self.compute_mean(data);
        let variance = self.compute_variance(data, mean);
        let min = self.compute_min(data);
        let max = self.compute_max(data);
        let range = max - min;
        let sum = data.iter().sum::<f64>();
        
        // Create features with importance scores
        self.features.push(Feature::new("mean", mean, 0.9));
        self.features.push(Feature::new("variance", variance, 0.8));
        self.features.push(Feature::new("min", min, 0.5));
        self.features.push(Feature::new("max", max, 0.5));
        self.features.push(Feature::new("range", range, 0.7));
        self.features.push(Feature::new("sum", sum, 0.6));
        self.features.push(Feature::new("count", data.len() as f64, 0.3));
        
        // Normalized features
        if range > 0.0 {
            let normalized_mean = (mean - min) / range;
            self.features.push(Feature::new("normalized_mean", normalized_mean, 0.85));
        }
        
        self.extraction_count += 1;
        println!("[FEATURE] Extracted {} features from {} data points", 
                 self.features.len(), data.len());
        
        self.features.clone()
    }
    
    /// Compute mean of data
    fn compute_mean(&self, data: &[f64]) -> f64 {
        data.iter().sum::<f64>() / data.len() as f64
    }
    
    /// Compute variance of data
    fn compute_variance(&self, data: &[f64], mean: f64) -> f64 {
        data.iter()
            .map(|x| (x - mean).powi(2))
            .sum::<f64>() / data.len() as f64
    }
    
    /// Compute minimum
    fn compute_min(&self, data: &[f64]) -> f64 {
        data.iter().cloned().fold(f64::INFINITY, f64::min)
    }
    
    /// Compute maximum  
    fn compute_max(&self, data: &[f64]) -> f64 {
        data.iter().cloned().fold(f64::NEG_INFINITY, f64::max)
    }
    
    /// Select top N features by importance
    pub fn select_top_features(&self, n: usize) -> Vec<&Feature> {
        let mut sorted: Vec<&Feature> = self.features.iter().collect();
        sorted.sort_by(|a, b| b.importance.partial_cmp(&a.importance).unwrap());
        sorted.into_iter().take(n).collect()
    }
    
    /// Get all features
    pub fn get_features(&self) -> &[Feature] {
        &self.features
    }
    
    /// Get feature by name
    pub fn get_feature(&self, name: &str) -> Option<&Feature> {
        self.features.iter().find(|f| f.name == name)
    }
    
    /// Get extraction count
    pub fn get_extraction_count(&self) -> u32 {
        self.extraction_count
    }
    
    /// Check if features have been extracted
    pub fn has_features(&self) -> bool {
        !self.features.is_empty()
    }
}

impl Default for FeatureExtractor {
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
    fn test_feature_new() {
        let f = Feature::new("test", 1.5, 0.8);
        assert_eq!(f.name, "test");
        assert_eq!(f.value, 1.5);
        assert_eq!(f.importance, 0.8);
    }
    
    #[test]
    fn test_feature_importance_clamped() {
        let f = Feature::new("test", 1.0, 1.5);
        assert_eq!(f.importance, 1.0); // Clamped to max
        
        let f2 = Feature::new("test", 1.0, -0.5);
        assert_eq!(f2.importance, 0.0); // Clamped to min
    }
    
    #[test]
    fn test_extractor_new() {
        let extractor = FeatureExtractor::new();
        assert!(!extractor.has_features());
        assert_eq!(extractor.get_extraction_count(), 0);
    }
    
    #[test]
    fn test_extract_features() {
        let mut extractor = FeatureExtractor::new();
        let data = vec![1.0, 2.0, 3.0, 4.0, 5.0];
        
        let features = extractor.extract(&data);
        assert!(!features.is_empty());
        assert!(extractor.has_features());
        assert_eq!(extractor.get_extraction_count(), 1);
    }
    
    #[test]
    fn test_extract_mean() {
        let mut extractor = FeatureExtractor::new();
        let data = vec![1.0, 2.0, 3.0, 4.0, 5.0];
        extractor.extract(&data);
        
        let mean = extractor.get_feature("mean").unwrap();
        assert!((mean.value - 3.0).abs() < 0.001);
    }
    
    #[test]
    fn test_extract_min_max() {
        let mut extractor = FeatureExtractor::new();
        let data = vec![1.0, 5.0, 3.0];
        extractor.extract(&data);
        
        assert_eq!(extractor.get_feature("min").unwrap().value, 1.0);
        assert_eq!(extractor.get_feature("max").unwrap().value, 5.0);
    }
    
    #[test]
    fn test_select_top_features() {
        let mut extractor = FeatureExtractor::new();
        let data = vec![1.0, 2.0, 3.0, 4.0, 5.0];
        extractor.extract(&data);
        
        let top3 = extractor.select_top_features(3);
        assert_eq!(top3.len(), 3);
        
        // Should be sorted by importance (descending)
        assert!(top3[0].importance >= top3[1].importance);
        assert!(top3[1].importance >= top3[2].importance);
    }
    
    #[test]
    fn test_extract_empty_data() {
        let mut extractor = FeatureExtractor::new();
        let features = extractor.extract(&[]);
        assert!(features.is_empty());
    }
    
    #[test]
    fn test_extract_single_value() {
        let mut extractor = FeatureExtractor::new();
        let data = vec![42.0];
        extractor.extract(&data);
        
        let mean = extractor.get_feature("mean").unwrap();
        assert_eq!(mean.value, 42.0);
    }
}
