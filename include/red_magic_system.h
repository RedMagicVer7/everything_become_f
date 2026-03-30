/**
 * red_magic_system.h - Red Magic Security System Core
 * =====================================================
 * 
 * The Red Magic system is the central security system of the
 * Magata Research Facility. It integrates:
 * - Timer counter (16-bit unsigned short)
 * - Electromagnetic locks
 * - Security cameras
 * - Sealed room monitoring
 * 
 * System states:
 * INITIALIZING → RUNNING → OVERFLOW_DETECTED → FAILSAFE_TRIGGERED → SYSTEM_DOWN
 */

#ifndef RED_MAGIC_SYSTEM_H
#define RED_MAGIC_SYSTEM_H

#include <stdbool.h>
#include <stdint.h>
#include "counter.h"
#include "electromagnetic_lock.h"
#include "security_camera.h"
#include "sealed_room.h"
#include "event_system.h"

#define MAX_CAMERAS_IN_SYSTEM 8
#define MAX_SEALED_ROOMS 4
#define MAX_SYSTEM_LOGS 100
#define MAX_LOG_MESSAGE_LEN 256
#define MAX_STATE_CHANGE_CALLBACKS 4

/* System states */
typedef enum {
    SYS_STATE_INITIALIZING,      /* System starting up */
    SYS_STATE_RUNNING,           /* Normal operation */
    SYS_STATE_OVERFLOW_DETECTED, /* Counter overflow detected */
    SYS_STATE_FAILSAFE_TRIGGERED,/* Emergency failsafe active */
    SYS_STATE_SYSTEM_DOWN        /* System shut down/crashed */
} SystemState;

/* Get system state name as string */
const char *system_state_name(SystemState state);

/**
 * SystemLog - A log entry in the system
 */
typedef struct {
    int32_t hour;
    char message[MAX_LOG_MESSAGE_LEN];
    char level[16];  /* INFO, WARNING, ERROR, CRITICAL */
} SystemLog;

/* Forward declaration */
typedef struct RedMagicSystem RedMagicSystem;

/* State change callback */
typedef void (*system_state_callback_t)(RedMagicSystem *system,
                                        SystemState old_state,
                                        SystemState new_state,
                                        void *context);

/**
 * RedMagicSystem - Core security system controller
 */
struct RedMagicSystem {
    char name[MAX_ROOM_NAME_LEN];
    SystemState state;
    
    /* Subsystems */
    UnsignedShortCounter counter;
    LockSystem lock_system;
    SecurityCamera cameras[MAX_CAMERAS_IN_SYSTEM];
    int camera_count;
    ClockSyncVulnerability clock_vulnerability;
    SealedRoom *sealed_rooms[MAX_SEALED_ROOMS];
    int sealed_room_count;
    EventBus event_bus;
    
    /* Logging */
    SystemLog logs[MAX_SYSTEM_LOGS];
    int log_count;
    
    /* Callbacks */
    system_state_callback_t state_callbacks[MAX_STATE_CHANGE_CALLBACKS];
    void *state_callback_contexts[MAX_STATE_CHANGE_CALLBACKS];
    int state_callback_count;
};

/* Initialize the Red Magic system */
void red_magic_init(RedMagicSystem *system, const char *name);

/* Initialize system (emit SYSTEM_INITIALIZED event) */
void red_magic_initialize(RedMagicSystem *system);

/* Start the system (transition to RUNNING) */
void red_magic_start(RedMagicSystem *system);

/* Advance system by one hour, returns true if overflow occurred */
bool red_magic_advance_hour(RedMagicSystem *system);

/* Fast forward by hours, returns overflow count */
uint32_t red_magic_fast_forward(RedMagicSystem *system, uint32_t hours);

/* Fast forward directly to overflow, returns hours forwarded */
uint32_t red_magic_fast_forward_to_overflow(RedMagicSystem *system);

/* Trigger failsafe */
void red_magic_trigger_failsafe(RedMagicSystem *system);

/* State queries */
SystemState red_magic_get_state(const RedMagicSystem *system);
bool red_magic_is_running(const RedMagicSystem *system);
bool red_magic_is_down(const RedMagicSystem *system);

/* Counter access */
UnsignedShortCounter *red_magic_get_counter(RedMagicSystem *system);
uint16_t red_magic_get_current_hour(const RedMagicSystem *system);
uint32_t red_magic_hours_until_overflow(const RedMagicSystem *system);
double red_magic_years_until_overflow(const RedMagicSystem *system);

/* Lock system access */
LockSystem *red_magic_get_lock_system(RedMagicSystem *system);

/* Event bus access */
EventBus *red_magic_get_event_bus(RedMagicSystem *system);

/* Camera management */
SecurityCamera *red_magic_add_camera(RedMagicSystem *system, 
                                     const char *camera_id, 
                                     const char *location);
SecurityCamera *red_magic_get_camera(RedMagicSystem *system, int index);
int red_magic_get_camera_count(const RedMagicSystem *system);

/* Lock management */
ElectromagneticLock *red_magic_add_lock(RedMagicSystem *system,
                                        const char *lock_id,
                                        const char *location);

/* Sealed room management */
bool red_magic_add_sealed_room(RedMagicSystem *system, SealedRoom *room);
int red_magic_get_sealed_room_count(const RedMagicSystem *system);
SealedRoom *red_magic_get_sealed_room(RedMagicSystem *system, int index);

/* Setup clock vulnerability */
void red_magic_setup_clock_vulnerability(RedMagicSystem *system);

/* Reset system */
void red_magic_reset(RedMagicSystem *system);

/* Register state change callback */
bool red_magic_register_state_callback(RedMagicSystem *system,
                                       system_state_callback_t cb, void *ctx);

/* Logging */
void red_magic_log(RedMagicSystem *system, int32_t hour, 
                   const char *message, const char *level);
int red_magic_get_log_count(const RedMagicSystem *system);

/**
 * create_standard_facility - Create standard Magata Research Facility setup
 */
void create_standard_facility(RedMagicSystem *system, MagataQuarters *quarters);

#endif /* RED_MAGIC_SYSTEM_H */
