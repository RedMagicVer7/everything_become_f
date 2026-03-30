/**
 * runtime.c - Everything Becomes F Runtime Engine Implementation
 */

#include <stdio.h>
#include <string.h>
#include "../include/runtime.h"

/* Phase info data */
static const PhaseInfo PHASE_INFO_DATA[] = {
    {
        PHASE_COLD_BOOT,
        "Cold Boot",
        "コールドブート",
        "System initialization. Dr. Magata begins her confinement.",
        0, 1000
    },
    {
        PHASE_KERNEL_ACCESS,
        "Kernel Access",
        "カーネルアクセス",
        "Core system running. The bug is planted in Red Magic.",
        1000, 20000
    },
    {
        PHASE_IO_BOUNDARY,
        "I/O Boundary",
        "入出力境界",
        "Time passes. Counter approaches half-way point.",
        20000, 45000
    },
    {
        PHASE_MEMORY_RECURSION,
        "Memory Recursion",
        "記憶の再帰",
        "Near overflow. Memory and time recursive patterns emerge.",
        45000, 65534
    },
    {
        PHASE_LOGIC_EXPLOSION,
        "Logic Explosion",
        "論理の爆発",
        "Counter reaches 0xFFFF. Overflow. Escape. System crash.",
        65534, 65536
    },
    {
        PHASE_TERMINATED,
        "Terminated",
        "終了",
        "System has crashed. Everything has become F.",
        65536, 65536
    }
};

const char *phase_name_en(Phase phase) {
    if (phase >= 0 && phase < PHASE_COUNT) {
        return PHASE_INFO_DATA[phase].name_en;
    }
    return "Unknown";
}

const char *phase_name_ja(Phase phase) {
    if (phase >= 0 && phase < PHASE_COUNT) {
        return PHASE_INFO_DATA[phase].name_ja;
    }
    return "不明";
}

const PhaseInfo *get_phase_info(Phase phase) {
    if (phase >= 0 && phase < PHASE_COUNT) {
        return &PHASE_INFO_DATA[phase];
    }
    return NULL;
}

/* ============================================================================
 * Internal Helpers
 * ============================================================================ */

static void transition_phase(EverythingBecomesFRuntime *runtime, Phase new_phase) {
    Phase old_phase = runtime->current_phase;
    runtime->current_phase = new_phase;
    
    /* Add to history */
    if (runtime->phase_history_count < MAX_PHASE_HISTORY) {
        runtime->phase_history[runtime->phase_history_count] = new_phase;
        runtime->phase_history_count++;
    }
    
    /* Emit phase transition event */
    event_bus_emit(&runtime->system->event_bus, EVT_PHASE_TRANSITION,
                   "Runtime", runtime->system->counter.value);
    
    /* Call callbacks */
    for (int i = 0; i < runtime->phase_callback_count; i++) {
        if (runtime->phase_callbacks[i]) {
            runtime->phase_callbacks[i](runtime, old_phase, new_phase,
                                        runtime->phase_callback_contexts[i]);
        }
    }
}

static void complete_simulation(EverythingBecomesFRuntime *runtime) {
    runtime->is_completed = true;
    runtime->is_running = false;
    runtime->current_phase = PHASE_TERMINATED;
    
    /* Handle escape if quarters exist */
    if (runtime->magata_quarters) {
        SealedRoom *room = &runtime->magata_quarters->base;
        if (sealed_room_is_breached(room) && !sealed_room_is_escaped(room)) {
            sealed_room_escape(room, runtime->system->counter.value);
            event_bus_emit(&runtime->system->event_bus, EVT_ROOM_ESCAPED,
                           "MagataQuarters", runtime->system->counter.value);
        }
    }
    
    /* Call completion callbacks */
    for (int i = 0; i < runtime->completion_callback_count; i++) {
        if (runtime->completion_callbacks[i]) {
            runtime->completion_callbacks[i](runtime,
                                             runtime->completion_callback_contexts[i]);
        }
    }
}

