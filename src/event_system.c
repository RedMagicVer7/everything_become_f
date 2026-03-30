/**
 * event_system.c - Asynchronous Event System Implementation
 */

#include <string.h>
#include <time.h>
#include "../include/event_system.h"

/* Event type names */
static const char *EVENT_TYPE_NAMES[] = {
    "SYSTEM_INITIALIZED",
    "SYSTEM_STARTED",
    "SYSTEM_SHUTDOWN",
    "SYSTEM_CRASH",
    "COUNTER_INCREMENT",
    "COUNTER_OVERFLOW",
    "COUNTER_RESET",
    "LOCK_ENGAGED",
    "LOCK_RELEASED",
    "LOCKS_RELEASED",
    "FAILSAFE_TRIGGERED",
    "CAMERA_RECORDING",
    "CAMERA_BLIND_SPOT",
    "CLOCK_SYNC_ERROR",
    "ROOM_SEALED",
    "ROOM_BREACHED",
    "ROOM_ESCAPED",
    "PHASE_TRANSITION",
    "PHASE_COMPLETED"
};

const char *event_type_name(EventType type) {
    if (type >= 0 && type < EVT_COUNT) {
        return EVENT_TYPE_NAMES[type];
    }
    return "UNKNOWN";
}

/* ============================================================================
 * Event Implementation
 * ============================================================================ */

void event_init(Event *event, EventType type, const char *source) {
    if (!event) return;
    
    memset(event, 0, sizeof(Event));
    event->event_type = type;
    event->timestamp = (uint32_t)time(NULL);
    event->hour = -1;
    event->data_int = 0;
    
    if (source) {
        strncpy(event->source, source, MAX_EVENT_SOURCE_LEN - 1);
    }
}

void event_set_hour(Event *event, int32_t hour) {
    if (event) {
        event->hour = hour;
    }
}

void event_set_data_int(Event *event, int32_t data) {
    if (event) {
        event->data_int = data;
    }
}

void event_set_data_str(Event *event, const char *data) {
    if (event && data) {
        strncpy(event->data_str, data, MAX_EVENT_DATA_LEN - 1);
    }
}

/* ============================================================================
 * EventBus Implementation
 * ============================================================================ */

void event_bus_init(EventBus *bus, const char *name) {
    if (!bus) return;
    
    memset(bus, 0, sizeof(EventBus));
    
    if (name) {
        strncpy(bus->name, name, MAX_EVENT_SOURCE_LEN - 1);
    }
    
    bus->history_count = 0;
    bus->history_start = 0;
    bus->paused = false;
    bus->all_handler_count = 0;
    
    for (int i = 0; i < EVT_COUNT; i++) {
        bus->handler_counts[i] = 0;
    }
}

bool event_bus_subscribe(EventBus *bus, EventType type,
                         event_handler_t handler, void *context) {
    if (!bus || !handler) return false;
    if (type < 0 || type >= EVT_COUNT) return false;
    if (bus->handler_counts[type] >= MAX_EVENT_HANDLERS_PER_TYPE) return false;
    
    /* Check if already subscribed */
    for (int i = 0; i < bus->handler_counts[type]; i++) {
        if (bus->handlers[type][i] == handler) {
            return false;  /* Already subscribed */
        }
    }
    
    int idx = bus->handler_counts[type];
    bus->handlers[type][idx] = handler;
    bus->handler_contexts[type][idx] = context;
    bus->handler_counts[type]++;
    
    return true;
}

bool event_bus_subscribe_all(EventBus *bus, event_handler_t handler, void *context) {
    if (!bus || !handler) return false;
    if (bus->all_handler_count >= MAX_ALL_EVENT_HANDLERS) return false;
    
    /* Check if already subscribed */
    for (int i = 0; i < bus->all_handler_count; i++) {
        if (bus->all_handlers[i] == handler) {
            return false;
        }
    }
    
    bus->all_handlers[bus->all_handler_count] = handler;
    bus->all_handler_contexts[bus->all_handler_count] = context;
    bus->all_handler_count++;
    
    return true;
}

bool event_bus_unsubscribe(EventBus *bus, EventType type, event_handler_t handler) {
    if (!bus || !handler) return false;
    if (type < 0 || type >= EVT_COUNT) return false;
    
    for (int i = 0; i < bus->handler_counts[type]; i++) {
        if (bus->handlers[type][i] == handler) {
            /* Shift remaining handlers */
            for (int j = i; j < bus->handler_counts[type] - 1; j++) {
                bus->handlers[type][j] = bus->handlers[type][j + 1];
                bus->handler_contexts[type][j] = bus->handler_contexts[type][j + 1];
            }
            bus->handler_counts[type]--;
            return true;
        }
    }
    return false;
}

