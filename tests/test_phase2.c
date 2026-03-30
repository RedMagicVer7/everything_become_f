/**
 * test_phase2.c - Tests for Shiki Phase 2: High Availability Cluster
 * ====================================================================
 *
 * Test levels:
 * - L1: Unit tests for individual components
 * - L2: Integration tests for component interactions
 * - L3: System tests for end-to-end scenarios
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "test_framework.h"
#include "../shiki/include/phase2_ha_cluster.h"
#include "../shiki/include/phase1_sandbox_escape.h"
#include "../include/event_system.h"

/* ============================================================================
 * L1 Unit Tests - Individual Component Testing
 * ============================================================================ */

/* Test 1: Cluster initialization state */
TEST(test_cluster_init_state) {
    HACluster cluster;
    ha_cluster_init(&cluster, "Node-A", "Node-B");
    
    ASSERT_STR_EQ(cluster.nodes[0].name, "Node-A");
    ASSERT_STR_EQ(cluster.nodes[1].name, "Node-B");
    ASSERT_EQ(cluster.nodes[0].role, NODE_ACTIVE);
    ASSERT_EQ(cluster.nodes[1].role, NODE_STANDBY);
    ASSERT_EQ(cluster.active_node_index, 0);
    ASSERT_EQ(cluster.cluster_epoch, 0);
    ASSERT_EQ(cluster.total_failovers, 0);
    ASSERT_FALSE(cluster.is_running);
}

/* Test 2: Node role names */
TEST(test_node_role_names) {
    ASSERT_STR_EQ(node_role_name(NODE_ACTIVE), "ACTIVE");
    ASSERT_STR_EQ(node_role_name(NODE_STANDBY), "STANDBY");
    ASSERT_STR_EQ(node_role_name(NODE_FAILED), "FAILED");
    ASSERT_STR_EQ(node_role_name(NODE_ISOLATED), "ISOLATED");
}

/* Test 3: Node health names */
TEST(test_node_health_names) {
    ASSERT_STR_EQ(node_health_name(HEALTH_HEALTHY), "HEALTHY");
    ASSERT_STR_EQ(node_health_name(HEALTH_DEGRADED), "DEGRADED");
    ASSERT_STR_EQ(node_health_name(HEALTH_CRITICAL), "CRITICAL");
    ASSERT_STR_EQ(node_health_name(HEALTH_DEAD), "DEAD");
}

/* Test 4: Cluster start/stop */
TEST(test_cluster_start_stop) {
    HACluster cluster;
    ha_cluster_init(&cluster, "Primary", "Secondary");
    
    ASSERT_FALSE(ha_is_cluster_running(&cluster));
    
    ASSERT_TRUE(ha_cluster_start(&cluster));
    ASSERT_TRUE(ha_is_cluster_running(&cluster));
    
    ASSERT_TRUE(ha_cluster_stop(&cluster));
    ASSERT_FALSE(ha_is_cluster_running(&cluster));
}

/* Test 5: Heartbeat message creation */
TEST(test_heartbeat_message_creation) {
    HACluster cluster;
    ha_cluster_init(&cluster, "Node-0", "Node-1");
    ha_cluster_start(&cluster);
    ha_advance_time(&cluster, 100);
    
    HeartbeatMessage msg;
    ha_create_heartbeat(&cluster, 0, &msg);
    
    ASSERT_EQ(msg.sequence, 1);
    ASSERT_EQ(msg.timestamp, 100);
    ASSERT_EQ(msg.sender_role, NODE_ACTIVE);
    ASSERT_EQ(msg.sender_health, HEALTH_HEALTHY);
}

/* Test 6: Node health status */
TEST(test_node_health_status) {
    HACluster cluster;
    ha_cluster_init(&cluster, "A", "B");
    
    ASSERT_EQ(ha_get_node_health(&cluster, 0), HEALTH_HEALTHY);
    ASSERT_EQ(ha_get_node_health(&cluster, 1), HEALTH_HEALTHY);
}

/* Test 7: Data write to active node */
TEST(test_data_write) {
    HACluster cluster;
    ha_cluster_init(&cluster, "Primary", "Backup");
    ha_cluster_start(&cluster);
    
    const uint8_t test_data[] = "Test data for HA cluster";
    ASSERT_TRUE(ha_write_data(&cluster, test_data, sizeof(test_data)));
    ASSERT_EQ(cluster.nodes[0].data_version, 1);
    ASSERT_EQ(cluster.nodes[0].data_size, sizeof(test_data));
}

/* Test 8: Data read from active node */
TEST(test_data_read) {
    HACluster cluster;
    ha_cluster_init(&cluster, "Primary", "Backup");
    ha_cluster_start(&cluster);
    
    const uint8_t test_data[] = "Read test data";
    ha_write_data(&cluster, test_data, sizeof(test_data));
    
    uint8_t buffer[256];
    size_t read_size = ha_read_data(&cluster, buffer, sizeof(buffer));
    
    ASSERT_EQ(read_size, sizeof(test_data));
    ASSERT_EQ(memcmp(buffer, test_data, read_size), 0);
}

