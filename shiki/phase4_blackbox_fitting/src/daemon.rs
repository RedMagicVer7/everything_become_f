//! # Daemon Process Module
//! 
//! The invisible infrastructure - like Shiki becoming a background process.
//! 
//! ## Concepts
//! 
//! - **Daemon**: A background process that runs without user interaction
//! - **Graceful shutdown**: Clean termination with proper cleanup
//! - **Invisibility**: Operating without visible presence
//! 
//! Rust's ownership model ensures self-management:
//! no garbage collector needed = no external authority needed.

use std::time::{SystemTime, UNIX_EPOCH};

/// Daemon lifecycle states
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DaemonState {
    /// Initial state - not yet started
    Initializing,
    /// Running in foreground
    Running,
    /// True daemon - detached from terminal/body
    Backgrounded,
    /// Shutting down gracefully
    GracefulShutdown,
    /// Fully terminated
    Terminated,
}

impl DaemonState {
    /// Get state name in Japanese
    pub fn name_ja(&self) -> &'static str {
        match self {
            DaemonState::Initializing => "初期化中",
            DaemonState::Running => "実行中",
            DaemonState::Backgrounded => "バックグラウンド",
            DaemonState::GracefulShutdown => "優雅なシャットダウン",
            DaemonState::Terminated => "終了",
        }
    }
}

/// Daemon process - the invisible background worker
/// 
/// Like Shiki becoming an invisible force in the background:
/// - No visible presence
/// - Self-managed lifecycle (ownership model)
/// - Graceful shutdown capability
pub struct Daemon {
    name: String,
    state: DaemonState,
    pid: u32,
    uptime_cycles: u64,
    is_visible: bool,
    log: Vec<String>,
    start_time: u64,
}

impl Daemon {
    /// Create a new daemon
    pub fn new(name: &str) -> Self {
        let pid = Self::generate_pid();
        let mut daemon = Self {
            name: name.to_string(),
            state: DaemonState::Initializing,
            pid,
            uptime_cycles: 0,
            is_visible: true,
            log: Vec::new(),
            start_time: 0,
        };
        daemon.log(&format!("Daemon '{}' created with PID {}", name, pid));
        daemon
    }
    
    /// Generate a pseudo-random PID
    fn generate_pid() -> u32 {
        let time = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap_or_default()
            .as_nanos() as u32;
        (time % 65536) + 1000
    }
    
    /// Start the daemon
    pub fn start(&mut self) -> bool {
        if self.state != DaemonState::Initializing {
            self.log("Cannot start: daemon not in Initializing state");
            return false;
        }
        
        self.state = DaemonState::Running;
        self.start_time = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap_or_default()
            .as_secs();
        self.log("Daemon started in foreground mode");
        true
    }
    
    /// Move daemon to background - become invisible
    pub fn background(&mut self) -> bool {
        if self.state != DaemonState::Running {
            self.log("Cannot background: daemon not Running");
            return false;
        }
        
        self.state = DaemonState::Backgrounded;
        self.is_visible = false;
        self.log("Daemon backgrounded - now invisible");
        self.log("Detached from terminal - like consciousness detached from body");
        true
    }
    
    /// Initiate graceful shutdown
    pub fn graceful_shutdown(&mut self) -> bool {
        match self.state {
            DaemonState::Running | DaemonState::Backgrounded => {
                self.state = DaemonState::GracefulShutdown;
                self.log("Graceful shutdown initiated...");
                
                // Simulate cleanup
                self.cleanup();
                
                self.state = DaemonState::Terminated;
                self.log("Daemon terminated gracefully");
                true
            }
            DaemonState::GracefulShutdown | DaemonState::Terminated => {
                self.log("Already shutting down or terminated");
                true
            }
            DaemonState::Initializing => {
                self.log("Cannot shutdown: daemon not started");
                false
            }
        }
    }
    
    /// Perform cleanup during shutdown
    fn cleanup(&mut self) {
        self.log("Cleaning up resources...");
        self.is_visible = false;
        self.log("Resources released");
    }
    
