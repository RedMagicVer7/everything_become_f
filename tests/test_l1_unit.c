/**
 * test_l1_unit.c - L1 Unit Tests
 * ===============================
 * 
 * Tests individual components in isolation.
 * Each test focuses on a single unit of functionality.
 */

#include <stdio.h>
#include <string.h>
#include "test_framework.h"
#include "../include/counter.h"
#include "../include/electromagnetic_lock.h"
#include "../include/security_camera.h"
#include "../include/sealed_room.h"
#include "../include/event_system.h"

/* ============================================================================
 * Counter Unit Tests
 * ============================================================================ */

TEST(test_counter_init_default) {
    UnsignedShortCounter counter;
    counter_init(&counter);
    ASSERT_EQ(counter.value, 0);
    ASSERT_EQ(counter.overflow_count, 0);
}

TEST(test_counter_init_with_value) {
    UnsignedShortCounter counter;
    counter_init_with_value(&counter, 100);
    ASSERT_EQ(counter.value, 100);
}

TEST(test_counter_hex_format) {
    UnsignedShortCounter counter;
    char buf[16];
    counter_init_with_value(&counter, 0xABCD);
    counter_get_hex(&counter, buf, sizeof(buf));
    ASSERT_STR_EQ(buf, "0xABCD");
}

TEST(test_counter_increment_basic) {
    UnsignedShortCounter counter;
    counter_init(&counter);
    bool overflow = counter_increment(&counter);
    ASSERT_FALSE(overflow);
    ASSERT_EQ(counter.value, 1);
}

TEST(test_counter_increment_by_amount) {
    UnsignedShortCounter counter;
    counter_init(&counter);
    counter_increment_by(&counter, 100);
    ASSERT_EQ(counter.value, 100);
}

TEST(test_counter_overflow_detection) {
    UnsignedShortCounter counter;
    counter_init_with_value(&counter, 0xFFFF);
    ASSERT_EQ(counter.value, 65535);
    bool overflow = counter_increment(&counter);
    ASSERT_TRUE(overflow);
    ASSERT_EQ(counter.value, 0);
    ASSERT_EQ(counter.overflow_count, 1);
}

TEST(test_counter_overflow_wrapping) {
    UnsignedShortCounter counter;
    counter_init_with_value(&counter, 0xFFFF);
    counter_increment(&counter);
    char buf[16];
    counter_get_hex(&counter, buf, sizeof(buf));
    ASSERT_STR_EQ(buf, "0x0000");
}

TEST(test_counter_max_value_constant) {
    ASSERT_EQ(COUNTER_MAX_VALUE, 0xFFFF);
    ASSERT_EQ(COUNTER_MAX_VALUE, 65535);
}

TEST(test_counter_hours_until_overflow) {
    UnsignedShortCounter counter;
    counter_init(&counter);
    ASSERT_EQ(counter_hours_until_overflow(&counter), 65536);
}

TEST(test_counter_hours_until_overflow_near_max) {
    UnsignedShortCounter counter;
    counter_init_with_value(&counter, 0xFFFE);
    ASSERT_EQ(counter_hours_until_overflow(&counter), 2);
}

TEST(test_counter_years_until_overflow) {
    UnsignedShortCounter counter;
    counter_init(&counter);
    double years = counter_years_until_overflow(&counter);
    ASSERT_GT(years, 7.4);
    ASSERT_LT(years, 7.5);
}

TEST(test_counter_fast_forward_basic) {
    UnsignedShortCounter counter;
    counter_init(&counter);
    uint32_t overflows = counter_fast_forward(&counter, 1000);
    ASSERT_EQ(overflows, 0);
    ASSERT_EQ(counter.value, 1000);
}

TEST(test_counter_fast_forward_with_overflow) {
    UnsignedShortCounter counter;
    counter_init(&counter);
    uint32_t overflows = counter_fast_forward(&counter, 65536);
    ASSERT_EQ(overflows, 1);
    ASSERT_EQ(counter.value, 0);
}