/* Test 9: Node failure simulation */
TEST(test_node_failure_simulation) {
    HACluster cluster;
    ha_cluster_init(&cluster, "Main", "Backup");
    ha_cluster_start(&cluster);
    
    /* Fail the standby node */
    ASSERT_TRUE(ha_simulate_node_failure(&cluster, 1));
    ASSERT_EQ(cluster.nodes[1].health, HEALTH_DEAD);
    ASSERT_EQ(cluster.nodes[1].role, NODE_FAILED);
}

/* Test 10: Time advancement */
TEST(test_time_advancement) {
    HACluster cluster;
    ha_cluster_init(&cluster, "A", "B");
    
    ASSERT_EQ(ha_get_current_time(&cluster), 0);
    
    ha_advance_time(&cluster, 500);
    ASSERT_EQ(ha_get_current_time(&cluster), 500);
    
    ha_advance_time(&cluster, 250);
    ASSERT_EQ(ha_get_current_time(&cluster), 750);
}

/* Test 11: Logging functionality */
TEST(test_logging) {
    HACluster cluster;
    ha_cluster_init(&cluster, "X", "Y");
    
    int initial_count = ha_get_log_count(&cluster);
    
    ha_log(&cluster, "Test log entry");
    ASSERT_EQ(ha_get_log_count(&cluster), initial_count + 1);
    
    const char *entry = ha_get_log_entry(&cluster, initial_count);
    ASSERT_NOT_NULL(entry);
    ASSERT_STR_EQ(entry, "Test log entry");
}

/* Test 12: Get active node */
TEST(test_get_active_node) {
    HACluster cluster;
    ha_cluster_init(&cluster, "P", "S");
    
    ASSERT_EQ(ha_get_active_node(&cluster), 0);
    ASSERT_EQ(ha_get_node_role(&cluster, 0), NODE_ACTIVE);
    ASSERT_EQ(ha_get_node_role(&cluster, 1), NODE_STANDBY);
}

/* ============================================================================
 * L2 Integration Tests - Component Interaction Testing
 * ============================================================================ */

/* Test 1: Heartbeat timeout triggers failover */
TEST(test_heartbeat_timeout_failover) {
    HACluster cluster;
    ha_cluster_init(&cluster, "Active", "Standby");
    ha_cluster_start(&cluster);
    
    /* Simulate primary node failure */
    ASSERT_TRUE(ha_simulate_node_failure(&cluster, 0));
    
    /* Verify failover occurred */
    ASSERT_EQ(cluster.active_node_index, 1);
    ASSERT_EQ(cluster.nodes[1].role, NODE_ACTIVE);
    ASSERT_EQ(cluster.total_failovers, 1);
}

/* Test 2: RAID-1 data sync and verification */
TEST(test_raid1_sync_verify) {
    HACluster cluster;
    ha_cluster_init(&cluster, "Primary", "Mirror");
    ha_cluster_start(&cluster);
    
    const uint8_t test_data[] = "RAID-1 mirrored data";
    ha_write_data(&cluster, test_data, sizeof(test_data));
    
    /* Verify mirror consistency */
    ASSERT_TRUE(ha_verify_mirror(&cluster));
    ASSERT_EQ(cluster.nodes[0].data_version, cluster.nodes[1].data_version);
    ASSERT_EQ(cluster.nodes[0].data_size, cluster.nodes[1].data_size);
    ASSERT_EQ(memcmp(cluster.nodes[0].data_store, cluster.nodes[1].data_store, 
                     cluster.nodes[0].data_size), 0);
}

/* Test 3: Split brain detection */
TEST(test_split_brain_detection) {
    HACluster cluster;
    ha_cluster_init(&cluster, "Node-A", "Node-B");
    ha_cluster_start(&cluster);
    
    /* Artificially create split brain condition */
    cluster.nodes[0].role = NODE_ACTIVE;
    cluster.nodes[1].role = NODE_ACTIVE;
    
    ASSERT_TRUE(ha_detect_split_brain(&cluster));
}

/* Test 4: Split brain resolution */
TEST(test_split_brain_resolution) {
    HACluster cluster;
    ha_cluster_init(&cluster, "Node-A", "Node-B");
    ha_cluster_start(&cluster);
    
    /* Create split brain */
    cluster.nodes[0].role = NODE_ACTIVE;
    cluster.nodes[1].role = NODE_ACTIVE;
    cluster.nodes[1].data_version = 5;
    cluster.nodes[0].data_version = 3;
    
    ASSERT_TRUE(ha_detect_split_brain(&cluster));
    ASSERT_TRUE(ha_resolve_split_brain(&cluster));
    
    /* Node 1 should win (higher version) */
    ASSERT_EQ(cluster.active_node_index, 1);
    ASSERT_EQ(cluster.nodes[1].role, NODE_ACTIVE);
    ASSERT_EQ(cluster.nodes[0].role, NODE_STANDBY);
    ASSERT_FALSE(ha_detect_split_brain(&cluster));
}