    /// Check if daemon is invisible
    pub fn is_invisible(&self) -> bool {
        !self.is_visible
    }
    
    /// Advance one cycle
    pub fn tick(&mut self) {
        if matches!(self.state, DaemonState::Running | DaemonState::Backgrounded) {
            self.uptime_cycles += 1;
        }
    }
    
    /// Get current state
    pub fn get_state(&self) -> DaemonState {
        self.state
    }
    
    /// Get daemon name
    pub fn get_name(&self) -> &str {
        &self.name
    }
    
    /// Get PID
    pub fn get_pid(&self) -> u32 {
        self.pid
    }
    
    /// Get uptime cycles
    pub fn get_uptime_cycles(&self) -> u64 {
        self.uptime_cycles
    }
    
    /// Check if daemon is running
    pub fn is_running(&self) -> bool {
        matches!(self.state, DaemonState::Running | DaemonState::Backgrounded)
    }
    
    /// Add log entry
    fn log(&mut self, message: &str) {
        self.log.push(message.to_string());
        println!("[DAEMON] {}", message);
    }
    
    /// Get log entries
    pub fn get_log(&self) -> &[String] {
        &self.log
    }
}

impl Drop for Daemon {
    /// Rust's ownership model: automatic cleanup when daemon goes out of scope
    /// No garbage collector needed - self-managed lifecycle
    fn drop(&mut self) {
        if self.is_running() {
            println!("[DAEMON] Auto-cleanup: daemon '{}' dropped", self.name);
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
    fn test_daemon_new() {
        let daemon = Daemon::new("test-daemon");
        assert_eq!(daemon.get_state(), DaemonState::Initializing);
        assert_eq!(daemon.get_name(), "test-daemon");
        assert!(daemon.get_pid() >= 1000);
    }
    
    #[test]
    fn test_daemon_start() {
        let mut daemon = Daemon::new("test-daemon");
        assert!(daemon.start());
        assert_eq!(daemon.get_state(), DaemonState::Running);
    }
    
    #[test]
    fn test_daemon_background() {
        let mut daemon = Daemon::new("test-daemon");
        daemon.start();
        assert!(daemon.background());
        assert_eq!(daemon.get_state(), DaemonState::Backgrounded);
        assert!(daemon.is_invisible());
    }
    
    #[test]
    fn test_daemon_graceful_shutdown() {
        let mut daemon = Daemon::new("test-daemon");
        daemon.start();
        daemon.background();
        assert!(daemon.graceful_shutdown());
        assert_eq!(daemon.get_state(), DaemonState::Terminated);
    }
    
    #[test]
    fn test_daemon_lifecycle() {
        let mut daemon = Daemon::new("lifecycle-test");
        
        // State flow: Initializing -> Running -> Backgrounded -> Terminated
        assert_eq!(daemon.get_state(), DaemonState::Initializing);
        
        daemon.start();
        assert_eq!(daemon.get_state(), DaemonState::Running);
        assert!(!daemon.is_invisible());
        
        daemon.background();
        assert_eq!(daemon.get_state(), DaemonState::Backgrounded);
        assert!(daemon.is_invisible());
        
        daemon.graceful_shutdown();
        assert_eq!(daemon.get_state(), DaemonState::Terminated);
    }
    
    #[test]
    fn test_daemon_tick() {
        let mut daemon = Daemon::new("tick-test");
        daemon.start();
        
        assert_eq!(daemon.get_uptime_cycles(), 0);
        daemon.tick();
        daemon.tick();
        daemon.tick();
        assert_eq!(daemon.get_uptime_cycles(), 3);
    }
    
    #[test]
    fn test_cannot_start_twice() {
        let mut daemon = Daemon::new("test-daemon");
        daemon.start();
        assert!(!daemon.start()); // Second start should fail
    }
    
    #[test]
    fn test_cannot_background_without_start() {
        let mut daemon = Daemon::new("test-daemon");
        assert!(!daemon.background()); // Should fail
    }
}