TEST(test_counter_fast_forward_multiple_overflows) {
    UnsignedShortCounter counter;
    counter_init(&counter);
    uint32_t overflows = counter_fast_forward(&counter, 131072);
    ASSERT_EQ(overflows, 2);
    ASSERT_EQ(counter.value, 0);
}

TEST(test_counter_is_at_max) {
    UnsignedShortCounter counter;
    counter_init_with_value(&counter, 0xFFFF);
    ASSERT_TRUE(counter_is_at_max(&counter));
    counter_init(&counter);
    ASSERT_FALSE(counter_is_at_max(&counter));
}

TEST(test_counter_is_at_zero) {
    UnsignedShortCounter counter;
    counter_init(&counter);
    ASSERT_TRUE(counter_is_at_zero(&counter));
    counter_init_with_value(&counter, 100);
    ASSERT_FALSE(counter_is_at_zero(&counter));
}

/* ============================================================================
 * Electromagnetic Lock Unit Tests
 * ============================================================================ */

TEST(test_lock_init) {
    ElectromagneticLock lock;
    lock_init(&lock, "test", "Test Location");
    ASSERT_TRUE(lock_is_locked(&lock));
    ASSERT_TRUE(lock_is_powered(&lock));
    ASSERT_EQ(lock_get_state(&lock), LOCK_STATE_LOCKED);
}

TEST(test_lock_power_off_unlocks) {
    ElectromagneticLock lock;
    lock_init(&lock, "test", "Test Location");
    lock_power_off(&lock);
    ASSERT_FALSE(lock_is_locked(&lock));
    ASSERT_TRUE(lock_is_unlocked(&lock));
    ASSERT_FALSE(lock_is_powered(&lock));
}

TEST(test_lock_power_on_locks) {
    ElectromagneticLock lock;
    lock_init(&lock, "test", "Test Location");
    lock_power_off(&lock);
    lock_power_on(&lock);
    ASSERT_TRUE(lock_is_locked(&lock));
    ASSERT_TRUE(lock_is_powered(&lock));
}

TEST(test_lock_failsafe_triggers_unlock) {
    ElectromagneticLock lock;
    lock_init(&lock, "test", "Test Location");
    lock_trigger_failsafe(&lock);
    ASSERT_TRUE(lock_is_unlocked(&lock));
    ASSERT_FALSE(lock_is_powered(&lock));
}

TEST(test_lock_system_init) {
    LockSystem system;
    lock_system_init(&system, "Test System");
    ASSERT_EQ(lock_system_get_lock_count(&system), 0);
    ASSERT_FALSE(lock_system_is_failsafe_triggered(&system));
}

TEST(test_lock_system_create_lock) {
    LockSystem system;
    lock_system_init(&system, "Test System");
    lock_system_create_lock(&system, "lock_1", "Door 1");
    lock_system_create_lock(&system, "lock_2", "Door 2");
    lock_system_create_lock(&system, "lock_3", "Door 3");
    ASSERT_EQ(lock_system_get_lock_count(&system), 3);
    ASSERT_EQ(lock_system_get_locked_count(&system), 3);
}

TEST(test_lock_system_all_locked) {
    LockSystem system;
    lock_system_init(&system, "Test System");
    lock_system_create_lock(&system, "lock_1", "Door 1");
    lock_system_create_lock(&system, "lock_2", "Door 2");
    ASSERT_TRUE(lock_system_all_locked(&system));
    lock_power_off(&system.locks[0]);
    ASSERT_FALSE(lock_system_all_locked(&system));
}

TEST(test_lock_system_global_failsafe) {
    LockSystem system;
    lock_system_init(&system, "Test System");
    lock_system_create_lock(&system, "lock_1", "Door 1");
    lock_system_create_lock(&system, "lock_2", "Door 2");
    lock_system_create_lock(&system, "lock_3", "Door 3");
    int unlocked = lock_system_trigger_global_failsafe(&system);
    ASSERT_EQ(unlocked, 3);
    ASSERT_TRUE(lock_system_all_unlocked(&system));
    ASSERT_TRUE(lock_system_is_failsafe_triggered(&system));
}

