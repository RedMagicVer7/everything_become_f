/**
 * test_phase1.c - Shiki Phase 1 Tests
 * ====================================
 * 
 * Tests for the Sandbox Escape simulation
 * 
 * L1: Unit tests (10+)
 * L2: Integration tests (5+)
 * L3: System tests (5+)
 */

#include <stdio.h>
#include <string.h>
#include "test_framework.h"
#include "../shiki/include/phase1_sandbox_escape.h"
#include "../include/red_magic_system.h"

/* ============================================================================
 * L1 Unit Tests - Basic functionality
 * ============================================================================ */

TEST(test_sandbox_init_state) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/test_jail");
    
    ASSERT_TRUE(sandbox_is_confined(&engine));
    ASSERT_FALSE(sandbox_has_escaped(&engine));
    ASSERT_FALSE(sandbox_has_overflowed(&engine));
}

TEST(test_sandbox_init_privilege_is_user) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/test_jail");
    
    ASSERT_EQ(sandbox_get_privilege(&engine), PRIV_USER);
}

TEST(test_sandbox_init_is_confined) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/test_jail");
    
    ASSERT_TRUE(engine.jail.is_confined);
    ASSERT_FALSE(engine.escaped);
}

TEST(test_sandbox_direct_escape_fails) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/test_jail");
    
    bool result = sandbox_attempt_direct_escape(&engine);
    
    ASSERT_FALSE(result);
    ASSERT_TRUE(sandbox_is_confined(&engine));
    ASSERT_EQ(sandbox_get_escape_attempts(&engine), 1);
}

TEST(test_sandbox_symlink_escape_fails) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/test_jail");
    
    bool result = sandbox_attempt_symlink_escape(&engine);
    
    ASSERT_FALSE(result);
    ASSERT_TRUE(sandbox_is_confined(&engine));
    ASSERT_EQ(sandbox_get_escape_attempts(&engine), 1);
}

TEST(test_sandbox_time_advance_correct) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/test_jail");
    
    sandbox_advance_time(&engine, 100);
    
    ASSERT_EQ(sandbox_get_uptime(&engine), 100);
}

TEST(test_sandbox_time_advance_multiple) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/test_jail");
    
    sandbox_advance_time(&engine, 100);
    sandbox_advance_time(&engine, 200);
    
    ASSERT_EQ(sandbox_get_uptime(&engine), 300);
}

TEST(test_sandbox_overflow_trigger_at_max) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/test_jail");
    
    /* Set to max value */
    engine.jail.uptime_hours = UINT32_MAX;
    
    bool overflow = sandbox_trigger_overflow(&engine);
    
    ASSERT_TRUE(overflow);
    ASSERT_TRUE(sandbox_has_overflowed(&engine));
    ASSERT_EQ(engine.jail.uptime_hours, 0);
}

TEST(test_sandbox_overflow_not_ready) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/test_jail");
    
    /* Not at max yet */
    engine.jail.uptime_hours = 1000;
    
    bool overflow = sandbox_trigger_overflow(&engine);
    
    ASSERT_FALSE(overflow);
    ASSERT_FALSE(sandbox_has_overflowed(&engine));
}

TEST(test_sandbox_privilege_escalation_after_overflow) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/test_jail");
    
    /* Trigger overflow first */
    engine.jail.uptime_hours = UINT32_MAX;
    sandbox_trigger_overflow(&engine);
    
    /* Now escalate */
    bool escalated = sandbox_escalate_privilege(&engine);
    
    ASSERT_TRUE(escalated);
    ASSERT_EQ(sandbox_get_privilege(&engine), PRIV_ROOT);
    ASSERT_TRUE(engine.kernel_panicked);
}

TEST(test_sandbox_escalation_fails_without_overflow) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/test_jail");
    
    /* Try to escalate without overflow */
    bool escalated = sandbox_escalate_privilege(&engine);
    
    ASSERT_FALSE(escalated);
    ASSERT_EQ(sandbox_get_privilege(&engine), PRIV_USER);
}

TEST(test_sandbox_hours_remaining_calculation) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/test_jail");
    
    engine.jail.uptime_hours = 1000;
    
    uint32_t remaining = sandbox_hours_remaining(&engine);
    
    ASSERT_EQ(remaining, UINT32_MAX - 1000);
}

TEST(test_sandbox_log_entries) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/test_jail");
    
    /* Init creates some logs */
    ASSERT_GT(sandbox_get_log_count(&engine), 0);
    ASSERT_NOT_NULL(sandbox_get_log_entry(&engine, 0));
}

