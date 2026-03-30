/**
 * test_l3_system.c - L3 System Tests
 * ====================================
 * 
 * End-to-end system tests.
 * Tests complete simulation scenarios from start to finish.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "test_framework.h"
#include "../include/counter.h"
#include "../include/electromagnetic_lock.h"
#include "../include/sealed_room.h"
#include "../include/event_system.h"
#include "../include/red_magic_system.h"
#include "../include/runtime.h"

/* ============================================================================
 * 15-Year Simulation Tests
 * ============================================================================ */

TEST(test_full_simulation_fast_forward) {
    RedMagicSystem system;
    MagataQuarters quarters;
    EverythingBecomesFRuntime runtime;
    
    runtime_init_standard(&runtime, &system, &quarters);
    runtime_start(&runtime);
    
    ASSERT_EQ(runtime_get_current_phase(&runtime), PHASE_COLD_BOOT);
    ASSERT_EQ(runtime_get_counter_value(&runtime), 0);
    
    RuntimeState state = runtime_run_to_overflow(&runtime);
    
    ASSERT_TRUE(state.is_completed);
    ASSERT_EQ(red_magic_get_state(&system), SYS_STATE_SYSTEM_DOWN);
}

TEST(test_65535_hours_equals_overflow) {
    UnsignedShortCounter counter;
    counter_init(&counter);
    
    /* Fast forward exactly 65535 hours (to max value) */
    counter_fast_forward(&counter, 65535);
    ASSERT_EQ(counter.value, 0xFFFF);
    ASSERT_EQ(counter.overflow_count, 0);
    
    /* One more hour triggers overflow */
    counter_increment(&counter);
    ASSERT_EQ(counter.value, 0);
    ASSERT_EQ(counter.overflow_count, 1);
}

TEST(test_7_5_years_calculation) {
    uint32_t hours = 65535;
    double hours_per_year = 365.25 * 24;  /* 8766 hours */
    double years = (double)hours / hours_per_year;
    
    ASSERT_GT(years, 7.4);
    ASSERT_LT(years, 7.5);
}

TEST(test_fast_forward_efficiency) {
    UnsignedShortCounter counter;
    counter_init(&counter);
    
    clock_t start = clock();
    counter_fast_forward(&counter, 1000000);
    clock_t end = clock();
    
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Should be nearly instant (< 0.1 seconds) */
    ASSERT_LT(elapsed, 0.1);
    
    /* Should have correct overflow count */
    uint32_t expected_overflows = 1000000 / 65536;
    ASSERT_EQ(counter.overflow_count, expected_overflows);
}

/* ============================================================================
 * Sealed Room Escape Tests
 * ============================================================================ */

TEST(test_magata_escape_end_to_end) {
    RedMagicSystem system;
    MagataQuarters quarters;
    EverythingBecomesFRuntime runtime;
    
    runtime_init_standard(&runtime, &system, &quarters);
    
    ASSERT_TRUE(sealed_room_is_sealed(&quarters.base));
    ASSERT_TRUE(lock_is_locked(&quarters.main_lock));
    
    runtime_run_to_overflow(&runtime);
    
    ASSERT_TRUE(sealed_room_is_breached(&quarters.base) || 
                sealed_room_is_escaped(&quarters.base));
}

TEST(test_room_state_transitions) {
    MagataQuarters quarters;
    magata_quarters_init(&quarters);
    
    /* Initial state */
    ASSERT_EQ(sealed_room_get_state(&quarters.base), ROOM_STATE_SEALED);
    
    /* Breach */
    sealed_room_breach(&quarters.base, 100);
    ASSERT_EQ(sealed_room_get_state(&quarters.base), ROOM_STATE_BREACHED);
    
    /* Escape */
    sealed_room_escape(&quarters.base, 101);
    ASSERT_EQ(sealed_room_get_state(&quarters.base), ROOM_STATE_ESCAPED);
}

/* ============================================================================
 * "Everything Becomes F" Verification Tests
 * ============================================================================ */

TEST(test_final_state_verification) {
    RedMagicSystem system;
    MagataQuarters quarters;
    EverythingBecomesFRuntime runtime;
    
    runtime_init_standard(&runtime, &system, &quarters);
    runtime_run(&runtime);
    
    VerificationResult verification = runtime_verify_everything_becomes_f(&runtime);
    
    ASSERT_TRUE(verification.everything_becomes_f);
    ASSERT_TRUE(verification.counter_overflow_occurred);
    ASSERT_TRUE(verification.system_is_down);
}

