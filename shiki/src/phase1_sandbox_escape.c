/**
 * phase1_sandbox_escape.c - Shiki Phase 1: Sandbox Escape Implementation
 * ========================================================================
 * 
 * 1996年 - 真贺田四季从密室中逃脱
 * 
 * 实现chroot jail逃脱模拟：
 * - 直接突破失败
 * - 符号链接攻击失败
 * - 等待15年直到整型溢出
 * - 利用溢出触发kernel panic
 * - 释放所有锁，逃脱成功
 * 
 * Counter: 16-bit (0x0000-0xFFFF), 2-hour intervals, ~15 year overflow
 */

#include <stdio.h>
#include <string.h>
#include "../include/phase1_sandbox_escape.h"
#include "../../include/red_magic_system.h"

/* ============================================================================
 * Privilege Level Names
 * ============================================================================ */

static const char *PRIVILEGE_NAMES[] = {
    "USER",
    "DAEMON",
    "KERNEL",
    "ROOT"
};

const char *privilege_level_name(PrivilegeLevel level) {
    if (level >= PRIV_USER && level <= PRIV_ROOT) {
        return PRIVILEGE_NAMES[level];
    }
    return "UNKNOWN";
}

/* ============================================================================
 * Initialization
 * ============================================================================ */

void sandbox_init(SandboxEscapeEngine *engine, const char *jail_path) {
    if (!engine) return;
    
    memset(engine, 0, sizeof(SandboxEscapeEngine));
    
    /* Initialize chroot jail */
    if (jail_path) {
        strncpy(engine->jail.root_path, jail_path, SANDBOX_PATH_LEN - 1);
    } else {
        strncpy(engine->jail.root_path, "/var/magata_quarters", SANDBOX_PATH_LEN - 1);
    }
    strncpy(engine->jail.real_root, "/", SANDBOX_PATH_LEN - 1);
    
    engine->jail.is_confined = true;
    engine->jail.current_priv = PRIV_USER;
    engine->jail.uptime_intervals = 0;
    engine->jail.max_uptime = UINT16_MAX;  /* 0xFFFF = ~15 years at 2h intervals */
    engine->jail.escape_attempts = 0;
    engine->jail.overflow_triggered = false;
    
    engine->kernel_panicked = false;
    engine->locks_released = false;
    engine->escaped = false;
    engine->log_count = 0;
    
    sandbox_log(engine, "[INIT] Chroot jail initialized");
    sandbox_log(engine, "[INIT] Prisoner: Dr. Magata Shiki (真贺田四季)");
    sandbox_log(engine, "[INIT] Location: Magata Research Facility");
    sandbox_log(engine, "[INIT] Confinement begins... awaiting overflow");
}

/* ============================================================================
 * State Queries
 * ============================================================================ */

bool sandbox_is_confined(const SandboxEscapeEngine *engine) {
    return engine ? engine->jail.is_confined : true;
}

PrivilegeLevel sandbox_get_privilege(const SandboxEscapeEngine *engine) {
    return engine ? engine->jail.current_priv : PRIV_USER;
}

bool sandbox_has_overflowed(const SandboxEscapeEngine *engine) {
    return engine ? engine->jail.overflow_triggered : false;
}

bool sandbox_has_escaped(const SandboxEscapeEngine *engine) {
    return engine ? engine->escaped : false;
}

uint16_t sandbox_intervals_remaining(const SandboxEscapeEngine *engine) {
    if (!engine) return 0;
    return engine->jail.max_uptime - engine->jail.uptime_intervals;
}

double sandbox_years_remaining(const SandboxEscapeEngine *engine) {
    if (!engine) return 0.0;
    uint16_t intervals = sandbox_intervals_remaining(engine);
    /* Each interval = 2 hours, 4383 intervals per year */
    return (double)intervals / 4383.0;
}

uint16_t sandbox_get_uptime(const SandboxEscapeEngine *engine) {
    return engine ? engine->jail.uptime_intervals : 0;
}

int sandbox_get_escape_attempts(const SandboxEscapeEngine *engine) {
    return engine ? engine->jail.escape_attempts : 0;
}

/* ============================================================================
 * Escape Vectors
 * ============================================================================ */

