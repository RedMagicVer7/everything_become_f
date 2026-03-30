/**
 * event_system.h - Asynchronous Event System
 * ============================================
 * 
 * Implements a publish/subscribe event bus for loose coupling
 * between system components.
 * 
 * Events propagate through the system:
 * COUNTER_OVERFLOW → LOCKS_RELEASED → CAMERA_BLIND_SPOT → ROOM_BREACHED → SYSTEM_CRASH
 */

#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define MAX_EVENT_HANDLERS_PER_TYPE 16
#define MAX_ALL_EVENT_HANDLERS 8
#define MAX_EVENT_HISTORY 100
#define MAX_EVENT_SOURCE_LEN 64
#define MAX_EVENT_DATA_LEN 256

/**
 * EventType - Types of events in the Red Magic system
 */
typedef enum {
    /* Core system events */
    EVT_SYSTEM_INITIALIZED,
    EVT_SYSTEM_STARTED,
    EVT_SYSTEM_SHUTDOWN,
    EVT_SYSTEM_CRASH,
    
    /* Counter events */
    EVT_COUNTER_INCREMENT,
    EVT_COUNTER_OVERFLOW,      /* Critical: triggers escape sequence */
    EVT_COUNTER_RESET,
    
    /* Lock events */
    EVT_LOCK_ENGAGED,
    EVT_LOCK_RELEASED,
    EVT_LOCKS_RELEASED,        /* All locks released (failsafe) */
    EVT_FAILSAFE_TRIGGERED,
    
    /* Camera events */
    EVT_CAMERA_RECORDING,
    EVT_CAMERA_BLIND_SPOT,     /* 1-minute surveillance gap */
    EVT_CLOCK_SYNC_ERROR,
    
    /* Room events */
    EVT_ROOM_SEALED,
    EVT_ROOM_BREACHED,         /* Security breach detected */
    EVT_ROOM_ESCAPED,          /* Occupant escaped */
    
    /* Phase events */
    EVT_PHASE_TRANSITION,
    EVT_PHASE_COMPLETED,
    
    EVT_COUNT                  /* Number of event types */
} EventType;

/* Get event type name as string */
const char *event_type_name(EventType type);

/**
 * Event - Represents an event in the system
 */
typedef struct {
    EventType event_type;
    uint32_t timestamp;
    char source[MAX_EVENT_SOURCE_LEN];
    int32_t hour;              /* System hour when event occurred */
    int32_t data_int;          /* Generic integer data */
    char data_str[MAX_EVENT_DATA_LEN];  /* Generic string data */
} Event;

/* Create an event */
void event_init(Event *event, EventType type, const char *source);
void event_set_hour(Event *event, int32_t hour);
void event_set_data_int(Event *event, int32_t data);
void event_set_data_str(Event *event, const char *data);

/* Forward declaration */
typedef struct EventBus EventBus;

/* Event handler callback */
typedef void (*event_handler_t)(const Event *event, void *context);

/**
 * EventBus - Publish/Subscribe event bus
 */
struct EventBus {
    char name[MAX_EVENT_SOURCE_LEN];
    event_handler_t handlers[EVT_COUNT][MAX_EVENT_HANDLERS_PER_TYPE];
    void *handler_contexts[EVT_COUNT][MAX_EVENT_HANDLERS_PER_TYPE];
    int handler_counts[EVT_COUNT];
    
    event_handler_t all_handlers[MAX_ALL_EVENT_HANDLERS];
    void *all_handler_contexts[MAX_ALL_EVENT_HANDLERS];
    int all_handler_count;
    
    Event event_history[MAX_EVENT_HISTORY];
    int history_count;
    int history_start;  /* Circular buffer start index */
    
    bool paused;
};

/* Initialize event bus */
void event_bus_init(EventBus *bus, const char *name);

/* Subscribe to specific event type */
bool event_bus_subscribe(EventBus *bus, EventType type, 
                         event_handler_t handler, void *context);

/* Subscribe to all events */
bool event_bus_subscribe_all(EventBus *bus, event_handler_t handler, void *context);

/* Unsubscribe from event type */
bool event_bus_unsubscribe(EventBus *bus, EventType type, event_handler_t handler);

/* Unsubscribe from all events */
bool event_bus_unsubscribe_all(EventBus *bus, event_handler_t handler);

/* Publish an event, returns number of handlers called */
int event_bus_publish(EventBus *bus, const Event *event);

/* Emit event (convenience - creates and publishes) */
Event *event_bus_emit(EventBus *bus, EventType type, const char *source, int32_t hour);

/* Get event count in history */
int event_bus_get_event_count(const EventBus *bus);

/* Check if event type has occurred */
bool event_bus_has_event(const EventBus *bus, EventType type);

/* Get recent events (returns count, fills events array) */
int event_bus_get_history(const EventBus *bus, Event *events, int max_count);

/* Pause/resume event delivery */
void event_bus_pause(EventBus *bus);
void event_bus_resume(EventBus *bus);
bool event_bus_is_paused(const EventBus *bus);

/* Clear history */
void event_bus_clear_history(EventBus *bus);

/* Clear all subscribers */
void event_bus_clear_subscribers(EventBus *bus);

/* Get subscriber count */
int event_bus_get_subscriber_count(const EventBus *bus, EventType type);
int event_bus_get_total_subscriber_count(const EventBus *bus);

#endif /* EVENT_SYSTEM_H */