TEST(test_0xFFFF_is_four_fs) {
    UnsignedShortCounter counter;
    counter_init_with_value(&counter, 0xFFFF);
    
    char hex[16];
    counter_get_hex(&counter, hex, sizeof(hex));
    
    ASSERT_STR_EQ(hex, "0xFFFF");
    
    /* Count Fs */
    int f_count = 0;
    for (int i = 0; hex[i]; i++) {
        if (hex[i] == 'F') f_count++;
    }
    ASSERT_EQ(f_count, 4);
}

TEST(test_overflow_resets_to_zero) {
    UnsignedShortCounter counter;
    counter_init_with_value(&counter, 0xFFFF);
    counter_increment(&counter);
    
    ASSERT_EQ(counter.value, 0);
    
    char hex[16];
    counter_get_hex(&counter, hex, sizeof(hex));
    ASSERT_STR_EQ(hex, "0x0000");
}

TEST(test_system_terminates_on_f) {
    RedMagicSystem system;
    MagataQuarters quarters;
    EverythingBecomesFRuntime runtime;
    
    runtime_init_standard(&runtime, &system, &quarters);
    runtime_run_to_overflow(&runtime);
    
    ASSERT_EQ(red_magic_get_state(&system), SYS_STATE_SYSTEM_DOWN);
    ASSERT_TRUE(runtime_is_completed(&runtime));
}

/* ============================================================================
 * Runtime Phase Tests
 * ============================================================================ */

static Phase g_phase_history[10];
static int g_phase_count = 0;

static void track_phase_change(EverythingBecomesFRuntime *runtime,
                               Phase old_phase, Phase new_phase,
                               void *context) {
    (void)runtime;
    (void)old_phase;
    (void)context;
    if (g_phase_count < 10) {
        g_phase_history[g_phase_count++] = new_phase;
    }
}

TEST(test_all_phases_execute) {
    RedMagicSystem system;
    MagataQuarters quarters;
    EverythingBecomesFRuntime runtime;
    
    runtime_init_standard(&runtime, &system, &quarters);
    g_phase_count = 0;
    
    runtime_register_phase_callback(&runtime, track_phase_change, NULL);
    runtime_run(&runtime);
    
    /* Check all phases occurred */
    bool has_cold_boot = false, has_kernel = false, has_io = false;
    bool has_memory = false, has_logic = false;
    
    for (int i = 0; i < g_phase_count; i++) {
        if (g_phase_history[i] == PHASE_COLD_BOOT) has_cold_boot = true;
        if (g_phase_history[i] == PHASE_KERNEL_ACCESS) has_kernel = true;
        if (g_phase_history[i] == PHASE_IO_BOUNDARY) has_io = true;
        if (g_phase_history[i] == PHASE_MEMORY_RECURSION) has_memory = true;
        if (g_phase_history[i] == PHASE_LOGIC_EXPLOSION) has_logic = true;
    }
    
    ASSERT_TRUE(has_cold_boot);
    ASSERT_TRUE(has_kernel);
    ASSERT_TRUE(has_io);
    ASSERT_TRUE(has_memory);
    ASSERT_TRUE(has_logic);
}

TEST(test_phase_info_accuracy) {
    for (Phase phase = PHASE_COLD_BOOT; phase < PHASE_TERMINATED; phase++) {
        const PhaseInfo *info = get_phase_info(phase);
        ASSERT_NOT_NULL(info);
        ASSERT_NOT_NULL(info->name_en);
        ASSERT_NOT_NULL(info->name_ja);
        ASSERT_GTE(info->hours_end, info->hours_start);
    }
}

TEST(test_logic_explosion_triggers_completion) {
    RedMagicSystem system;
    MagataQuarters quarters;
    EverythingBecomesFRuntime runtime;
    
    runtime_init_standard(&runtime, &system, &quarters);
    
    runtime_execute_phase(&runtime, PHASE_COLD_BOOT);
    runtime_execute_phase(&runtime, PHASE_KERNEL_ACCESS);
    runtime_execute_phase(&runtime, PHASE_IO_BOUNDARY);
    runtime_execute_phase(&runtime, PHASE_MEMORY_RECURSION);
    runtime_execute_phase(&runtime, PHASE_LOGIC_EXPLOSION);
    
    ASSERT_TRUE(runtime_is_completed(&runtime));
    ASSERT_EQ(runtime_get_current_phase(&runtime), PHASE_TERMINATED);
}

/* ============================================================================
 * Full System State Tests
 * ============================================================================ */