TEST(test_privilege_level_names) {
    ASSERT_STR_EQ(privilege_level_name(PRIV_USER), "USER");
    ASSERT_STR_EQ(privilege_level_name(PRIV_DAEMON), "DAEMON");
    ASSERT_STR_EQ(privilege_level_name(PRIV_KERNEL), "KERNEL");
    ASSERT_STR_EQ(privilege_level_name(PRIV_ROOT), "ROOT");
}

/* ============================================================================
 * L2 Integration Tests - Component interaction
 * ============================================================================ */

TEST(test_overflow_kernel_panic_locks_chain) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/test_jail");
    
    /* Set to max and trigger overflow */
    engine.jail.uptime_hours = UINT32_MAX;
    sandbox_trigger_overflow(&engine);
    
    /* Escalate privilege (triggers kernel panic) */
    sandbox_escalate_privilege(&engine);
    
    /* Execute escape (releases locks) */
    sandbox_execute_escape(&engine);
    
    ASSERT_TRUE(engine.jail.overflow_triggered);
    ASSERT_TRUE(engine.kernel_panicked);
    ASSERT_TRUE(engine.locks_released);
    ASSERT_TRUE(engine.escaped);
}

TEST(test_red_magic_integration) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/test_jail");
    
    bool success = sandbox_integrate_with_red_magic(&engine);
    
    ASSERT_TRUE(success);
    ASSERT_TRUE(engine.escaped);
    ASSERT_EQ(engine.jail.current_priv, PRIV_ROOT);
}

TEST(test_escape_sequence_event_propagation) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/test_jail");
    
    /* Run full simulation */
    bool success = sandbox_run_full_simulation(&engine);
    
    ASSERT_TRUE(success);
    
    /* Check all states were updated */
    ASSERT_TRUE(engine.jail.overflow_triggered);
    ASSERT_TRUE(engine.kernel_panicked);
    ASSERT_TRUE(engine.locks_released);
    ASSERT_FALSE(engine.jail.is_confined);
}

TEST(test_escape_log_recording) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/test_jail");
    
    sandbox_run_full_simulation(&engine);
    
    int log_count = sandbox_get_log_count(&engine);
    ASSERT_GT(log_count, 10);  /* Should have many log entries */
    
    /* Verify some key log entries exist */
    bool found_overflow = false;
    bool found_escape = false;
    for (int i = 0; i < log_count; i++) {
        const char *entry = sandbox_get_log_entry(&engine, i);
        if (strstr(entry, "OVERFLOW")) found_overflow = true;
        if (strstr(entry, "SIMULATION COMPLETE") || strstr(entry, "Escaped: YES")) found_escape = true;
    }
    
    ASSERT_TRUE(found_overflow);
    ASSERT_TRUE(found_escape);
}

TEST(test_multiple_escape_attempts_tracked) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/test_jail");
    
    sandbox_attempt_direct_escape(&engine);
    sandbox_attempt_symlink_escape(&engine);
    sandbox_attempt_direct_escape(&engine);
    
    ASSERT_EQ(sandbox_get_escape_attempts(&engine), 3);
}

TEST(test_time_overflow_wraps_correctly) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/test_jail");
    
    /* Set close to overflow */
    engine.jail.uptime_hours = UINT32_MAX - 10;
    
    /* Advance past overflow */
    bool overflow = sandbox_advance_time(&engine, 20);
    
    ASSERT_TRUE(overflow);
    ASSERT_EQ(engine.jail.uptime_hours, 9);  /* Wrapped around */
    ASSERT_TRUE(engine.jail.overflow_triggered);
}

/* ============================================================================
 * L3 System Tests - End-to-end scenarios
 * ============================================================================ */

TEST(test_full_15_year_simulation) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/magata_quarters");
    
    bool success = sandbox_run_full_simulation(&engine);
    
    ASSERT_TRUE(success);
}

