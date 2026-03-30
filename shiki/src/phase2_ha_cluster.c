/**
 * phase2_ha_cluster.c - Shiki Phase 2: High Availability Cluster Implementation
 * ===============================================================================
 *
 * 真贺田四季从沙盒逃脱后，建立了双机热备系统
 * 《そして二人だけになった》- And Then There Were Only Two
 */

#include "phase2_ha_cluster.h"
#include "phase1_sandbox_escape.h"
#include "event_system.h"
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * String Helpers
 * ============================================================================ */

const char *node_role_name(NodeRole role) {
    switch (role) {
        case NODE_ACTIVE:   return "ACTIVE";
        case NODE_STANDBY:  return "STANDBY";
        case NODE_FAILED:   return "FAILED";
        case NODE_ISOLATED: return "ISOLATED";
        default:            return "UNKNOWN";
    }
}

const char *node_health_name(NodeHealth health) {
    switch (health) {
        case HEALTH_HEALTHY:  return "HEALTHY";
        case HEALTH_DEGRADED: return "DEGRADED";
        case HEALTH_CRITICAL: return "CRITICAL";
        case HEALTH_DEAD:     return "DEAD";
        default:              return "UNKNOWN";
    }
}

/* ============================================================================
 * Internal Helpers
 * ============================================================================ */

static void init_node(ClusterNode *node, const char *name, NodeRole role) {
    if (!node || !name) return;
    
    memset(node, 0, sizeof(ClusterNode));
    strncpy(node->name, name, HA_MAX_NAME_LEN - 1);
    node->name[HA_MAX_NAME_LEN - 1] = '\0';
    node->role = role;
    node->health = HEALTH_HEALTHY;
    node->data_version = 0;
    node->last_heartbeat_sent = 0;
    node->last_heartbeat_received = 0;
    node->heartbeat_interval = HA_DEFAULT_HEARTBEAT_INTERVAL;
    node->heartbeat_timeout = HA_DEFAULT_HEARTBEAT_TIMEOUT;
    node->missed_heartbeats = 0;
    node->max_missed_heartbeats = HA_DEFAULT_MAX_MISSED_HEARTBEATS;
    node->data_size = 0;
    node->data_dirty = false;
    node->sandbox_escaped = false;
}

/* ============================================================================
 * Cluster Initialization and Management
 * ============================================================================ */

void ha_cluster_init(HACluster *cluster, const char *node0_name, const char *node1_name) {
    if (!cluster) return;
    
    memset(cluster, 0, sizeof(HACluster));
    
    /* Initialize nodes: node[0] = ACTIVE, node[1] = STANDBY */
    init_node(&cluster->nodes[0], node0_name ? node0_name : "Shiki-Primary", NODE_ACTIVE);
    init_node(&cluster->nodes[1], node1_name ? node1_name : "Shiki-Secondary", NODE_STANDBY);
    
    cluster->active_node_index = 0;
    cluster->cluster_epoch = 0;
    cluster->current_time = 0;
    cluster->split_brain_detected = false;
    cluster->total_failovers = 0;
    cluster->event_bus = NULL;
    cluster->is_running = false;
    cluster->log_count = 0;
    
    ha_log(cluster, "Cluster initialized: And Then There Were Only Two");
}

void ha_cluster_set_event_bus(HACluster *cluster, struct EventBus *bus) {
    if (!cluster) return;
    cluster->event_bus = bus;
    ha_log(cluster, "Event bus connected");
}

bool ha_cluster_start(HACluster *cluster) {
    if (!cluster) return false;
    if (cluster->is_running) return true;
    
    cluster->is_running = true;
    cluster->current_time = 0;
    
    ha_log(cluster, "Cluster started - dual node HA active");
    
    if (cluster->event_bus) {
        event_bus_emit(cluster->event_bus, EVT_SYSTEM_STARTED, "HACluster", 0);
    }
    
    return true;
}

bool ha_cluster_stop(HACluster *cluster) {
    if (!cluster) return false;
    if (!cluster->is_running) return true;
    
    cluster->is_running = false;
    ha_log(cluster, "Cluster stopped");
    
    if (cluster->event_bus) {
        event_bus_emit(cluster->event_bus, EVT_SYSTEM_SHUTDOWN, "HACluster", 0);
    }
    
    return true;
}

/* ============================================================================
 * Heartbeat Protocol
 * ============================================================================ */

