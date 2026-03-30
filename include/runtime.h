/**
 * runtime.h - Everything Becomes F Runtime Engine
 * =================================================
 * 
 * Maps the novel's 11 episodes to system state transitions,
 * simulating the complete narrative arc from system boot
 * to final "Everything Becomes F" conclusion.
 * 
 * The novel's episodes (色彩 - colors) map to system phases:
 * - Phase 1 (Cold Boot): White Conference, Amber Court
 * - Phase 2 (Kernel Access): Magic Red, Seven Diamonds
 * - Phase 3 (I/O Boundary): Blue Water, Yellow Edge, Silver Boundary
 * - Phase 4 (Memory Recursion): Green Origin, Purple Memory
 * - Phase 5 (Logic Explosion): Deep Red Square, Everything Becomes F
 */

#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdbool.h>
#include <stdint.h>
#include "red_magic_system.h"
#include "sealed_room.h"

#define MAX_PHASE_NAME_LEN 64
#define MAX_DESCRIPTION_LEN 256
#define MAX_PHASE_CALLBACKS 4
#define MAX_COMPLETION_CALLBACKS 4
#define MAX_PHASE_HISTORY 10

/**
 * Phase - Simulation phases mapped to novel episodes
 */
typedef enum {
    /* Phase 1: Cold Boot - System initialization */
    PHASE_COLD_BOOT,
    
    /* Phase 2: Kernel Access - Core system running */
    PHASE_KERNEL_ACCESS,
    
    /* Phase 3: I/O Boundary - Time passing */
    PHASE_IO_BOUNDARY,
    
    /* Phase 4: Memory Recursion - Near overflow */
    PHASE_MEMORY_RECURSION,
    
    /* Phase 5: Logic Explosion - Overflow, escape */
    PHASE_LOGIC_EXPLOSION,
    
    /* Terminal state */
    PHASE_TERMINATED,
    
    PHASE_COUNT
} Phase;

/* Get phase name in English */
const char *phase_name_en(Phase phase);

/* Get phase name in Japanese */
const char *phase_name_ja(Phase phase);

/**
 * PhaseInfo - Information about a simulation phase
 */
typedef struct {
    Phase phase;
    const char *name_en;
    const char *name_ja;
    const char *description;
    uint32_t hours_start;
    uint32_t hours_end;
} PhaseInfo;

/* Get phase info */
const PhaseInfo *get_phase_info(Phase phase);

/**
 * RuntimeState - Current state of the runtime
 */
typedef struct {
    Phase phase;
    SystemState system_state;
    uint16_t counter_value;
    char counter_hex[8];
    bool is_completed;
} RuntimeState;

/* Forward declaration */
typedef struct EverythingBecomesFRuntime EverythingBecomesFRuntime;

/* Callback types */
typedef void (*phase_change_callback_t)(EverythingBecomesFRuntime *runtime,
                                        Phase old_phase, Phase new_phase,
                                        void *context);
typedef void (*completion_callback_t)(EverythingBecomesFRuntime *runtime, void *context);

/**
 * EverythingBecomesFRuntime - Runtime engine for the simulation
 */
struct EverythingBecomesFRuntime {
    RedMagicSystem *system;
    MagataQuarters *magata_quarters;
    Phase current_phase;
    Phase phase_history[MAX_PHASE_HISTORY];
    int phase_history_count;
    bool is_running;
    bool is_completed;
    bool owns_system;  /* Whether runtime owns the system memory */
    
    phase_change_callback_t phase_callbacks[MAX_PHASE_CALLBACKS];
    void *phase_callback_contexts[MAX_PHASE_CALLBACKS];
    int phase_callback_count;
    
    completion_callback_t completion_callbacks[MAX_COMPLETION_CALLBACKS];
    void *completion_callback_contexts[MAX_COMPLETION_CALLBACKS];
    int completion_callback_count;
};

/* Initialize runtime with existing system */
void runtime_init(EverythingBecomesFRuntime *runtime, 
                  RedMagicSystem *system,
                  MagataQuarters *quarters);

/* Initialize runtime with new standard facility */
void runtime_init_standard(EverythingBecomesFRuntime *runtime,
                           RedMagicSystem *system,
                           MagataQuarters *quarters);

/* Start the simulation */
void runtime_start(EverythingBecomesFRuntime *runtime);

/* Execute a specific phase */
bool runtime_execute_phase(EverythingBecomesFRuntime *runtime, Phase phase);

/* Advance to next phase */
Phase runtime_advance_to_next_phase(EverythingBecomesFRuntime *runtime);

/* Run complete simulation */
RuntimeState runtime_run(EverythingBecomesFRuntime *runtime);

/* Run directly to overflow */
RuntimeState runtime_run_to_overflow(EverythingBecomesFRuntime *runtime);

/* Get current state */
RuntimeState runtime_get_state(const EverythingBecomesFRuntime *runtime);

/* State queries */
Phase runtime_get_current_phase(const EverythingBecomesFRuntime *runtime);
bool runtime_is_running(const EverythingBecomesFRuntime *runtime);
bool runtime_is_completed(const EverythingBecomesFRuntime *runtime);
uint16_t runtime_get_counter_value(const EverythingBecomesFRuntime *runtime);
void runtime_get_counter_hex(const EverythingBecomesFRuntime *runtime, char *buf, size_t len);

/* Get underlying system */
RedMagicSystem *runtime_get_system(EverythingBecomesFRuntime *runtime);

/* Register callbacks */
bool runtime_register_phase_callback(EverythingBecomesFRuntime *runtime,
                                     phase_change_callback_t cb, void *ctx);
bool runtime_register_completion_callback(EverythingBecomesFRuntime *runtime,
                                          completion_callback_t cb, void *ctx);

/**
 * VerificationResult - Result of "Everything Becomes F" verification
 */
typedef struct {
    bool everything_becomes_f;
    bool counter_overflow_occurred;
    uint32_t overflow_count;
    SystemState system_state;
    bool system_is_down;
    bool escaped;
    uint16_t final_counter_value;
    char final_counter_hex[8];
} VerificationResult;

/* Verify "Everything Becomes F" condition */
VerificationResult runtime_verify_everything_becomes_f(const EverythingBecomesFRuntime *runtime);

/* Reset runtime to initial state */
void runtime_reset(EverythingBecomesFRuntime *runtime);

#endif /* RUNTIME_H */