bool event_bus_unsubscribe_all(EventBus *bus, event_handler_t handler) {
    if (!bus || !handler) return false;
    
    for (int i = 0; i < bus->all_handler_count; i++) {
        if (bus->all_handlers[i] == handler) {
            for (int j = i; j < bus->all_handler_count - 1; j++) {
                bus->all_handlers[j] = bus->all_handlers[j + 1];
                bus->all_handler_contexts[j] = bus->all_handler_contexts[j + 1];
            }
            bus->all_handler_count--;
            return true;
        }
    }
    return false;
}

int event_bus_publish(EventBus *bus, const Event *event) {
    if (!bus || !event) return 0;
    if (bus->paused) return 0;
    
    /* Add to history (circular buffer) */
    int history_idx = (bus->history_start + bus->history_count) % MAX_EVENT_HISTORY;
    if (bus->history_count >= MAX_EVENT_HISTORY) {
        bus->history_start = (bus->history_start + 1) % MAX_EVENT_HISTORY;
    } else {
        bus->history_count++;
    }
    bus->event_history[history_idx] = *event;
    
    int handlers_called = 0;
    EventType type = event->event_type;
    
    /* Call specific handlers */
    for (int i = 0; i < bus->handler_counts[type]; i++) {
        if (bus->handlers[type][i]) {
            bus->handlers[type][i](event, bus->handler_contexts[type][i]);
            handlers_called++;
        }
    }
    
    /* Call all-event handlers */
    for (int i = 0; i < bus->all_handler_count; i++) {
        if (bus->all_handlers[i]) {
            bus->all_handlers[i](event, bus->all_handler_contexts[i]);
            handlers_called++;
        }
    }
    
    return handlers_called;
}

/* Static buffer for emit - reused to avoid stack allocation issues */
static Event g_emit_event;

Event *event_bus_emit(EventBus *bus, EventType type, const char *source, int32_t hour) {
    if (!bus) return NULL;
    
    event_init(&g_emit_event, type, source);
    event_set_hour(&g_emit_event, hour);
    event_bus_publish(bus, &g_emit_event);
    
    return &g_emit_event;
}

int event_bus_get_event_count(const EventBus *bus) {
    return bus ? bus->history_count : 0;
}

bool event_bus_has_event(const EventBus *bus, EventType type) {
    if (!bus) return false;
    
    for (int i = 0; i < bus->history_count; i++) {
        int idx = (bus->history_start + i) % MAX_EVENT_HISTORY;
        if (bus->event_history[idx].event_type == type) {
            return true;
        }
    }
    return false;
}

int event_bus_get_history(const EventBus *bus, Event *events, int max_count) {
    if (!bus || !events || max_count <= 0) return 0;
    
    int count = bus->history_count < max_count ? bus->history_count : max_count;
    
    /* Return most recent first */
    for (int i = 0; i < count; i++) {
        int idx = (bus->history_start + bus->history_count - 1 - i) % MAX_EVENT_HISTORY;
        events[i] = bus->event_history[idx];
    }
    
    return count;
}

void event_bus_pause(EventBus *bus) {
    if (bus) {
        bus->paused = true;
    }
}

void event_bus_resume(EventBus *bus) {
    if (bus) {
        bus->paused = false;
    }
}

bool event_bus_is_paused(const EventBus *bus) {
    return bus && bus->paused;
}

void event_bus_clear_history(EventBus *bus) {
    if (bus) {
        bus->history_count = 0;
        bus->history_start = 0;
    }
}

void event_bus_clear_subscribers(EventBus *bus) {
    if (!bus) return;
    
    for (int i = 0; i < EVT_COUNT; i++) {
        bus->handler_counts[i] = 0;
    }
    bus->all_handler_count = 0;
}

int event_bus_get_subscriber_count(const EventBus *bus, EventType type) {
    if (!bus || type < 0 || type >= EVT_COUNT) return 0;
    return bus->handler_counts[type];
}

int event_bus_get_total_subscriber_count(const EventBus *bus) {
    if (!bus) return 0;
    
    int total = bus->all_handler_count;
    for (int i = 0; i < EVT_COUNT; i++) {
        total += bus->handler_counts[i];
    }
    return total;
}