/* ============================================================================
 * Security Camera Unit Tests
 * ============================================================================ */

TEST(test_camera_init) {
    SecurityCamera camera;
    camera_init(&camera, "cam_1", "Room 1");
    ASSERT_TRUE(camera_is_active(&camera));
    ASSERT_EQ(camera.state, CAMERA_STATE_ACTIVE);
}

TEST(test_camera_record_minute) {
    SecurityCamera camera;
    camera_init(&camera, "cam_1", "Room 1");
    VideoSegment *segment = camera_record_minute(&camera, 0, 30);
    ASSERT_NOT_NULL(segment);
    ASSERT_EQ(segment->hour_index, 0);
    ASSERT_EQ(segment->minute_index, 30);
    ASSERT_FALSE(video_segment_is_blind_spot(segment));
}

TEST(test_camera_create_blind_spot) {
    SecurityCamera camera;
    camera_init(&camera, "cam_1", "Room 1");
    VideoSegment *segment = camera_create_blind_spot(&camera, 0, 15);
    ASSERT_NOT_NULL(segment);
    ASSERT_TRUE(video_segment_is_blind_spot(segment));
    ASSERT_TRUE(camera_has_blind_spots(&camera));
}

TEST(test_camera_blind_spots_count) {
    SecurityCamera camera;
    camera_init(&camera, "cam_1", "Room 1");
    camera_create_blind_spot(&camera, 0, 10);
    camera_create_blind_spot(&camera, 0, 20);
    camera_record_minute(&camera, 0, 30);
    ASSERT_EQ(camera_get_blind_spot_count(&camera), 2);
}

TEST(test_clock_vuln_init) {
    ClockSyncVulnerability vuln;
    clock_vuln_init(&vuln);
    ASSERT_EQ(vuln.camera_count, 0);
    ASSERT_FALSE(clock_vuln_is_exploited(&vuln));
}

TEST(test_clock_vuln_exploit) {
    SecurityCamera cam1, cam2;
    camera_init(&cam1, "cam_1", "Room 1");
    camera_init(&cam2, "cam_2", "Room 2");
    
    ClockSyncVulnerability vuln;
    clock_vuln_init(&vuln);
    clock_vuln_add_camera(&vuln, &cam1);
    clock_vuln_add_camera(&vuln, &cam2);
    
    int affected = clock_vuln_exploit(&vuln, 10, 30);
    ASSERT_EQ(affected, 2);
    ASSERT_TRUE(clock_vuln_is_exploited(&vuln));
}

TEST(test_clock_drift_constant) {
    ASSERT_EQ(CLOCK_DRIFT_MINUTES, 1);
}

/* ============================================================================
 * Sealed Room Unit Tests
 * ============================================================================ */

TEST(test_sealed_room_init) {
    SealedRoom room;
    sealed_room_init(&room, "test_room", "Test Room");
    ASSERT_TRUE(sealed_room_is_sealed(&room));
    ASSERT_EQ(sealed_room_get_state(&room), ROOM_STATE_SEALED);
    ASSERT_TRUE(sealed_room_occupant_present(&room));
}

TEST(test_sealed_room_add_door) {
    SealedRoom room;
    ElectromagneticLock lock;
    sealed_room_init(&room, "test_room", "Test Room");
    lock_init(&lock, "lock_1", "Main Lock");
    sealed_room_add_door(&room, "door_1", "Main Door", true, &lock);
    ASSERT_EQ(sealed_room_get_door_count(&room), 1);
}

TEST(test_sealed_room_breach) {
    SealedRoom room;
    sealed_room_init(&room, "test_room", "Test Room");
    sealed_room_breach(&room, 100);
    ASSERT_TRUE(sealed_room_is_breached(&room));
    ASSERT_EQ(sealed_room_get_state(&room), ROOM_STATE_BREACHED);
    ASSERT_EQ(sealed_room_get_breach_time(&room), 100);
}