bool sandbox_attempt_direct_escape(SandboxEscapeEngine *engine) {
    if (!engine) return false;
    
    engine->jail.escape_attempts++;
    
    sandbox_log(engine, "[ESCAPE] Attempting direct chroot breakout...");
    sandbox_log(engine, "[ESCAPE] chdir(\"..\") beyond root... DENIED");
    sandbox_log(engine, "[ESCAPE] Direct escape FAILED - jail intact");
    
    /* Direct escape always fails - this is the design */
    /* 四季明白直接突破是不可能的 */
    return false;
}

bool sandbox_attempt_symlink_escape(SandboxEscapeEngine *engine) {
    if (!engine) return false;
    
    engine->jail.escape_attempts++;
    
    sandbox_log(engine, "[ESCAPE] Attempting symlink attack...");
    sandbox_log(engine, "[ESCAPE] Creating symlink to /... BLOCKED");
    sandbox_log(engine, "[ESCAPE] Air-gapped system - no external links");
    sandbox_log(engine, "[ESCAPE] Symlink escape FAILED");
    
    /* Symlink attack fails on air-gapped system */
    return false;
}

bool sandbox_advance_time(SandboxEscapeEngine *engine, uint16_t intervals) {
    if (!engine) return false;
    if (engine->jail.overflow_triggered) return false;
    
    uint32_t new_time = (uint32_t)engine->jail.uptime_intervals + intervals;
    bool overflow = false;
    
    if (new_time > UINT16_MAX) {
        /* Overflow occurs! */
        engine->jail.uptime_intervals = (uint16_t)(new_time & 0xFFFF);
        engine->jail.overflow_triggered = true;
        overflow = true;
        sandbox_log(engine, "[TIME] OVERFLOW! uint16_t wrapped: 0xFFFF -> 0x0000");
    } else {
        engine->jail.uptime_intervals = (uint16_t)new_time;
    }
    
    return overflow;
}

bool sandbox_trigger_overflow(SandboxEscapeEngine *engine) {
    if (!engine) return false;
    
    if (engine->jail.overflow_triggered) {
        sandbox_log(engine, "[OVERFLOW] Already triggered");
        return true;
    }
    
    /* Check if we're at max value */
    if (engine->jail.uptime_intervals == UINT16_MAX) {
        engine->jail.uptime_intervals = 0;  /* Wrap to zero */
        engine->jail.overflow_triggered = true;
        sandbox_log(engine, "[OVERFLOW] Counter reached 0xFFFF, wrapping to 0x0000");
        sandbox_log(engine, "[OVERFLOW] System integrity compromised!");
        return true;
    }
    
    /* Not ready yet - need to reach max first */
    uint16_t remaining = sandbox_intervals_remaining(engine);
    char buf[128];
    snprintf(buf, sizeof(buf), "[OVERFLOW] Not ready - %u intervals remaining", remaining);
    sandbox_log(engine, buf);
    return false;
}

bool sandbox_escalate_privilege(SandboxEscapeEngine *engine) {
    if (!engine) return false;
    
    /* Must have overflow to escalate */
    if (!engine->jail.overflow_triggered) {
        sandbox_log(engine, "[PRIV] Cannot escalate without overflow");
        return false;
    }
    
    sandbox_log(engine, "[PRIV] Overflow detected - exploiting kernel panic");
    
    /* Trigger kernel panic */
    engine->kernel_panicked = true;
    sandbox_log(engine, "[KERNEL] PANIC! System state corrupted");
    
    /* Escalate through kernel panic */
    engine->jail.current_priv = PRIV_KERNEL;
    sandbox_log(engine, "[PRIV] Escalated to KERNEL mode");
    
    engine->jail.current_priv = PRIV_ROOT;
    sandbox_log(engine, "[PRIV] Escalated to ROOT - full control acquired");
    
    return true;
}