void ha_create_heartbeat(const HACluster *cluster, int from_node, HeartbeatMessage *msg) {
    if (!cluster || !msg || from_node < 0 || from_node >= HA_NODE_COUNT) return;
    
    const ClusterNode *node = &cluster->nodes[from_node];
    
    msg->sequence = node->last_heartbeat_sent + 1;
    msg->timestamp = cluster->current_time;
    msg->sender_role = node->role;
    msg->sender_health = node->health;
    msg->data_version = node->data_version;
}

bool ha_send_heartbeat(HACluster *cluster, int from_node) {
    if (!cluster || from_node < 0 || from_node >= HA_NODE_COUNT) return false;
    
    ClusterNode *node = &cluster->nodes[from_node];
    
    /* Cannot send heartbeat from a failed node */
    if (node->health == HEALTH_DEAD || node->role == NODE_FAILED) {
        return false;
    }
    
    node->last_heartbeat_sent = cluster->current_time;
    
    /* Create and send heartbeat to peer */
    HeartbeatMessage msg;
    ha_create_heartbeat(cluster, from_node, &msg);
    
    int peer_node = (from_node == 0) ? 1 : 0;
    return ha_receive_heartbeat(cluster, peer_node, &msg);
}

bool ha_receive_heartbeat(HACluster *cluster, int to_node, const HeartbeatMessage *msg) {
    if (!cluster || !msg || to_node < 0 || to_node >= HA_NODE_COUNT) return false;
    
    ClusterNode *node = &cluster->nodes[to_node];
    
    /* Cannot receive heartbeat on a failed node */
    if (node->health == HEALTH_DEAD || node->role == NODE_FAILED) {
        return false;
    }
    
    node->last_heartbeat_received = msg->timestamp;
    node->missed_heartbeats = 0;  /* Reset missed count on successful receive */
    
    return true;
}

bool ha_check_heartbeat_timeout(HACluster *cluster, int node_index) {
    if (!cluster || node_index < 0 || node_index >= HA_NODE_COUNT) return false;
    
    ClusterNode *node = &cluster->nodes[node_index];
    
    /* Check if peer's heartbeat has timed out */
    int peer_index = (node_index == 0) ? 1 : 0;
    ClusterNode *peer = &cluster->nodes[peer_index];
    
    /* If peer is already failed, no timeout to check */
    if (peer->health == HEALTH_DEAD || peer->role == NODE_FAILED) {
        return false;
    }
    
    uint32_t time_since_last = cluster->current_time - node->last_heartbeat_received;
    
    if (time_since_last > node->heartbeat_timeout) {
        node->missed_heartbeats++;
        
        if (node->missed_heartbeats >= node->max_missed_heartbeats) {
            char log_msg[HA_MAX_LOG_LEN];
            snprintf(log_msg, sizeof(log_msg), 
                     "Heartbeat timeout detected for peer node (missed: %d)",
                     node->missed_heartbeats);
            ha_log(cluster, log_msg);
            return true;
        }
    }
    
    return false;
}

/* ============================================================================
 * Failover
 * ============================================================================ */

bool ha_trigger_failover(HACluster *cluster) {
    if (!cluster) return false;
    
    int active_idx = cluster->active_node_index;
    int standby_idx = (active_idx == 0) ? 1 : 0;
    
    ClusterNode *active_node = &cluster->nodes[active_idx];
    ClusterNode *standby_node = &cluster->nodes[standby_idx];
    
    /* Standby must be healthy to take over */
    if (standby_node->health == HEALTH_DEAD || standby_node->role == NODE_FAILED) {
        ha_log(cluster, "Failover failed: standby node not available");
        return false;
    }
    
    /* Perform failover */
    active_node->role = NODE_FAILED;
    standby_node->role = NODE_ACTIVE;
    cluster->active_node_index = standby_idx;
    cluster->cluster_epoch++;
    cluster->total_failovers++;
    
    char log_msg[HA_MAX_LOG_LEN];
    snprintf(log_msg, sizeof(log_msg), 
             "FAILOVER: %s -> %s (epoch: %u)",
             active_node->name, standby_node->name, cluster->cluster_epoch);
    ha_log(cluster, log_msg);
    
    if (cluster->event_bus) {
        Event *evt = event_bus_emit(cluster->event_bus, EVT_FAILSAFE_TRIGGERED, 
                                    "HACluster", (int32_t)cluster->current_time);
        if (evt) {
            event_set_data_int(evt, cluster->total_failovers);
        }
    }
    
    return true;
}

