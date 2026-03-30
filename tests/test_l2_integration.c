/**
 * test_l2_integration.c - L2 Integration Tests
 * ==============================================
 * 
 * Tests interactions between components.
 * Verifies that components work together correctly.
 */

#include <stdio.h>
#include <string.h>
#include "test_framework.h"
#include "../include/counter.h"
#include "../include/electromagnetic_lock.h"
#include "../include/security_camera.h"
#include "../include/sealed_room.h"
#include "../include/event_system.h"
#include "../include/red_magic_system.h"

/* ============================================================================
 * Counter + Lock Integration Tests
 * ============================================================================ */

static LockSystem *g_test_lock_system = NULL;

static void overflow_callback_trigger_failsafe(UnsignedShortCounter *counter, void *context) {
    (void)counter;
    LockSystem *system = (LockSystem *)context;
    if (system) {
        lock_system_trigger_global_failsafe(system);
    }
}

TEST(test_overflow_triggers_lock_release) {
    UnsignedShortCounter counter;
    LockSystem lock_system;
    
    counter_init(&counter);
    lock_system_init(&lock_system, "Test System");
    lock_system_create_lock(&lock_system, "lock_1", "Door 1");
    lock_system_create_lock(&lock_system, "lock_2", "Door 2");
    
    counter_register_overflow_callback(&counter, overflow_callback_trigger_failsafe, &lock_system);
    
    /* Verify locks are initially locked */
    ASSERT_TRUE(lock_system_all_locked(&lock_system));
    
    /* Cause overflow */
    counter_set_value(&counter, 0xFFFF);
    counter_increment(&counter);
    
    /* Verify locks are now unlocked */
    ASSERT_TRUE(lock_system_all_unlocked(&lock_system));
    ASSERT_TRUE(lock_system_is_failsafe_triggered(&lock_system));
}

TEST(test_fast_forward_to_overflow_unlocks) {
    UnsignedShortCounter counter;
    LockSystem lock_system;
    
    counter_init(&counter);
    lock_system_init(&lock_system, "Test System");
    lock_system_create_lock(&lock_system, "lock_1", "Door 1");
    
    counter_register_overflow_callback(&counter, overflow_callback_trigger_failsafe, &lock_system);
    
    /* Fast forward past overflow */
    counter_fast_forward(&counter, 65536);
    
    ASSERT_TRUE(lock_system_all_unlocked(&lock_system));
}

/* ============================================================================
 * Event Propagation Tests
 * ============================================================================ */

static EventType g_received_events[20];
static int g_event_count = 0;

static void track_event_handler(const Event *event, void *context) {
    (void)context;
    if (g_event_count < 20) {
        g_received_events[g_event_count++] = event->event_type;
    }
}

TEST(test_overflow_event_chain) {
    EventBus bus;
    event_bus_init(&bus, "Test");
    g_event_count = 0;
    
    event_bus_subscribe_all(&bus, track_event_handler, NULL);
    
    /* Simulate event chain */
    event_bus_emit(&bus, EVT_COUNTER_OVERFLOW, "counter", 0);
    event_bus_emit(&bus, EVT_FAILSAFE_TRIGGERED, "system", 0);
    event_bus_emit(&bus, EVT_LOCKS_RELEASED, "locks", 0);
    event_bus_emit(&bus, EVT_ROOM_BREACHED, "room", 0);
    
    ASSERT_EQ(g_event_count, 4);
    ASSERT_EQ(g_received_events[0], EVT_COUNTER_OVERFLOW);
    ASSERT_EQ(g_received_events[1], EVT_FAILSAFE_TRIGGERED);
    ASSERT_EQ(g_received_events[2], EVT_LOCKS_RELEASED);
    ASSERT_EQ(g_received_events[3], EVT_ROOM_BREACHED);
}

static LockSystem *g_chain_lock_system = NULL;
static SealedRoom *g_chain_room = NULL;
static EventBus *g_chain_bus = NULL;

static void on_overflow_trigger_failsafe(const Event *event, void *context) {
    (void)event;
    (void)context;
    if (g_chain_lock_system) {
        lock_system_trigger_global_failsafe(g_chain_lock_system);
        event_bus_emit(g_chain_bus, EVT_LOCKS_RELEASED, "lock_system", 0);
    }
}

static void on_locks_released_breach_room(const Event *event, void *context) {
    (void)event;
    (void)context;
    if (g_chain_room) {
        sealed_room_breach(g_chain_room, 0);
        event_bus_emit(g_chain_bus, EVT_ROOM_BREACHED, "room", 0);
    }
}