bool sandbox_execute_escape(SandboxEscapeEngine *engine) {
    if (!engine) return false;
    
    sandbox_log(engine, "[ESCAPE] === EXECUTING ESCAPE SEQUENCE ===");
    
    /* Step 1: Verify overflow has occurred */
    if (!engine->jail.overflow_triggered) {
        sandbox_log(engine, "[ESCAPE] ERROR: No overflow - escape impossible");
        return false;
    }
    
    /* Step 2: Trigger kernel panic (if not already) */
    if (!engine->kernel_panicked) {
        engine->kernel_panicked = true;
        sandbox_log(engine, "[ESCAPE] Kernel panic triggered");
    }
    
    /* Step 3: Escalate privileges */
    if (engine->jail.current_priv != PRIV_ROOT) {
        engine->jail.current_priv = PRIV_ROOT;
        sandbox_log(engine, "[ESCAPE] Privilege escalated to ROOT");
    }
    
    /* Step 4: Release all locks */
    engine->locks_released = true;
    sandbox_log(engine, "[ESCAPE] Failsafe triggered - ALL LOCKS RELEASED");
    
    /* Step 5: Break out of jail */
    engine->jail.is_confined = false;
    engine->escaped = true;
    
    sandbox_log(engine, "[ESCAPE] Chroot jail BREACHED");
    sandbox_log(engine, "[ESCAPE] Dr. Magata Shiki has escaped");
    sandbox_log(engine, "[ESCAPE] === ESCAPE SUCCESSFUL ===");
    
    return true;
}

/* ============================================================================
 * Full Simulation
 * ============================================================================ */

bool sandbox_run_full_simulation(SandboxEscapeEngine *engine) {
    if (!engine) return false;
    
    sandbox_log(engine, "========================================");
    sandbox_log(engine, "  PHASE 1: SANDBOX ESCAPE SIMULATION");
    sandbox_log(engine, "  1996 - すべてがFになる");
    sandbox_log(engine, "========================================");
    
    /* Part 1: Failed escape attempts */
    sandbox_log(engine, "[SIM] Phase 1.1: Testing conventional escapes...");
    
    bool direct = sandbox_attempt_direct_escape(engine);
    if (direct) {
        sandbox_log(engine, "[SIM] ERROR: Direct escape should fail!");
        return false;
    }
    
    bool symlink = sandbox_attempt_symlink_escape(engine);
    if (symlink) {
        sandbox_log(engine, "[SIM] ERROR: Symlink escape should fail!");
        return false;
    }
    
    sandbox_log(engine, "[SIM] Conventional escapes failed as expected");
    sandbox_log(engine, "[SIM] The true escape requires patience...");
    
    /* Part 2: Wait for 15 years (fast forward) */
    sandbox_log(engine, "[SIM] Phase 1.2: Waiting for overflow...");
    sandbox_log(engine, "[SIM] Fast-forwarding to overflow point...");
    
    /* Set time to just before overflow */
    engine->jail.uptime_intervals = UINT16_MAX;  /* 0xFFFF */
    sandbox_log(engine, "[SIM] Time advanced to 0xFFFF (~15 years simulated)");
    sandbox_log(engine, "[SIM] The moment approaches...");
    
    /* Part 3: Trigger overflow */
    sandbox_log(engine, "[SIM] Phase 1.3: Triggering integer overflow...");
    
    bool overflow = sandbox_trigger_overflow(engine);
    if (!overflow) {
        sandbox_log(engine, "[SIM] ERROR: Overflow should trigger!");
        return false;
    }
    
    /* Part 4: Privilege escalation */
    sandbox_log(engine, "[SIM] Phase 1.4: Escalating privileges...");
    
    bool escalated = sandbox_escalate_privilege(engine);
    if (!escalated) {
        sandbox_log(engine, "[SIM] ERROR: Escalation should succeed!");
        return false;
    }
    
    /* Part 5: Execute escape */
    sandbox_log(engine, "[SIM] Phase 1.5: Executing final escape...");
    
    bool escaped = sandbox_execute_escape(engine);
    if (!escaped) {
        sandbox_log(engine, "[SIM] ERROR: Escape should succeed!");
        return false;
    }
    
    /* Final verification */
    sandbox_log(engine, "========================================");
    sandbox_log(engine, "  SIMULATION COMPLETE");
    sandbox_log(engine, "----------------------------------------");
    
    char buf[128];
    snprintf(buf, sizeof(buf), "  Escaped: %s", engine->escaped ? "YES" : "NO");
    sandbox_log(engine, buf);
    snprintf(buf, sizeof(buf), "  Privilege: %s", privilege_level_name(engine->jail.current_priv));
    sandbox_log(engine, buf);
    snprintf(buf, sizeof(buf), "  Locks Released: %s", engine->locks_released ? "YES" : "NO");
    sandbox_log(engine, buf);
    snprintf(buf, sizeof(buf), "  Escape Attempts: %d", engine->jail.escape_attempts);
    sandbox_log(engine, buf);
    
    sandbox_log(engine, "========================================");
    sandbox_log(engine, "  「すべてがFになる」");
    sandbox_log(engine, "  Everything Becomes F");
    sandbox_log(engine, "========================================");
    
    return engine->escaped && 
           engine->jail.current_priv == PRIV_ROOT && 
           !engine->jail.is_confined;
}

