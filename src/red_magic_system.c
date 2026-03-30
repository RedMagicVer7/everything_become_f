/**
 * red_magic_system.c - Red Magic Security System Implementation
 */

#include <stdio.h>
#include <string.h>
#include "../include/red_magic_system.h"

/* System state names */
static const char *SYSTEM_STATE_NAMES[] = {
    "INITIALIZING",
    "RUNNING",
    "OVERFLOW_DETECTED",
    "FAILSAFE_TRIGGERED",
    "SYSTEM_DOWN"
};

const char *system_state_name(SystemState state) {
    if (state >= 0 && state <= SYS_STATE_SYSTEM_DOWN) {
        return SYSTEM_STATE_NAMES[state];
    }
    return "UNKNOWN";
}

/* Forward declaration for internal callback */
static void on_counter_overflow(UnsignedShortCounter *counter, void *context);

/* ============================================================================
 * Internal Helpers
 * ============================================================================ */

static void transition_to(RedMagicSystem *system, SystemState new_state) {
    SystemState old_state = system->state;
    system->state = new_state;
    
    for (int i = 0; i < system->state_callback_count; i++) {
        if (system->state_callbacks[i]) {
            system->state_callbacks[i](system, old_state, new_state,
                                       system->state_callback_contexts[i]);
        }
    }
}

/* ============================================================================
 * RedMagicSystem Implementation
 * ============================================================================ */

void red_magic_init(RedMagicSystem *system, const char *name) {
    if (!system) return;
    
    memset(system, 0, sizeof(RedMagicSystem));
    
    if (name) {
        strncpy(system->name, name, MAX_ROOM_NAME_LEN - 1);
    }
    
    system->state = SYS_STATE_INITIALIZING;
    
    /* Initialize subsystems */
    counter_init(&system->counter);
    lock_system_init(&system->lock_system, "Red Magic Locks");
    event_bus_init(&system->event_bus, "Red Magic Events");
    clock_vuln_init(&system->clock_vulnerability);
    
    system->camera_count = 0;
    system->sealed_room_count = 0;
    system->log_count = 0;
    system->state_callback_count = 0;
    
    /* Register overflow callback */
    counter_register_overflow_callback(&system->counter, on_counter_overflow, system);
}

static void on_counter_overflow(UnsignedShortCounter *counter, void *context) {
    RedMagicSystem *system = (RedMagicSystem *)context;
    if (!system) return;
    
    red_magic_log(system, counter->overflow_count * (COUNTER_MAX_VALUE + 1),
                  "COUNTER OVERFLOW DETECTED! Value wrapped to 0x0000", "CRITICAL");
    
    /* Emit overflow event */
    event_bus_emit(&system->event_bus, EVT_COUNTER_OVERFLOW,
                   "UnsignedShortCounter", system->counter.total_increments);
    
    /* Transition to overflow detected */
    transition_to(system, SYS_STATE_OVERFLOW_DETECTED);
    
    /* Trigger failsafe */
    red_magic_trigger_failsafe(system);
}

void red_magic_initialize(RedMagicSystem *system) {
    if (!system) return;
    
    system->state = SYS_STATE_INITIALIZING;
    red_magic_log(system, 0, "Red Magic Security System initializing...", "INFO");
    
    event_bus_emit(&system->event_bus, EVT_SYSTEM_INITIALIZED,
                   "RedMagicSystem", 0);
    
    red_magic_log(system, 0, "System initialization complete", "INFO");
}

void red_magic_start(RedMagicSystem *system) {
    if (!system) return;
    if (system->state != SYS_STATE_INITIALIZING) return;
    
    transition_to(system, SYS_STATE_RUNNING);
    red_magic_log(system, system->counter.value, "System started - RUNNING", "INFO");
    
    event_bus_emit(&system->event_bus, EVT_SYSTEM_STARTED,
                   "RedMagicSystem", system->counter.value);
}

bool red_magic_advance_hour(RedMagicSystem *system) {
    if (!system) return false;
    if (system->state != SYS_STATE_RUNNING) return false;
    
    bool overflow = counter_increment(&system->counter);
    
    event_bus_emit(&system->event_bus, EVT_COUNTER_INCREMENT,
                   "UnsignedShortCounter", system->counter.value);
    
    return overflow;
}