TEST(test_event_bus_component_communication) {
    EventBus bus;
    LockSystem lock_system;
    SealedRoom room;
    
    event_bus_init(&bus, "Test");
    lock_system_init(&lock_system, "Test System");
    lock_system_create_lock(&lock_system, "lock_1", "Door 1");
    sealed_room_init(&room, "room_1", "Test Room");
    
    g_chain_lock_system = &lock_system;
    g_chain_room = &room;
    g_chain_bus = &bus;
    
    event_bus_subscribe(&bus, EVT_COUNTER_OVERFLOW, on_overflow_trigger_failsafe, NULL);
    event_bus_subscribe(&bus, EVT_LOCKS_RELEASED, on_locks_released_breach_room, NULL);
    
    /* Trigger the chain */
    event_bus_emit(&bus, EVT_COUNTER_OVERFLOW, "counter", 0);
    
    /* Verify end state */
    ASSERT_TRUE(lock_system_all_unlocked(&lock_system));
    ASSERT_TRUE(sealed_room_is_breached(&room));
    
    g_chain_lock_system = NULL;
    g_chain_room = NULL;
    g_chain_bus = NULL;
}

/* ============================================================================
 * Clock Sync Vulnerability + Camera Tests
 * ============================================================================ */

TEST(test_vulnerability_creates_blind_spot) {
    SecurityCamera cam1, cam2;
    camera_init(&cam1, "cam_1", "Room 1");
    camera_init(&cam2, "cam_2", "Room 2");
    
    /* Record some footage first */
    for (int minute = 0; minute < 60; minute++) {
        camera_record_minute(&cam1, 100, minute);
        camera_record_minute(&cam2, 100, minute);
    }
    
    ClockSyncVulnerability vuln;
    clock_vuln_init(&vuln);
    clock_vuln_add_camera(&vuln, &cam1);
    clock_vuln_add_camera(&vuln, &cam2);
    
    int affected = clock_vuln_exploit(&vuln, 100, 30);
    
    ASSERT_EQ(affected, 2);
    ASSERT_TRUE(camera_has_blind_spots(&cam1));
    ASSERT_TRUE(camera_has_blind_spots(&cam2));
}

TEST(test_reinstall_simulation) {
    SecurityCamera cam;
    camera_init(&cam, "cam_1", "Room 1");
    
    ClockSyncVulnerability vuln;
    clock_vuln_init(&vuln);
    clock_vuln_add_camera(&vuln, &cam);
    
    ReinstallResult result = clock_vuln_simulate_reinstall(&vuln, 50, 15);
    
    ASSERT_TRUE(result.reinstall_triggered);
    ASSERT_TRUE(result.blind_spot_created);
    ASSERT_EQ(result.clock_drift_minutes, 1);
    ASSERT_EQ(result.blind_spot_hour, 50);
    ASSERT_EQ(result.blind_spot_minute, 15);
}

/* ============================================================================
 * Red Magic System Integration Tests
 * ============================================================================ */

TEST(test_system_state_transitions) {
    RedMagicSystem system;
    MagataQuarters quarters;
    
    create_standard_facility(&system, &quarters);
    red_magic_start(&system);
    
    ASSERT_EQ(red_magic_get_state(&system), SYS_STATE_RUNNING);
    
    red_magic_fast_forward_to_overflow(&system);
    
    ASSERT_EQ(red_magic_get_state(&system), SYS_STATE_SYSTEM_DOWN);
}

TEST(test_system_initializing_to_running) {
    RedMagicSystem system;
    red_magic_init(&system, "Test System");
    red_magic_initialize(&system);
    
    ASSERT_EQ(red_magic_get_state(&system), SYS_STATE_INITIALIZING);
    
    red_magic_start(&system);
    
    ASSERT_EQ(red_magic_get_state(&system), SYS_STATE_RUNNING);
}

TEST(test_overflow_triggers_full_sequence) {
    RedMagicSystem system;
    MagataQuarters quarters;
    
    create_standard_facility(&system, &quarters);
    red_magic_start(&system);
    
    red_magic_fast_forward_to_overflow(&system);
    
    ASSERT_EQ(red_magic_get_state(&system), SYS_STATE_SYSTEM_DOWN);
    ASSERT_TRUE(lock_system_is_failsafe_triggered(&system.lock_system));
    ASSERT_TRUE(sealed_room_is_breached(&quarters.base));
}