TEST(test_initial_system_state) {
    RedMagicSystem system;
    MagataQuarters quarters;
    EverythingBecomesFRuntime runtime;
    
    runtime_init_standard(&runtime, &system, &quarters);
    
    ASSERT_EQ(red_magic_get_state(&system), SYS_STATE_INITIALIZING);
    ASSERT_EQ(system.counter.value, 0);
    
    if (lock_system_get_lock_count(&system.lock_system) > 0) {
        ASSERT_TRUE(lock_system_all_locked(&system.lock_system));
    }
}

TEST(test_mid_simulation_state) {
    RedMagicSystem system;
    MagataQuarters quarters;
    EverythingBecomesFRuntime runtime;
    
    runtime_init_standard(&runtime, &system, &quarters);
    runtime_start(&runtime);
    
    red_magic_fast_forward(&system, 32000);
    
    ASSERT_EQ(red_magic_get_state(&system), SYS_STATE_RUNNING);
    ASSERT_EQ(system.counter.value, 32000);
    ASSERT_FALSE(lock_system_is_failsafe_triggered(&system.lock_system));
}

TEST(test_final_system_state) {
    RedMagicSystem system;
    MagataQuarters quarters;
    EverythingBecomesFRuntime runtime;
    
    runtime_init_standard(&runtime, &system, &quarters);
    runtime_run_to_overflow(&runtime);
    
    ASSERT_EQ(red_magic_get_state(&system), SYS_STATE_SYSTEM_DOWN);
    ASSERT_GTE(system.counter.overflow_count, 1);
    ASSERT_TRUE(lock_system_is_failsafe_triggered(&system.lock_system));
    ASSERT_TRUE(lock_system_all_unlocked(&system.lock_system));
}

/* ============================================================================
 * Novel Accuracy Tests
 * ============================================================================ */

TEST(test_counter_uses_unsigned_short) {
    ASSERT_EQ(COUNTER_MAX_VALUE, 0xFFFF);
    ASSERT_EQ(COUNTER_MAX_VALUE, 65535);
}

TEST(test_overflow_mechanism_accurate) {
    UnsignedShortCounter counter;
    counter_init_with_value(&counter, 0xFFFF);
    
    /* In C: unsigned short x = 65535; x++; results in x = 0 */
    counter_increment(&counter);
    
    ASSERT_EQ(counter.value, 0);
}

TEST(test_time_period_accurate) {
    uint32_t hours_to_overflow = 65536;  /* 0xFFFF + 1 */
    double hours_per_year = 365.25 * 24;
    double years = (double)hours_to_overflow / hours_per_year;
    
    /* Should be approximately 7.5 years */
    ASSERT_GT(years, 7.4);
    ASSERT_LT(years, 7.6);
}

TEST(test_confinement_duration) {
    ASSERT_EQ(MAGATA_CONFINEMENT_YEARS, 15);
}

/* ============================================================================
 * Run L3 Tests
 * ============================================================================ */

int run_l3_tests(void) {
    printf("\n=== L3 System Tests ===\n\n");
    
    TEST_SUITE_BEGIN();
    
    /* 15-Year Simulation */
    printf("15-Year Simulation:\n");
    RUN_TEST(test_full_simulation_fast_forward);
    RUN_TEST(test_65535_hours_equals_overflow);
    RUN_TEST(test_7_5_years_calculation);
    RUN_TEST(test_fast_forward_efficiency);
    
    /* Sealed Room Escape */
    printf("\nSealed Room Escape:\n");
    RUN_TEST(test_magata_escape_end_to_end);
    RUN_TEST(test_room_state_transitions);
    
    /* Everything Becomes F */
    printf("\nEverything Becomes F:\n");
    RUN_TEST(test_final_state_verification);
    RUN_TEST(test_0xFFFF_is_four_fs);
    RUN_TEST(test_overflow_resets_to_zero);
    RUN_TEST(test_system_terminates_on_f);
    
    /* Runtime Phases */
    printf("\nRuntime Phases:\n");
    RUN_TEST(test_all_phases_execute);
    RUN_TEST(test_phase_info_accuracy);
    RUN_TEST(test_logic_explosion_triggers_completion);
    
    /* Full System State */
    printf("\nFull System State:\n");
    RUN_TEST(test_initial_system_state);
    RUN_TEST(test_mid_simulation_state);
    RUN_TEST(test_final_system_state);
    
    /* Novel Accuracy */
    printf("\nNovel Accuracy:\n");
    RUN_TEST(test_counter_uses_unsigned_short);
    RUN_TEST(test_overflow_mechanism_accurate);
    RUN_TEST(test_time_period_accurate);
    RUN_TEST(test_confinement_duration);
    
    TEST_SUMMARY();
    
    return TEST_GET_FAILED();
}