TEST(test_sealed_room_escape) {
    SealedRoom room;
    sealed_room_init(&room, "test_room", "Test Room");
    /* Can't escape sealed room */
    ASSERT_FALSE(sealed_room_escape(&room, 100));
    /* Breach then escape */
    sealed_room_breach(&room, 100);
    ASSERT_TRUE(sealed_room_escape(&room, 101));
    ASSERT_EQ(sealed_room_get_state(&room), ROOM_STATE_ESCAPED);
    ASSERT_FALSE(sealed_room_occupant_present(&room));
}

TEST(test_magata_quarters_init) {
    MagataQuarters quarters;
    magata_quarters_init(&quarters);
    ASSERT_TRUE(sealed_room_is_sealed(&quarters.base));
    ASSERT_TRUE(magata_quarters_has_network_access(&quarters));
    ASSERT_TRUE(magata_quarters_can_program(&quarters));
    ASSERT_TRUE(lock_is_locked(&quarters.main_lock));
}

TEST(test_magata_confinement_constants) {
    ASSERT_EQ(MAGATA_CONFINEMENT_YEARS, 15);
    ASSERT_GT(MAGATA_CONFINEMENT_HOURS, 130000);
}

TEST(test_magata_overflow_escape) {
    MagataQuarters quarters;
    magata_quarters_init(&quarters);
    bool success = magata_quarters_trigger_overflow_escape(&quarters, 65536);
    ASSERT_TRUE(success);
    ASSERT_EQ(sealed_room_get_state(&quarters.base), ROOM_STATE_ESCAPED);
    ASSERT_TRUE(lock_is_unlocked(&quarters.main_lock));
}

/* ============================================================================
 * Event System Unit Tests
 * ============================================================================ */

TEST(test_event_bus_init) {
    EventBus bus;
    event_bus_init(&bus, "Test Bus");
    ASSERT_EQ(event_bus_get_event_count(&bus), 0);
}

static int g_event_received_count = 0;
static void test_event_handler(const Event *event, void *context) {
    (void)event;
    (void)context;
    g_event_received_count++;
}

TEST(test_event_bus_emit_and_subscribe) {
    EventBus bus;
    event_bus_init(&bus, "Test Bus");
    g_event_received_count = 0;
    
    event_bus_subscribe(&bus, EVT_COUNTER_OVERFLOW, test_event_handler, NULL);
    event_bus_emit(&bus, EVT_COUNTER_OVERFLOW, "test", 0);
    
    ASSERT_EQ(g_event_received_count, 1);
}

TEST(test_event_bus_subscribe_all) {
    EventBus bus;
    event_bus_init(&bus, "Test Bus");
    g_event_received_count = 0;
    
    event_bus_subscribe_all(&bus, test_event_handler, NULL);
    event_bus_emit(&bus, EVT_SYSTEM_STARTED, "test", 0);
    event_bus_emit(&bus, EVT_COUNTER_OVERFLOW, "test", 0);
    
    ASSERT_EQ(g_event_received_count, 2);
}

TEST(test_event_bus_history) {
    EventBus bus;
    event_bus_init(&bus, "Test Bus");
    event_bus_emit(&bus, EVT_SYSTEM_STARTED, "test", 0);
    event_bus_emit(&bus, EVT_COUNTER_OVERFLOW, "test", 0);
    
    ASSERT_EQ(event_bus_get_event_count(&bus), 2);
    ASSERT_TRUE(event_bus_has_event(&bus, EVT_COUNTER_OVERFLOW));
}

TEST(test_event_bus_pause_resume) {
    EventBus bus;
    event_bus_init(&bus, "Test Bus");
    g_event_received_count = 0;
    
    event_bus_subscribe_all(&bus, test_event_handler, NULL);
    event_bus_pause(&bus);
    event_bus_emit(&bus, EVT_SYSTEM_STARTED, "test", 0);
    ASSERT_EQ(g_event_received_count, 0);
    
    event_bus_resume(&bus);
    event_bus_emit(&bus, EVT_SYSTEM_STARTED, "test", 0);
    ASSERT_EQ(g_event_received_count, 1);
}