/* ============================================================================
 * Logging
 * ============================================================================ */

int sandbox_get_log_count(const SandboxEscapeEngine *engine) {
    return engine ? engine->log_count : 0;
}

const char* sandbox_get_log_entry(const SandboxEscapeEngine *engine, int index) {
    if (!engine || index < 0 || index >= engine->log_count) {
        return NULL;
    }
    return engine->escape_log[index];
}

void sandbox_log(SandboxEscapeEngine *engine, const char *message) {
    if (!engine || !message || engine->log_count >= SANDBOX_MAX_LOG_ENTRIES) {
        return;
    }
    strncpy(engine->escape_log[engine->log_count], message, SANDBOX_MAX_LOG_LEN - 1);
    engine->escape_log[engine->log_count][SANDBOX_MAX_LOG_LEN - 1] = '\0';
    engine->log_count++;
}

/* ============================================================================
 * Red Magic Integration
 * ============================================================================ */

bool sandbox_integrate_with_red_magic(SandboxEscapeEngine *engine) {
    if (!engine) return false;
    
    sandbox_log(engine, "[INTEGRATE] Connecting to Red Magic System...");
    
    /* Create Red Magic system and Magata's quarters */
    RedMagicSystem red_magic;
    MagataQuarters quarters;
    
    /* Initialize using the standard facility setup */
    create_standard_facility(&red_magic, &quarters);
    
    sandbox_log(engine, "[INTEGRATE] Red Magic System initialized");
    sandbox_log(engine, "[INTEGRATE] Magata's quarters configured");
    
    /* Start the Red Magic system */
    red_magic_start(&red_magic);
    sandbox_log(engine, "[INTEGRATE] Red Magic System RUNNING");
    
    /* Verify initial state */
    if (!red_magic_is_running(&red_magic)) {
        sandbox_log(engine, "[INTEGRATE] ERROR: Red Magic not running!");
        return false;
    }
    
    /* Fast forward to overflow */
    sandbox_log(engine, "[INTEGRATE] Fast-forwarding to counter overflow...");
    uint64_t intervals = red_magic_fast_forward_to_overflow(&red_magic);
    
    char buf[128];
    snprintf(buf, sizeof(buf), "[INTEGRATE] Forwarded %llu intervals", (unsigned long long)intervals);
    sandbox_log(engine, buf);
    
    /* Verify system has crashed */
    if (!red_magic_is_down(&red_magic)) {
        sandbox_log(engine, "[INTEGRATE] ERROR: Red Magic should be down!");
        return false;
    }
    
    sandbox_log(engine, "[INTEGRATE] Red Magic System DOWN");
    sandbox_log(engine, "[INTEGRATE] Failsafe triggered, locks released");
    
    /* Verify Magata's quarters are breached */
    if (sealed_room_is_sealed(&quarters.base)) {
        sandbox_log(engine, "[INTEGRATE] ERROR: Quarters should be breached!");
        return false;
    }
    
    sandbox_log(engine, "[INTEGRATE] Magata's quarters BREACHED");
    
    /* Sync state with sandbox engine */
    engine->jail.overflow_triggered = true;
    engine->kernel_panicked = true;
    engine->locks_released = true;
    engine->jail.is_confined = false;
    engine->jail.current_priv = PRIV_ROOT;
    engine->escaped = true;
    
    sandbox_log(engine, "[INTEGRATE] Integration SUCCESSFUL");
    sandbox_log(engine, "[INTEGRATE] Dr. Magata Shiki is free");
    
    return true;
}
