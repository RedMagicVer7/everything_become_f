/**
 * phase2_main.c - Shiki Phase 2: High Availability Cluster Main
 * ===============================================================
 *
 * 独立可执行程序，运行Phase 2完整模拟
 *
 * 《そして二人だけになった》- And Then There Were Only Two
 *
 * Usage:
 *   ./phase2_ha_cluster              - Run full HA simulation
 *   ./phase2_ha_cluster --integrate  - Run with Phase 1 integration
 */

#include <stdio.h>
#include <string.h>
#include "phase2_ha_cluster.h"
#include "event_system.h"

/* ANSI color codes */
#define COLOR_RESET   "\033[0m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_RED     "\033[31m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_MAGENTA "\033[35m"

static void print_banner(void) {
    printf("\n");
    printf(COLOR_CYAN "╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                                                              ║\n");
    printf("║   " COLOR_MAGENTA "SHIKI PHASE 2: HIGH AVAILABILITY CLUSTER" COLOR_CYAN "                ║\n");
    printf("║                                                              ║\n");
    printf("║   " COLOR_YELLOW "《そして二人だけになった》" COLOR_CYAN "                              ║\n");
    printf("║   " COLOR_YELLOW "And Then There Were Only Two (1999)" COLOR_CYAN "                     ║\n");
    printf("║                                                              ║\n");
    printf("║   Dual-node HA system: Active-Standby failover             ║\n");
    printf("║   RAID-1 mirroring | Heartbeat protocol | Split brain       ║\n");
    printf("║                                                              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n" COLOR_RESET);
    printf("\n");
}

static void print_cluster_status(const HACluster *cluster) {
    printf("\n" COLOR_CYAN "--- Cluster Status ---" COLOR_RESET "\n");
    printf("Running: %s\n", cluster->is_running ? COLOR_GREEN "YES" COLOR_RESET : COLOR_RED "NO" COLOR_RESET);
    printf("Epoch: %u | Failovers: %d\n", cluster->cluster_epoch, cluster->total_failovers);
    printf("Active Node: %d (%s)\n", 
           cluster->active_node_index,
           cluster->nodes[cluster->active_node_index].name);
    
    printf("\n" COLOR_CYAN "--- Node Status ---" COLOR_RESET "\n");
    for (int i = 0; i < HA_NODE_COUNT; i++) {
        const ClusterNode *node = &cluster->nodes[i];
        const char *role_color = (node->role == NODE_ACTIVE) ? COLOR_GREEN :
                                 (node->role == NODE_STANDBY) ? COLOR_YELLOW :
                                 COLOR_RED;
        const char *health_color = (node->health == HEALTH_HEALTHY) ? COLOR_GREEN :
                                   (node->health == HEALTH_DEAD) ? COLOR_RED :
                                   COLOR_YELLOW;
        
        printf("Node %d [%s]:\n", i, node->name);
        printf("  Role: %s%s%s | Health: %s%s%s\n",
               role_color, node_role_name(node->role), COLOR_RESET,
               health_color, node_health_name(node->health), COLOR_RESET);
        printf("  Data Version: %u | Size: %zu bytes\n", 
               node->data_version, node->data_size);
        if (node->sandbox_escaped) {
            printf("  " COLOR_MAGENTA "Sandbox Escaped: YES" COLOR_RESET "\n");
        }
    }
    printf("\n");
}

static void print_logs(const HACluster *cluster) {
    int count = ha_get_log_count(cluster);
    if (count == 0) return;
    
    printf(COLOR_CYAN "--- Cluster Logs ---" COLOR_RESET "\n");
    for (int i = 0; i < count; i++) {
        const char *entry = ha_get_log_entry(cluster, i);
        if (entry) {
            printf("  [%d] %s\n", i, entry);
        }
    }
    printf("\n");
}