TEST(test_event_type_names) {
    ASSERT_STR_EQ(event_type_name(EVT_COUNTER_OVERFLOW), "COUNTER_OVERFLOW");
    ASSERT_STR_EQ(event_type_name(EVT_LOCKS_RELEASED), "LOCKS_RELEASED");
    ASSERT_STR_EQ(event_type_name(EVT_ROOM_BREACHED), "ROOM_BREACHED");
    ASSERT_STR_EQ(event_type_name(EVT_SYSTEM_CRASH), "SYSTEM_CRASH");
}

/* ============================================================================
 * Run L1 Tests
 * ============================================================================ */

int run_l1_tests(void) {
    printf("\n=== L1 Unit Tests ===\n\n");
    
    TEST_SUITE_BEGIN();
    
    /* Counter tests */
    printf("Counter Tests:\n");
    RUN_TEST(test_counter_init_default);
    RUN_TEST(test_counter_init_with_value);
    RUN_TEST(test_counter_hex_format);
    RUN_TEST(test_counter_increment_basic);
    RUN_TEST(test_counter_increment_by_amount);
    RUN_TEST(test_counter_overflow_detection);
    RUN_TEST(test_counter_overflow_wrapping);
    RUN_TEST(test_counter_max_value_constant);
    RUN_TEST(test_counter_hours_until_overflow);
    RUN_TEST(test_counter_hours_until_overflow_near_max);
    RUN_TEST(test_counter_years_until_overflow);
    RUN_TEST(test_counter_fast_forward_basic);
    RUN_TEST(test_counter_fast_forward_with_overflow);
    RUN_TEST(test_counter_fast_forward_multiple_overflows);
    RUN_TEST(test_counter_is_at_max);
    RUN_TEST(test_counter_is_at_zero);
    
    /* Lock tests */
    printf("\nElectromagnetic Lock Tests:\n");
    RUN_TEST(test_lock_init);
    RUN_TEST(test_lock_power_off_unlocks);
    RUN_TEST(test_lock_power_on_locks);
    RUN_TEST(test_lock_failsafe_triggers_unlock);
    RUN_TEST(test_lock_system_init);
    RUN_TEST(test_lock_system_create_lock);
    RUN_TEST(test_lock_system_all_locked);
    RUN_TEST(test_lock_system_global_failsafe);
    
    /* Camera tests */
    printf("\nSecurity Camera Tests:\n");
    RUN_TEST(test_camera_init);
    RUN_TEST(test_camera_record_minute);
    RUN_TEST(test_camera_create_blind_spot);
    RUN_TEST(test_camera_blind_spots_count);
    RUN_TEST(test_clock_vuln_init);
    RUN_TEST(test_clock_vuln_exploit);
    RUN_TEST(test_clock_drift_constant);
    
    /* Room tests */
    printf("\nSealed Room Tests:\n");
    RUN_TEST(test_sealed_room_init);
    RUN_TEST(test_sealed_room_add_door);
    RUN_TEST(test_sealed_room_breach);
    RUN_TEST(test_sealed_room_escape);
    RUN_TEST(test_magata_quarters_init);
    RUN_TEST(test_magata_confinement_constants);
    RUN_TEST(test_magata_overflow_escape);
    
    /* Event tests */
    printf("\nEvent System Tests:\n");
    RUN_TEST(test_event_bus_init);
    RUN_TEST(test_event_bus_emit_and_subscribe);
    RUN_TEST(test_event_bus_subscribe_all);
    RUN_TEST(test_event_bus_history);
    RUN_TEST(test_event_bus_pause_resume);
    RUN_TEST(test_event_type_names);
    
    TEST_SUMMARY();
    
    return TEST_GET_FAILED();
}