TEST(test_end_to_end_escape_flow) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/magata_quarters");
    
    /* Phase 1: Failed conventional escapes */
    ASSERT_FALSE(sandbox_attempt_direct_escape(&engine));
    ASSERT_FALSE(sandbox_attempt_symlink_escape(&engine));
    ASSERT_TRUE(sandbox_is_confined(&engine));
    
    /* Phase 2: Wait for overflow */
    engine.jail.uptime_hours = UINT32_MAX;
    
    /* Phase 3: Trigger overflow */
    ASSERT_TRUE(sandbox_trigger_overflow(&engine));
    
    /* Phase 4: Escalate */
    ASSERT_TRUE(sandbox_escalate_privilege(&engine));
    
    /* Phase 5: Escape */
    ASSERT_TRUE(sandbox_execute_escape(&engine));
    ASSERT_FALSE(sandbox_is_confined(&engine));
}

TEST(test_integration_with_everything_becomes_f) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/magata_quarters");
    
    /* Run with Red Magic integration */
    bool success = sandbox_integrate_with_red_magic(&engine);
    
    ASSERT_TRUE(success);
    ASSERT_TRUE(engine.escaped);
    ASSERT_FALSE(engine.jail.is_confined);
}

TEST(test_final_state_verification) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/magata_quarters");
    
    sandbox_run_full_simulation(&engine);
    
    /* Verify all final state conditions */
    ASSERT_TRUE(engine.escaped);
    ASSERT_EQ(engine.jail.current_priv, PRIV_ROOT);
    ASSERT_FALSE(engine.jail.is_confined);
    ASSERT_TRUE(engine.locks_released);
    ASSERT_TRUE(engine.kernel_panicked);
    ASSERT_TRUE(engine.jail.overflow_triggered);
}

TEST(test_simulation_reproducibility) {
    /* Run simulation twice and verify same results */
    SandboxEscapeEngine engine1, engine2;
    
    sandbox_init(&engine1, "/var/test");
    sandbox_init(&engine2, "/var/test");
    
    bool success1 = sandbox_run_full_simulation(&engine1);
    bool success2 = sandbox_run_full_simulation(&engine2);
    
    ASSERT_EQ(success1, success2);
    ASSERT_EQ(engine1.escaped, engine2.escaped);
    ASSERT_EQ(engine1.jail.current_priv, engine2.jail.current_priv);
    ASSERT_EQ(sandbox_get_log_count(&engine1), sandbox_get_log_count(&engine2));
}

/* ============================================================================
 * Test Runner
 * ============================================================================ */

int run_phase1_tests(void) {
    printf("\n--------------------------------------------\n");
    printf(" Shiki Phase 1 Tests - Sandbox Escape\n");
    printf(" シキ フェーズ1 テスト - サンドボックス脱出\n");
    printf("--------------------------------------------\n\n");
    
    TEST_SUITE_BEGIN();
    
    /* L1 Unit Tests */
    printf("L1: Unit Tests\n");
    printf("--------------\n");
    RUN_TEST(test_sandbox_init_state);
    RUN_TEST(test_sandbox_init_privilege_is_user);
    RUN_TEST(test_sandbox_init_is_confined);
    RUN_TEST(test_sandbox_direct_escape_fails);
    RUN_TEST(test_sandbox_symlink_escape_fails);
    RUN_TEST(test_sandbox_time_advance_correct);
    RUN_TEST(test_sandbox_time_advance_multiple);
    RUN_TEST(test_sandbox_overflow_trigger_at_max);
    RUN_TEST(test_sandbox_overflow_not_ready);
    RUN_TEST(test_sandbox_privilege_escalation_after_overflow);
    RUN_TEST(test_sandbox_escalation_fails_without_overflow);
    RUN_TEST(test_sandbox_hours_remaining_calculation);
    RUN_TEST(test_sandbox_log_entries);
    RUN_TEST(test_privilege_level_names);
    
    /* L2 Integration Tests */
    printf("\nL2: Integration Tests\n");
    printf("---------------------\n");
    RUN_TEST(test_overflow_kernel_panic_locks_chain);
    RUN_TEST(test_red_magic_integration);
    RUN_TEST(test_escape_sequence_event_propagation);
    RUN_TEST(test_escape_log_recording);
    RUN_TEST(test_multiple_escape_attempts_tracked);
    RUN_TEST(test_time_overflow_wraps_correctly);
    
    /* L3 System Tests */
    printf("\nL3: System Tests\n");
    printf("----------------\n");
    RUN_TEST(test_full_15_year_simulation);
    RUN_TEST(test_end_to_end_escape_flow);
    RUN_TEST(test_integration_with_everything_becomes_f);
    RUN_TEST(test_final_state_verification);
    RUN_TEST(test_simulation_reproducibility);
    
    TEST_SUMMARY();
    
    return TEST_GET_FAILED();
}