uint32_t red_magic_fast_forward(RedMagicSystem *system, uint32_t hours) {
    if (!system) return 0;
    if (system->state != SYS_STATE_RUNNING) return 0;
    
    char msg[128];
    snprintf(msg, sizeof(msg), "Fast forwarding %u hours...", hours);
    red_magic_log(system, system->counter.value, msg, "INFO");
    
    return counter_fast_forward(&system->counter, hours);
}

uint32_t red_magic_fast_forward_to_overflow(RedMagicSystem *system) {
    if (!system) return 0;
    
    uint32_t hours_needed = counter_hours_until_overflow(&system->counter);
    counter_fast_forward(&system->counter, hours_needed);
    return hours_needed;
}

void red_magic_trigger_failsafe(RedMagicSystem *system) {
    if (!system) return;
    
    red_magic_log(system, system->counter.value,
                  "FAILSAFE TRIGGERED - All locks releasing", "CRITICAL");
    
    transition_to(system, SYS_STATE_FAILSAFE_TRIGGERED);
    
    event_bus_emit(&system->event_bus, EVT_FAILSAFE_TRIGGERED,
                   "RedMagicSystem", system->counter.value);
    
    /* Release all locks */
    int unlocked = lock_system_trigger_global_failsafe(&system->lock_system);
    
    char msg[128];
    snprintf(msg, sizeof(msg), "Released %d locks", unlocked);
    red_magic_log(system, system->counter.value, msg, "INFO");
    
    event_bus_emit(&system->event_bus, EVT_LOCKS_RELEASED,
                   "LockSystem", system->counter.value);
    
    /* Breach all sealed rooms */
    for (int i = 0; i < system->sealed_room_count; i++) {
        SealedRoom *room = system->sealed_rooms[i];
        if (room && sealed_room_is_sealed(room)) {
            sealed_room_breach(room, system->counter.value);
            event_bus_emit(&system->event_bus, EVT_ROOM_BREACHED,
                           "SealedRoom", system->counter.value);
        }
    }
    
    /* System goes down */
    transition_to(system, SYS_STATE_SYSTEM_DOWN);
    red_magic_log(system, system->counter.value, "SYSTEM DOWN", "CRITICAL");
    
    event_bus_emit(&system->event_bus, EVT_SYSTEM_CRASH,
                   "RedMagicSystem", system->counter.value);
}

SystemState red_magic_get_state(const RedMagicSystem *system) {
    return system ? system->state : SYS_STATE_INITIALIZING;
}

bool red_magic_is_running(const RedMagicSystem *system) {
    return system && system->state == SYS_STATE_RUNNING;
}

bool red_magic_is_down(const RedMagicSystem *system) {
    return system && system->state == SYS_STATE_SYSTEM_DOWN;
}

UnsignedShortCounter *red_magic_get_counter(RedMagicSystem *system) {
    return system ? &system->counter : NULL;
}

uint16_t red_magic_get_current_hour(const RedMagicSystem *system) {
    return system ? system->counter.value : 0;
}

uint32_t red_magic_hours_until_overflow(const RedMagicSystem *system) {
    return system ? counter_hours_until_overflow(&system->counter) : 0;
}

double red_magic_years_until_overflow(const RedMagicSystem *system) {
    return system ? counter_years_until_overflow(&system->counter) : 0.0;
}

LockSystem *red_magic_get_lock_system(RedMagicSystem *system) {
    return system ? &system->lock_system : NULL;
}

EventBus *red_magic_get_event_bus(RedMagicSystem *system) {
    return system ? &system->event_bus : NULL;
}

SecurityCamera *red_magic_add_camera(RedMagicSystem *system,
                                     const char *camera_id,
                                     const char *location) {
    if (!system) return NULL;
    if (system->camera_count >= MAX_CAMERAS_IN_SYSTEM) return NULL;
    
    SecurityCamera *camera = &system->cameras[system->camera_count];
    camera_init(camera, camera_id, location);
    system->camera_count++;
    
    /* Add to vulnerability system */
    clock_vuln_add_camera(&system->clock_vulnerability, camera);
    
    return camera;
}