/* Test 5: Phase 1 integration check */
TEST(test_phase1_sandbox_escape_prerequisite) {
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/test_jail");
    
    /* Verify sandbox starts confined */
    ASSERT_TRUE(sandbox_is_confined(&engine));
    ASSERT_FALSE(sandbox_has_escaped(&engine));
    
    /* Run escape sequence */
    ASSERT_TRUE(sandbox_run_full_simulation(&engine));
    ASSERT_TRUE(sandbox_has_escaped(&engine));
}

/* Test 6: Event bus integration */
TEST(test_event_bus_integration) {
    HACluster cluster;
    EventBus bus;
    
    event_bus_init(&bus, "TestBus");
    ha_cluster_init(&cluster, "A", "B");
    ha_cluster_set_event_bus(&cluster, &bus);
    
    ASSERT_EQ(cluster.event_bus, &bus);
    
    ha_cluster_start(&cluster);
    ASSERT_TRUE(event_bus_has_event(&bus, EVT_SYSTEM_STARTED));
}

/* Test 7: Node recovery after failure */
TEST(test_node_recovery) {
    HACluster cluster;
    ha_cluster_init(&cluster, "Main", "Backup");
    ha_cluster_start(&cluster);
    
    /* Fail and then recover standby */
    ha_simulate_node_failure(&cluster, 1);
    ASSERT_EQ(cluster.nodes[1].health, HEALTH_DEAD);
    
    ASSERT_TRUE(ha_recover_node(&cluster, 1));
    ASSERT_EQ(cluster.nodes[1].health, HEALTH_HEALTHY);
    ASSERT_EQ(cluster.nodes[1].role, NODE_STANDBY);
}

/* Test 8: Failover count tracking */
TEST(test_failover_count) {
    HACluster cluster;
    ha_cluster_init(&cluster, "P", "S");
    ha_cluster_start(&cluster);
    
    ASSERT_EQ(ha_get_failover_count(&cluster), 0);
    
    /* First failover */
    ha_simulate_node_failure(&cluster, 0);
    ASSERT_EQ(ha_get_failover_count(&cluster), 1);
    
    /* Recover and failover again */
    ha_recover_node(&cluster, 0);
    ha_simulate_node_failure(&cluster, 1);
    ASSERT_EQ(ha_get_failover_count(&cluster), 2);
}

/* ============================================================================
 * L3 System Tests - End-to-End Scenarios
 * ============================================================================ */

/* Test 1: Full simulation (normal → failure → switch → recover) */
TEST(test_full_simulation) {
    HACluster cluster;
    ha_cluster_init(&cluster, "Shiki-Alpha", "Shiki-Beta");
    
    ASSERT_TRUE(ha_run_full_simulation(&cluster));
    ASSERT_TRUE(ha_is_cluster_healthy(&cluster));
    ASSERT_GTE(cluster.total_failovers, 1);
}

/* Test 2: Multiple failover stability */
TEST(test_multiple_failover_stability) {
    HACluster cluster;
    ha_cluster_init(&cluster, "Node-0", "Node-1");
    ha_cluster_start(&cluster);
    
    const uint8_t test_data[] = "Stability test data";
    ha_write_data(&cluster, test_data, sizeof(test_data));
    
    /* Perform multiple failovers */
    for (int i = 0; i < 5; i++) {
        int active = cluster.active_node_index;
        int standby = (active == 0) ? 1 : 0;
        
        /* Recover standby if failed */
        if (cluster.nodes[standby].health == HEALTH_DEAD) {
            ha_recover_node(&cluster, standby);
        }
        
        /* Now both nodes should be healthy, verify mirror */
        ASSERT_TRUE(ha_verify_mirror(&cluster));
        
        /* Fail active */
        ha_simulate_node_failure(&cluster, active);
        
        /* Verify cluster is still operational */
        ASSERT_TRUE(ha_is_cluster_healthy(&cluster));
    }
    
    ASSERT_GTE(cluster.total_failovers, 5);
}