TEST(test_system_events_during_overflow) {
    RedMagicSystem system;
    MagataQuarters quarters;
    
    create_standard_facility(&system, &quarters);
    g_event_count = 0;
    
    event_bus_subscribe_all(&system.event_bus, track_event_handler, NULL);
    
    red_magic_start(&system);
    red_magic_fast_forward_to_overflow(&system);
    
    /* Check key events occurred */
    bool has_overflow = false, has_failsafe = false, has_locks = false, has_crash = false;
    for (int i = 0; i < g_event_count; i++) {
        if (g_received_events[i] == EVT_COUNTER_OVERFLOW) has_overflow = true;
        if (g_received_events[i] == EVT_FAILSAFE_TRIGGERED) has_failsafe = true;
        if (g_received_events[i] == EVT_LOCKS_RELEASED) has_locks = true;
        if (g_received_events[i] == EVT_SYSTEM_CRASH) has_crash = true;
    }
    
    ASSERT_TRUE(has_overflow);
    ASSERT_TRUE(has_failsafe);
    ASSERT_TRUE(has_locks);
    ASSERT_TRUE(has_crash);
}

TEST(test_standard_facility_setup) {
    RedMagicSystem system;
    MagataQuarters quarters;
    
    create_standard_facility(&system, &quarters);
    
    ASSERT_GTE(lock_system_get_lock_count(&system.lock_system), 1);
    ASSERT_GTE(red_magic_get_camera_count(&system), 1);
    ASSERT_GTE(red_magic_get_sealed_room_count(&system), 1);
    ASSERT_EQ(red_magic_get_state(&system), SYS_STATE_INITIALIZING);
}

/* ============================================================================
 * Room + Lock Integration Tests
 * ============================================================================ */

TEST(test_room_breach_when_locks_released) {
    SealedRoom room;
    ElectromagneticLock lock;
    
    sealed_room_init(&room, "test_room", "Test Room");
    lock_init(&lock, "lock_1", "Main Lock");
    sealed_room_add_door(&room, "door_1", "Main Door", true, &lock);
    
    /* Release the lock */
    lock_trigger_failsafe(&lock);
    
    /* Room should still be sealed (lock release doesn't auto-breach) */
    ASSERT_TRUE(sealed_room_is_sealed(&room));
    
    /* But now breach is possible */
    sealed_room_breach(&room, 0);
    ASSERT_TRUE(sealed_room_is_breached(&room));
}

TEST(test_magata_quarters_full_escape_sequence) {
    MagataQuarters quarters;
    magata_quarters_init(&quarters);
    
    /* Initial state */
    ASSERT_TRUE(sealed_room_is_sealed(&quarters.base));
    ASSERT_TRUE(lock_is_locked(&quarters.main_lock));
    
    /* Trigger overflow escape */
    magata_quarters_trigger_overflow_escape(&quarters, 65536);
    
    /* Final state */
    ASSERT_EQ(sealed_room_get_state(&quarters.base), ROOM_STATE_ESCAPED);
    ASSERT_TRUE(lock_is_unlocked(&quarters.main_lock));
    ASSERT_FALSE(sealed_room_occupant_present(&quarters.base));
}

/* ============================================================================
 * Run L2 Tests
 * ============================================================================ */

int run_l2_tests(void) {
    printf("\n=== L2 Integration Tests ===\n\n");
    
    TEST_SUITE_BEGIN();
    
    /* Counter + Lock integration */
    printf("Counter + Lock Integration:\n");
    RUN_TEST(test_overflow_triggers_lock_release);
    RUN_TEST(test_fast_forward_to_overflow_unlocks);
    
    /* Event propagation */
    printf("\nEvent Propagation:\n");
    RUN_TEST(test_overflow_event_chain);
    RUN_TEST(test_event_bus_component_communication);
    
    /* Camera vulnerability */
    printf("\nClock Sync Vulnerability:\n");
    RUN_TEST(test_vulnerability_creates_blind_spot);
    RUN_TEST(test_reinstall_simulation);
    
    /* Red Magic System */
    printf("\nRed Magic System:\n");
    RUN_TEST(test_system_state_transitions);
    RUN_TEST(test_system_initializing_to_running);
    RUN_TEST(test_overflow_triggers_full_sequence);
    RUN_TEST(test_system_events_during_overflow);
    RUN_TEST(test_standard_facility_setup);
    
    /* Room + Lock */
    printf("\nRoom + Lock Integration:\n");
    RUN_TEST(test_room_breach_when_locks_released);
    RUN_TEST(test_magata_quarters_full_escape_sequence);
    
    TEST_SUMMARY();
    
    return TEST_GET_FAILED();
}