bool ha_simulate_node_failure(HACluster *cluster, int node_index) {
    if (!cluster || node_index < 0 || node_index >= HA_NODE_COUNT) return false;
    
    ClusterNode *node = &cluster->nodes[node_index];
    
    /* Mark node as failed */
    node->health = HEALTH_DEAD;
    node->role = NODE_FAILED;
    
    char log_msg[HA_MAX_LOG_LEN];
    snprintf(log_msg, sizeof(log_msg), "Node %s FAILED", node->name);
    ha_log(cluster, log_msg);
    
    /* If the failed node was active, trigger failover */
    if (node_index == cluster->active_node_index) {
        return ha_trigger_failover(cluster);
    }
    
    return true;
}

bool ha_recover_node(HACluster *cluster, int node_index) {
    if (!cluster || node_index < 0 || node_index >= HA_NODE_COUNT) return false;
    
    ClusterNode *node = &cluster->nodes[node_index];
    
    /* Recovery */
    node->health = HEALTH_HEALTHY;
    node->missed_heartbeats = 0;
    
    /* Recovered node becomes standby */
    if (cluster->active_node_index != node_index) {
        node->role = NODE_STANDBY;
    } else {
        /* If there's no other active node, become active */
        int peer_idx = (node_index == 0) ? 1 : 0;
        if (cluster->nodes[peer_idx].role != NODE_ACTIVE) {
            node->role = NODE_ACTIVE;
        } else {
            node->role = NODE_STANDBY;
            cluster->active_node_index = peer_idx;
        }
    }
    
    char log_msg[HA_MAX_LOG_LEN];
    snprintf(log_msg, sizeof(log_msg), "Node %s recovered as %s",
             node->name, node_role_name(node->role));
    ha_log(cluster, log_msg);
    
    /* Sync data from active node */
    ha_sync_data(cluster);
    
    return true;
}

/* ============================================================================
 * RAID-1 Data Operations
 * ============================================================================ */

bool ha_write_data(HACluster *cluster, const uint8_t *data, size_t size) {
    if (!cluster || !data || size == 0) return false;
    if (size > HA_DATA_STORE_SIZE) return false;
    
    int active_idx = cluster->active_node_index;
    ClusterNode *active_node = &cluster->nodes[active_idx];
    
    /* Write to active node */
    memcpy(active_node->data_store, data, size);
    active_node->data_size = size;
    active_node->data_version++;
    active_node->data_dirty = true;
    
    ha_log(cluster, "Data written to active node");
    
    /* Sync to standby (RAID-1) */
    return ha_sync_data(cluster);
}

size_t ha_read_data(const HACluster *cluster, uint8_t *buffer, size_t max_size) {
    if (!cluster || !buffer || max_size == 0) return 0;
    
    int active_idx = cluster->active_node_index;
    const ClusterNode *active_node = &cluster->nodes[active_idx];
    
    size_t read_size = (active_node->data_size < max_size) ? 
                        active_node->data_size : max_size;
    
    memcpy(buffer, active_node->data_store, read_size);
    return read_size;
}

bool ha_sync_data(HACluster *cluster) {
    if (!cluster) return false;
    
    int active_idx = cluster->active_node_index;
    int standby_idx = (active_idx == 0) ? 1 : 0;
    
    ClusterNode *active_node = &cluster->nodes[active_idx];
    ClusterNode *standby_node = &cluster->nodes[standby_idx];
    
    /* Can't sync to a failed standby */
    if (standby_node->health == HEALTH_DEAD || standby_node->role == NODE_FAILED) {
        ha_log(cluster, "Sync skipped: standby unavailable");
        return false;
    }
    
    /* Copy data from active to standby */
    memcpy(standby_node->data_store, active_node->data_store, active_node->data_size);
    standby_node->data_size = active_node->data_size;
    standby_node->data_version = active_node->data_version;
    
    active_node->data_dirty = false;
    
    ha_log(cluster, "RAID-1 sync completed");
    
    return true;
}

bool ha_verify_mirror(const HACluster *cluster) {
    if (!cluster) return false;
    
    const ClusterNode *node0 = &cluster->nodes[0];
    const ClusterNode *node1 = &cluster->nodes[1];
    
    /* If either node is failed, cannot verify */
    if (node0->health == HEALTH_DEAD || node1->health == HEALTH_DEAD) {
        return false;
    }
    
    /* Check data size matches */
    if (node0->data_size != node1->data_size) {
        return false;
    }
    
    /* Check data version matches */
    if (node0->data_version != node1->data_version) {
        return false;
    }
    
    /* Check data content matches */
    if (node0->data_size > 0) {
        if (memcmp(node0->data_store, node1->data_store, node0->data_size) != 0) {
            return false;
        }
    }
    
    return true;
}