/* ============================================================================
 * Runtime Implementation
 * ============================================================================ */

void runtime_init(EverythingBecomesFRuntime *runtime,
                  RedMagicSystem *system,
                  MagataQuarters *quarters) {
    if (!runtime || !system) return;
    
    memset(runtime, 0, sizeof(EverythingBecomesFRuntime));
    
    runtime->system = system;
    runtime->magata_quarters = quarters;
    runtime->current_phase = PHASE_COLD_BOOT;
    runtime->phase_history_count = 0;
    runtime->is_running = false;
    runtime->is_completed = false;
    runtime->owns_system = false;
    runtime->phase_callback_count = 0;
    runtime->completion_callback_count = 0;
}

void runtime_init_standard(EverythingBecomesFRuntime *runtime,
                           RedMagicSystem *system,
                           MagataQuarters *quarters) {
    if (!runtime || !system || !quarters) return;
    
    create_standard_facility(system, quarters);
    runtime_init(runtime, system, quarters);
}

void runtime_start(EverythingBecomesFRuntime *runtime) {
    if (!runtime || runtime->is_running) return;
    
    runtime->is_running = true;
    red_magic_start(runtime->system);
    transition_phase(runtime, PHASE_COLD_BOOT);
}

bool runtime_execute_phase(EverythingBecomesFRuntime *runtime, Phase phase) {
    if (!runtime || runtime->is_completed) return false;
    if (phase < 0 || phase >= PHASE_COUNT) return false;
    
    if (!runtime->is_running) {
        runtime_start(runtime);
    }
    
    transition_phase(runtime, phase);
    
    const PhaseInfo *info = get_phase_info(phase);
    if (!info) return false;
    
    /* Calculate hours to advance */
    uint16_t current_hour = runtime->system->counter.value;
    
    if (current_hour < info->hours_start) {
        /* Fast forward to phase start */
        uint32_t hours_to_start = info->hours_start - current_hour;
        red_magic_fast_forward(runtime->system, hours_to_start);
    }
    
    /* Fast forward through the phase */
    current_hour = runtime->system->counter.value;
    uint32_t hours_in_phase = 0;
    if (info->hours_end > current_hour) {
        hours_in_phase = info->hours_end - current_hour;
    }
    
    if (hours_in_phase > 0) {
        red_magic_fast_forward(runtime->system, hours_in_phase);
    }
    
    /* Check if this triggers completion */
    if (phase == PHASE_LOGIC_EXPLOSION) {
        complete_simulation(runtime);
    }
    
    return true;
}

Phase runtime_advance_to_next_phase(EverythingBecomesFRuntime *runtime) {
    if (!runtime || runtime->is_completed) return PHASE_TERMINATED;
    
    Phase next_phase = (Phase)(runtime->current_phase + 1);
    if (next_phase >= PHASE_TERMINATED) {
        return PHASE_TERMINATED;
    }
    
    runtime_execute_phase(runtime, next_phase);
    return next_phase;
}

RuntimeState runtime_run(EverythingBecomesFRuntime *runtime) {
    RuntimeState state;
    memset(&state, 0, sizeof(RuntimeState));
    
    if (!runtime) return state;
    
    runtime_start(runtime);
    
    /* Execute all phases in order */
    for (Phase phase = PHASE_COLD_BOOT; phase < PHASE_TERMINATED; phase++) {
        runtime_execute_phase(runtime, phase);
    }
    
    return runtime_get_state(runtime);
}

RuntimeState runtime_run_to_overflow(EverythingBecomesFRuntime *runtime) {
    RuntimeState state;
    memset(&state, 0, sizeof(RuntimeState));
    
    if (!runtime) return state;
    
    if (!runtime->is_running) {
        runtime_start(runtime);
    }
    
    /* Fast forward to overflow */
    red_magic_fast_forward_to_overflow(runtime->system);
    
    transition_phase(runtime, PHASE_LOGIC_EXPLOSION);
    complete_simulation(runtime);
    
    return runtime_get_state(runtime);
}

