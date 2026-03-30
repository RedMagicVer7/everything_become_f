/**
 * main.c - Everything Becomes F Main Program
 * ============================================
 * 
 * Entry point for the Red Magic system simulation.
 * Runs the complete 5-phase simulation from system boot
 * to "Everything Becomes F" conclusion.
 * 
 * Based on the novel "すべてがFになる" (The Perfect Insider)
 * by Hiroshi Mori, 1996.
 */

#include <stdio.h>
#include <string.h>
#include "../include/runtime.h"

/* Phase change callback for logging */
static void on_phase_change(EverythingBecomesFRuntime *runtime,
                            Phase old_phase, Phase new_phase,
                            void *context) {
    (void)runtime;
    (void)old_phase;
    (void)context;
    
    const PhaseInfo *info = get_phase_info(new_phase);
    if (info) {
        printf("\n=== Phase: %s (%s) ===\n", info->name_en, info->name_ja);
        printf("    %s\n", info->description);
    }
}

/* Print system status */
static void print_system_status(RedMagicSystem *system) {
    char hex[8];
    counter_get_hex(&system->counter, hex, sizeof(hex));
    
    printf("\nSystem Status:\n");
    printf("  State: %s\n", system_state_name(system->state));
    printf("  Counter: %s (%u)\n", hex, system->counter.value);
    printf("  Overflow count: %u\n", system->counter.overflow_count);
    printf("  Locks: %d total, %d locked, %d unlocked\n",
           lock_system_get_lock_count(&system->lock_system),
           lock_system_get_locked_count(&system->lock_system),
           lock_system_get_unlocked_count(&system->lock_system));
    printf("  Failsafe triggered: %s\n",
           lock_system_is_failsafe_triggered(&system->lock_system) ? "YES" : "NO");
}

/* Print final verification */
static void print_verification(VerificationResult *result) {
    printf("\n");
    printf("===========================================\n");
    printf("       EVERYTHING BECOMES F: %s\n",
           result->everything_becomes_f ? "TRUE" : "FALSE");
    printf("===========================================\n");
    printf("\n");
    printf("Verification Details:\n");
    printf("  Counter overflow occurred: %s\n",
           result->counter_overflow_occurred ? "YES" : "NO");
    printf("  Overflow count: %u\n", result->overflow_count);
    printf("  System state: %s\n", system_state_name(result->system_state));
    printf("  System is down: %s\n", result->system_is_down ? "YES" : "NO");
    printf("  Escaped: %s\n", result->escaped ? "YES" : "NO");
    printf("  Final counter value: %s (%u)\n",
           result->final_counter_hex, result->final_counter_value);
    printf("\n");
    printf("Meaning:\n");
    printf("  0xFFFF (65535) represents the maximum value.\n");
    printf("  When incremented, it overflows to 0x0000.\n");
    printf("  In hexadecimal, F is the highest digit.\n");
    printf("  Everything becoming F means reaching the limit,\n");
    printf("  then returning to zero - the perfect cycle.\n");
    printf("\n");
}

int main(void) {
    printf("============================================\n");
    printf("   Everything Becomes F - Red Magic System\n");
    printf("   すべてがFになる - Red Magicシステム\n");
    printf("============================================\n\n");
    
    printf("Based on the novel by Hiroshi Mori (1996)\n");
    printf("Simulating the 16-bit unsigned short overflow bug\n");
    printf("that enabled Dr. Magata Shiki's escape.\n\n");
    
    /* Create system and quarters */
    RedMagicSystem system;
    MagataQuarters quarters;
    EverythingBecomesFRuntime runtime;
    
    /* Initialize with standard facility */
    runtime_init_standard(&runtime, &system, &quarters);
    
    /* Register phase change callback */
    runtime_register_phase_callback(&runtime, on_phase_change, NULL);
    
    printf("System initialized.\n");
    printf("Counter starts at 0x0000\n");
    printf("Maximum value: 0xFFFF (65535)\n");
    printf("Time to overflow: ~%.2f years (65536 hours)\n\n",
           counter_years_until_overflow(&system.counter));
    
    printf("Starting simulation...\n");
    
    /* Run complete simulation */
    RuntimeState state = runtime_run(&runtime);
    
    /* Print final status */
    print_system_status(&system);
    
    /* Verify "Everything Becomes F" */
    VerificationResult verification = runtime_verify_everything_becomes_f(&runtime);
    print_verification(&verification);
    
    /* Print conclusion */
    if (verification.everything_becomes_f) {
        printf("============================================\n");
        printf("          全部成为F / すべてがFになる\n");
        printf("            The Perfect Insider\n");
        printf("============================================\n\n");
        
        printf("After %u hours (~%.1f years), the counter\n",
               (unsigned)(COUNTER_MAX_VALUE + 1),
               (double)(COUNTER_MAX_VALUE + 1) / HOURS_PER_YEAR);
        printf("overflowed from 0xFFFF to 0x0000.\n\n");
        printf("The failsafe triggered.\n");
        printf("All electromagnetic locks released.\n");
        printf("The sealed room was breached.\n");
        printf("Dr. Magata Shiki escaped.\n\n");
        printf("Everything has become F.\n");
    }
    
    return state.is_completed ? 0 : 1;
}