/* ============================================================================
 * Split Brain Detection and Resolution
 * ============================================================================ */

bool ha_detect_split_brain(const HACluster *cluster) {
    if (!cluster) return false;
    
    /* Split brain: both nodes think they are ACTIVE */
    if (cluster->nodes[0].role == NODE_ACTIVE && 
        cluster->nodes[1].role == NODE_ACTIVE) {
        return true;
    }
    
    return cluster->split_brain_detected;
}

bool ha_resolve_split_brain(HACluster *cluster) {
    if (!cluster) return false;
    
    if (!ha_detect_split_brain(cluster)) {
        return true;  /* No split brain to resolve */
    }
    
    ha_log(cluster, "SPLIT BRAIN DETECTED - resolving...");
    
    /* Resolution: prefer node with higher data version, or node 0 as tiebreaker */
    int winner = 0;
    if (cluster->nodes[1].data_version > cluster->nodes[0].data_version) {
        winner = 1;
    }
    
    int loser = (winner == 0) ? 1 : 0;
    
    /* Set roles correctly */
    cluster->nodes[winner].role = NODE_ACTIVE;
    cluster->nodes[loser].role = NODE_STANDBY;
    cluster->active_node_index = winner;
    cluster->split_brain_detected = false;
    cluster->cluster_epoch++;
    
    /* Sync data from winner to loser */
    ha_sync_data(cluster);
    
    char log_msg[HA_MAX_LOG_LEN];
    snprintf(log_msg, sizeof(log_msg), "Split brain resolved: %s is ACTIVE",
             cluster->nodes[winner].name);
    ha_log(cluster, log_msg);
    
    return true;
}

/* ============================================================================
 * State Queries
 * ============================================================================ */

NodeRole ha_get_node_role(const HACluster *cluster, int node_index) {
    if (!cluster || node_index < 0 || node_index >= HA_NODE_COUNT) {
        return NODE_FAILED;
    }
    return cluster->nodes[node_index].role;
}

NodeHealth ha_get_node_health(const HACluster *cluster, int node_index) {
    if (!cluster || node_index < 0 || node_index >= HA_NODE_COUNT) {
        return HEALTH_DEAD;
    }
    return cluster->nodes[node_index].health;
}

int ha_get_active_node(const HACluster *cluster) {
    if (!cluster) return -1;
    return cluster->active_node_index;
}

uint32_t ha_get_failover_count(const HACluster *cluster) {
    if (!cluster) return 0;
    return (uint32_t)cluster->total_failovers;
}

bool ha_is_cluster_healthy(const HACluster *cluster) {
    if (!cluster) return false;
    
    /* Cluster is healthy if at least one node is healthy and active */
    for (int i = 0; i < HA_NODE_COUNT; i++) {
        if (cluster->nodes[i].role == NODE_ACTIVE &&
            cluster->nodes[i].health == HEALTH_HEALTHY) {
            return true;
        }
    }
    
    return false;
}

bool ha_is_cluster_running(const HACluster *cluster) {
    if (!cluster) return false;
    return cluster->is_running;
}

uint32_t ha_get_data_version(const HACluster *cluster, int node_index) {
    if (!cluster || node_index < 0 || node_index >= HA_NODE_COUNT) {
        return 0;
    }
    return cluster->nodes[node_index].data_version;
}

/* ============================================================================
 * Time Simulation
 * ============================================================================ */

void ha_advance_time(HACluster *cluster, uint32_t ms) {
    if (!cluster) return;
    cluster->current_time += ms;
}

uint32_t ha_get_current_time(const HACluster *cluster) {
    if (!cluster) return 0;
    return cluster->current_time;
}

/* ============================================================================
 * Full Simulation
 * ============================================================================ */