RuntimeState runtime_get_state(const EverythingBecomesFRuntime *runtime) {
    RuntimeState state;
    memset(&state, 0, sizeof(RuntimeState));
    
    if (!runtime) return state;
    
    state.phase = runtime->current_phase;
    state.system_state = runtime->system->state;
    state.counter_value = runtime->system->counter.value;
    state.is_completed = runtime->is_completed;
    
    counter_get_hex(&runtime->system->counter, state.counter_hex, sizeof(state.counter_hex));
    
    return state;
}

Phase runtime_get_current_phase(const EverythingBecomesFRuntime *runtime) {
    return runtime ? runtime->current_phase : PHASE_COLD_BOOT;
}

bool runtime_is_running(const EverythingBecomesFRuntime *runtime) {
    return runtime && runtime->is_running;
}

bool runtime_is_completed(const EverythingBecomesFRuntime *runtime) {
    return runtime && runtime->is_completed;
}

uint16_t runtime_get_counter_value(const EverythingBecomesFRuntime *runtime) {
    return runtime ? runtime->system->counter.value : 0;
}

void runtime_get_counter_hex(const EverythingBecomesFRuntime *runtime, char *buf, size_t len) {
    if (!runtime || !buf || len == 0) return;
    counter_get_hex(&runtime->system->counter, buf, len);
}

RedMagicSystem *runtime_get_system(EverythingBecomesFRuntime *runtime) {
    return runtime ? runtime->system : NULL;
}

bool runtime_register_phase_callback(EverythingBecomesFRuntime *runtime,
                                     phase_change_callback_t cb, void *ctx) {
    if (!runtime || !cb) return false;
    if (runtime->phase_callback_count >= MAX_PHASE_CALLBACKS) return false;
    
    runtime->phase_callbacks[runtime->phase_callback_count] = cb;
    runtime->phase_callback_contexts[runtime->phase_callback_count] = ctx;
    runtime->phase_callback_count++;
    return true;
}

bool runtime_register_completion_callback(EverythingBecomesFRuntime *runtime,
                                          completion_callback_t cb, void *ctx) {
    if (!runtime || !cb) return false;
    if (runtime->completion_callback_count >= MAX_COMPLETION_CALLBACKS) return false;
    
    runtime->completion_callbacks[runtime->completion_callback_count] = cb;
    runtime->completion_callback_contexts[runtime->completion_callback_count] = ctx;
    runtime->completion_callback_count++;
    return true;
}

VerificationResult runtime_verify_everything_becomes_f(const EverythingBecomesFRuntime *runtime) {
    VerificationResult result;
    memset(&result, 0, sizeof(VerificationResult));
    
    if (!runtime) return result;
    
    const UnsignedShortCounter *counter = &runtime->system->counter;
    
    /* Check if overflow occurred */
    bool overflow_occurred = counter->overflow_count > 0;
    
    /* Check system state */
    bool system_down = runtime->system->state == SYS_STATE_SYSTEM_DOWN;
    
    /* Check escape */
    bool escaped = false;
    if (runtime->magata_quarters) {
        escaped = sealed_room_is_escaped(&runtime->magata_quarters->base);
    }
    
    result.everything_becomes_f = overflow_occurred && system_down;
    result.counter_overflow_occurred = overflow_occurred;
    result.overflow_count = counter->overflow_count;
    result.system_state = runtime->system->state;
    result.system_is_down = system_down;
    result.escaped = escaped;
    result.final_counter_value = counter->value;
    
    counter_get_hex(counter, result.final_counter_hex, sizeof(result.final_counter_hex));
    
    return result;
}

void runtime_reset(EverythingBecomesFRuntime *runtime) {
    if (!runtime) return;
    
    red_magic_reset(runtime->system);
    runtime->current_phase = PHASE_COLD_BOOT;
    runtime->phase_history_count = 0;
    runtime->is_running = false;
    runtime->is_completed = false;
}