SecurityCamera *red_magic_get_camera(RedMagicSystem *system, int index) {
    if (!system || index < 0 || index >= system->camera_count) return NULL;
    return &system->cameras[index];
}

int red_magic_get_camera_count(const RedMagicSystem *system) {
    return system ? system->camera_count : 0;
}

ElectromagneticLock *red_magic_add_lock(RedMagicSystem *system,
                                        const char *lock_id,
                                        const char *location) {
    if (!system) return NULL;
    return lock_system_create_lock(&system->lock_system, lock_id, location);
}

bool red_magic_add_sealed_room(RedMagicSystem *system, SealedRoom *room) {
    if (!system || !room) return false;
    if (system->sealed_room_count >= MAX_SEALED_ROOMS) return false;
    
    system->sealed_rooms[system->sealed_room_count] = room;
    system->sealed_room_count++;
    return true;
}

int red_magic_get_sealed_room_count(const RedMagicSystem *system) {
    return system ? system->sealed_room_count : 0;
}

SealedRoom *red_magic_get_sealed_room(RedMagicSystem *system, int index) {
    if (!system || index < 0 || index >= system->sealed_room_count) return NULL;
    return system->sealed_rooms[index];
}

void red_magic_setup_clock_vulnerability(RedMagicSystem *system) {
    if (!system) return;
    
    /* Cameras are already added to vulnerability in red_magic_add_camera */
    /* This function is kept for API compatibility */
}

void red_magic_reset(RedMagicSystem *system) {
    if (!system) return;
    
    system->state = SYS_STATE_INITIALIZING;
    
    /* Reinitialize counter */
    counter_init(&system->counter);
    counter_register_overflow_callback(&system->counter, on_counter_overflow, system);
    
    lock_system_reset(&system->lock_system);
    system->log_count = 0;
    event_bus_clear_history(&system->event_bus);
}

bool red_magic_register_state_callback(RedMagicSystem *system,
                                       system_state_callback_t cb, void *ctx) {
    if (!system || !cb) return false;
    if (system->state_callback_count >= MAX_STATE_CHANGE_CALLBACKS) return false;
    
    system->state_callbacks[system->state_callback_count] = cb;
    system->state_callback_contexts[system->state_callback_count] = ctx;
    system->state_callback_count++;
    return true;
}

void red_magic_log(RedMagicSystem *system, int32_t hour,
                   const char *message, const char *level) {
    if (!system || !message) return;
    if (system->log_count >= MAX_SYSTEM_LOGS) return;
    
    SystemLog *log = &system->logs[system->log_count];
    log->hour = hour;
    strncpy(log->message, message, MAX_LOG_MESSAGE_LEN - 1);
    strncpy(log->level, level ? level : "INFO", 15);
    
    system->log_count++;
}

int red_magic_get_log_count(const RedMagicSystem *system) {
    return system ? system->log_count : 0;
}

void create_standard_facility(RedMagicSystem *system, MagataQuarters *quarters) {
    if (!system || !quarters) return;
    
    red_magic_init(system, "Magata Research Facility - Red Magic");
    
    /* Initialize Magata's quarters */
    magata_quarters_init(quarters);
    
    /* Add room to system */
    red_magic_add_sealed_room(system, &quarters->base);
    
    /* Add Magata's lock to lock system */
    lock_system_add_lock(&system->lock_system, &quarters->main_lock);
    
    /* Create additional locks */
    red_magic_add_lock(system, "entrance_lock", "Facility Main Entrance");
    red_magic_add_lock(system, "corridor_lock", "Secure Corridor");
    
    /* Create cameras */
    red_magic_add_camera(system, "cam_001", "Magata's Room");
    red_magic_add_camera(system, "cam_002", "Corridor A");
    red_magic_add_camera(system, "cam_003", "Main Entrance");
    
    /* Setup clock vulnerability */
    red_magic_setup_clock_vulnerability(system);
    
    /* Initialize system */
    red_magic_initialize(system);
}