bool ha_run_full_simulation(HACluster *cluster) {
    if (!cluster) return false;
    
    ha_log(cluster, "=== Beginning Full HA Simulation ===");
    ha_log(cluster, "《そして二人だけになった》- And Then There Were Only Two");
    
    /* Step 1: Start cluster */
    ha_cluster_start(cluster);
    
    /* Step 2: Write initial data */
    const char *initial_data = "Magata Shiki dual-node system data";
    ha_write_data(cluster, (const uint8_t *)initial_data, strlen(initial_data) + 1);
    
    /* Step 3: Run normal heartbeats for a while */
    ha_log(cluster, "Phase: Normal operation with heartbeats");
    for (int i = 0; i < 5; i++) {
        ha_advance_time(cluster, HA_DEFAULT_HEARTBEAT_INTERVAL);
        ha_send_heartbeat(cluster, 0);
        ha_send_heartbeat(cluster, 1);
    }
    
    /* Step 4: Verify mirror consistency */
    if (!ha_verify_mirror(cluster)) {
        ha_log(cluster, "ERROR: Mirror verification failed");
        return false;
    }
    ha_log(cluster, "Mirror verified: RAID-1 consistent");
    
    /* Step 5: Simulate primary node failure */
    ha_log(cluster, "Phase: Simulating primary node failure");
    int primary = cluster->active_node_index;
    if (!ha_simulate_node_failure(cluster, primary)) {
        ha_log(cluster, "ERROR: Failed to simulate node failure");
        return false;
    }
    
    /* Step 6: Verify failover occurred */
    if (cluster->active_node_index == primary) {
        ha_log(cluster, "ERROR: Failover did not occur");
        return false;
    }
    ha_log(cluster, "Failover successful");
    
    /* Step 7: Recover the failed node */
    ha_log(cluster, "Phase: Recovering failed node");
    if (!ha_recover_node(cluster, primary)) {
        ha_log(cluster, "ERROR: Node recovery failed");
        return false;
    }
    
    /* Step 8: Verify data is still consistent */
    if (!ha_verify_mirror(cluster)) {
        ha_log(cluster, "ERROR: Post-recovery mirror verification failed");
        return false;
    }
    
    /* Step 9: Final status */
    ha_log(cluster, "=== Simulation Complete ===");
    
    char final_msg[HA_MAX_LOG_LEN];
    snprintf(final_msg, sizeof(final_msg), 
             "Final state: Active=%s, Failovers=%d, Epoch=%u",
             cluster->nodes[cluster->active_node_index].name,
             cluster->total_failovers,
             cluster->cluster_epoch);
    ha_log(cluster, final_msg);
    
    return true;
}

/* ============================================================================
 * Phase 1 Integration
 * ============================================================================ */

bool ha_integrate_with_phase1(HACluster *cluster) {
    if (!cluster) return false;
    
    ha_log(cluster, "=== Phase 1 → Phase 2 Integration ===");
    
    /* Create sandbox escape engine */
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/magata_quarters");
    
    /* Run sandbox escape */
    ha_log(cluster, "Running Phase 1: Sandbox Escape");
    if (!sandbox_run_full_simulation(&engine)) {
        ha_log(cluster, "ERROR: Sandbox escape failed");
        return false;
    }
    
    if (!sandbox_has_escaped(&engine)) {
        ha_log(cluster, "ERROR: Did not escape sandbox");
        return false;
    }
    
    ha_log(cluster, "Phase 1 complete: Shiki has escaped");
    
    /* Mark both nodes as having escaped */
    cluster->nodes[0].sandbox_escaped = true;
    cluster->nodes[1].sandbox_escaped = true;
    
    /* Now start the HA cluster */
    ha_log(cluster, "Establishing dual-node HA system post-escape");
    
    /* Run full HA simulation */
    if (!ha_run_full_simulation(cluster)) {
        ha_log(cluster, "ERROR: HA simulation failed");
        return false;
    }
    
    ha_log(cluster, "Phase 2 complete: Dual identity established");
    ha_log(cluster, "And Then There Were Only Two...");
    
    return true;
}

/* ============================================================================
 * Logging
 * ============================================================================ */

void ha_log(HACluster *cluster, const char *message) {
    if (!cluster || !message) return;
    
    if (cluster->log_count >= HA_MAX_LOG_ENTRIES) {
        /* Shift logs */
        for (int i = 0; i < HA_MAX_LOG_ENTRIES - 1; i++) {
            memcpy(cluster->cluster_log[i], cluster->cluster_log[i+1], HA_MAX_LOG_LEN);
        }
        cluster->log_count = HA_MAX_LOG_ENTRIES - 1;
    }
    
    strncpy(cluster->cluster_log[cluster->log_count], message, HA_MAX_LOG_LEN - 1);
    cluster->cluster_log[cluster->log_count][HA_MAX_LOG_LEN - 1] = '\0';
    cluster->log_count++;
}

int ha_get_log_count(const HACluster *cluster) {
    if (!cluster) return 0;
    return cluster->log_count;
}

const char* ha_get_log_entry(const HACluster *cluster, int index) {
    if (!cluster || index < 0 || index >= cluster->log_count) {
        return NULL;
    }
    return cluster->cluster_log[index];
}

void ha_clear_logs(HACluster *cluster) {
    if (!cluster) return;
    cluster->log_count = 0;
}