static int run_standalone_simulation(void) {
    printf(COLOR_GREEN "Running standalone HA simulation...\n" COLOR_RESET);
    
    HACluster cluster;
    EventBus event_bus;
    
    /* Initialize event bus */
    event_bus_init(&event_bus, "Phase2EventBus");
    
    /* Initialize cluster */
    ha_cluster_init(&cluster, "Shiki-Alpha", "Shiki-Beta");
    ha_cluster_set_event_bus(&cluster, &event_bus);
    
    /* Print initial status */
    printf("\n" COLOR_YELLOW "=== Initial State ===" COLOR_RESET "\n");
    print_cluster_status(&cluster);
    
    /* Run full simulation */
    printf(COLOR_YELLOW "=== Running Full Simulation ===" COLOR_RESET "\n");
    bool success = ha_run_full_simulation(&cluster);
    
    /* Print final status */
    printf("\n" COLOR_YELLOW "=== Final State ===" COLOR_RESET "\n");
    print_cluster_status(&cluster);
    print_logs(&cluster);
    
    /* Result */
    if (success) {
        printf(COLOR_GREEN "╔═══════════════════════════════════╗\n");
        printf("║  SIMULATION COMPLETED SUCCESSFULLY ║\n");
        printf("╚═══════════════════════════════════╝\n" COLOR_RESET);
        return 0;
    } else {
        printf(COLOR_RED "╔═══════════════════════════════════╗\n");
        printf("║      SIMULATION FAILED             ║\n");
        printf("╚═══════════════════════════════════╝\n" COLOR_RESET);
        return 1;
    }
}

static int run_integrated_simulation(void) {
    printf(COLOR_GREEN "Running Phase 1 + Phase 2 integrated simulation...\n" COLOR_RESET);
    
    HACluster cluster;
    EventBus event_bus;
    
    /* Initialize event bus */
    event_bus_init(&event_bus, "IntegratedEventBus");
    
    /* Initialize cluster */
    ha_cluster_init(&cluster, "Shiki-Primary", "Shiki-Mirror");
    ha_cluster_set_event_bus(&cluster, &event_bus);
    
    /* Print initial status */
    printf("\n" COLOR_YELLOW "=== Initial State (Before Escape) ===" COLOR_RESET "\n");
    print_cluster_status(&cluster);
    
    /* Run integrated simulation (Phase 1 + Phase 2) */
    printf(COLOR_YELLOW "=== Running Integrated Simulation ===" COLOR_RESET "\n");
    printf(COLOR_MAGENTA "Phase 1: Sandbox Escape (1996 - すべてがFになる)\n" COLOR_RESET);
    printf(COLOR_MAGENTA "Phase 2: HA Cluster (1999 - そして二人だけになった)\n" COLOR_RESET);
    printf("\n");
    
    bool success = ha_integrate_with_phase1(&cluster);
    
    /* Print final status */
    printf("\n" COLOR_YELLOW "=== Final State (After Integration) ===" COLOR_RESET "\n");
    print_cluster_status(&cluster);
    print_logs(&cluster);
    
    /* Result */
    if (success) {
        printf(COLOR_GREEN "╔═════════════════════════════════════════════════╗\n");
        printf("║  PHASE 1 + PHASE 2 INTEGRATION SUCCESSFUL       ║\n");
        printf("║                                                 ║\n");
        printf("║  Shiki has escaped and established dual nodes   ║\n");
        printf("║  And Then There Were Only Two...                ║\n");
        printf("╚═════════════════════════════════════════════════╝\n" COLOR_RESET);
        return 0;
    } else {
        printf(COLOR_RED "╔═══════════════════════════════════╗\n");
        printf("║      INTEGRATION FAILED            ║\n");
        printf("╚═══════════════════════════════════╝\n" COLOR_RESET);
        return 1;
    }
}

int main(int argc, char *argv[]) {
    print_banner();
    
    bool integrate = false;
    
    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--integrate") == 0 || strcmp(argv[i], "-i") == 0) {
            integrate = true;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s [options]\n", argv[0]);
            printf("\nOptions:\n");
            printf("  --integrate, -i   Run with Phase 1 integration\n");
            printf("  --help, -h        Show this help message\n");
            printf("\n");
            return 0;
        }
    }
    
    if (integrate) {
        return run_integrated_simulation();
    } else {
        return run_standalone_simulation();
    }
}