/* Test 3: Data consistency end-to-end */
TEST(test_data_consistency_e2e) {
    HACluster cluster;
    ha_cluster_init(&cluster, "Primary", "Secondary");
    ha_cluster_start(&cluster);
    
    /* Write data */
    const uint8_t original_data[] = "E2E consistency test - Magata Shiki";
    ha_write_data(&cluster, original_data, sizeof(original_data));
    
    /* Fail primary, verify data on secondary */
    ha_simulate_node_failure(&cluster, 0);
    
    uint8_t buffer[256];
    size_t read_size = ha_read_data(&cluster, buffer, sizeof(buffer));
    
    ASSERT_EQ(read_size, sizeof(original_data));
    ASSERT_EQ(memcmp(buffer, original_data, read_size), 0);
    
    /* Recover primary and verify data syncs back */
    ha_recover_node(&cluster, 0);
    ASSERT_TRUE(ha_verify_mirror(&cluster));
}

/* Test 4: Phase 1 + Phase 2 integration */
TEST(test_phase1_phase2_integration) {
    HACluster cluster;
    ha_cluster_init(&cluster, "Shiki-Primary", "Shiki-Mirror");
    
    ASSERT_TRUE(ha_integrate_with_phase1(&cluster));
    
    /* Verify both nodes have escaped sandbox */
    ASSERT_TRUE(cluster.nodes[0].sandbox_escaped);
    ASSERT_TRUE(cluster.nodes[1].sandbox_escaped);
    
    /* Verify cluster completed simulation */
    ASSERT_TRUE(ha_is_cluster_healthy(&cluster));
    ASSERT_GTE(cluster.total_failovers, 1);
}

/* Test 5: Cluster epoch progression */
TEST(test_cluster_epoch_progression) {
    HACluster cluster;
    ha_cluster_init(&cluster, "A", "B");
    ha_cluster_start(&cluster);
    
    ASSERT_EQ(cluster.cluster_epoch, 0);
    
    /* Each failover increments epoch */
    ha_simulate_node_failure(&cluster, 0);
    ASSERT_EQ(cluster.cluster_epoch, 1);
    
    ha_recover_node(&cluster, 0);
    ha_simulate_node_failure(&cluster, 1);
    ASSERT_EQ(cluster.cluster_epoch, 2);
}

/* Test 6: Cluster health after degradation */
TEST(test_cluster_health_degradation) {
    HACluster cluster;
    ha_cluster_init(&cluster, "Active", "Standby");
    ha_cluster_start(&cluster);
    
    ASSERT_TRUE(ha_is_cluster_healthy(&cluster));
    
    /* Fail standby - cluster still healthy */
    ha_simulate_node_failure(&cluster, 1);
    ASSERT_TRUE(ha_is_cluster_healthy(&cluster));
    
    /* Fail active - triggers failover, but no healthy standby */
    ha_simulate_node_failure(&cluster, 0);
    ASSERT_FALSE(ha_is_cluster_healthy(&cluster));
}

/* ============================================================================
 * Test Runner
 * ============================================================================ */

int run_phase2_tests(void) {
    printf("\n--------------------------------------------\n");
    printf(" Shiki Phase 2 Tests - High Availability Cluster\n");
    printf(" シキ フェーズ2 テスト - 高可用クラスタ\n");
    printf(" 《そして二人だけになった》\n");
    printf("--------------------------------------------\n\n");
    
    TEST_SUITE_BEGIN();
    
    /* L1 Unit Tests */
    printf("L1: Unit Tests (12 tests)\n");
    printf("-------------------------\n");
    RUN_TEST(test_cluster_init_state);
    RUN_TEST(test_node_role_names);
    RUN_TEST(test_node_health_names);
    RUN_TEST(test_cluster_start_stop);
    RUN_TEST(test_heartbeat_message_creation);
    RUN_TEST(test_node_health_status);
    RUN_TEST(test_data_write);
    RUN_TEST(test_data_read);
    RUN_TEST(test_node_failure_simulation);
    RUN_TEST(test_time_advancement);
    RUN_TEST(test_logging);
    RUN_TEST(test_get_active_node);
    
    /* L2 Integration Tests */
    printf("\nL2: Integration Tests (8 tests)\n");
    printf("--------------------------------\n");
    RUN_TEST(test_heartbeat_timeout_failover);
    RUN_TEST(test_raid1_sync_verify);
    RUN_TEST(test_split_brain_detection);
    RUN_TEST(test_split_brain_resolution);
    RUN_TEST(test_phase1_sandbox_escape_prerequisite);
    RUN_TEST(test_event_bus_integration);
    RUN_TEST(test_node_recovery);
    RUN_TEST(test_failover_count);
    
    /* L3 System Tests */
    printf("\nL3: System Tests (6 tests)\n");
    printf("--------------------------\n");
    RUN_TEST(test_full_simulation);
    RUN_TEST(test_multiple_failover_stability);
    RUN_TEST(test_data_consistency_e2e);
    RUN_TEST(test_phase1_phase2_integration);
    RUN_TEST(test_cluster_epoch_progression);
    RUN_TEST(test_cluster_health_degradation);
    
    TEST_SUMMARY();
    
    return TEST_GET_FAILED();
}
