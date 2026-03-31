/**
 * phase2_ha_cluster.h - Shiki Phase 2: High Availability Cluster
 * ================================================================
 *
 * ===== 1999: 真贺田四季 - 高可用集群 =====
 *
 * 対応：《そして二人だけになった》(And Then There Were Only Two)
 *
 * 核心概念：
 * - 双機熱備 (Active-Standby): 主節点運行、備節点待命
 * - 心跳検測 (Heartbeat): 定期検測対方存活
 * - 故障転移 (Failover): 主節点故障時備節点接管
 * - RAID-1鏡像: 数据実時同歩到両個節点
 * - 脳裂 (Split Brain): 両節点都認為自己是主節点的危険状態
 *
 * 真贺田四季从沙盒逃脱后，建立了双机热备系统——隐喻她的双重身份/人格。
 * 两个节点互为镜像，一个倒下另一个接管。
 */

#ifndef PHASE2_HA_CLUSTER_H
#define PHASE2_HA_CLUSTER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Forward declaration - use struct tag instead of typedef to avoid redefinition */
struct EventBus;

/* ============================================================================
 * Constants
 * ============================================================================ */

#define HA_MAX_NAME_LEN         64
#define HA_DATA_STORE_SIZE      1024
#define HA_MAX_LOG_ENTRIES      64
#define HA_MAX_LOG_LEN          128
#define HA_DEFAULT_HEARTBEAT_INTERVAL  100   /* ms */
#define HA_DEFAULT_HEARTBEAT_TIMEOUT   500   /* ms */
#define HA_DEFAULT_MAX_MISSED_HEARTBEATS 3
#define HA_NODE_COUNT           2

/* ============================================================================
 * Node Role - 節点角色
 * ============================================================================ */

typedef enum {
    NODE_ACTIVE,      /* 主節点 - 処理請求 */
    NODE_STANDBY,     /* 備節点 - 等待接管 */
    NODE_FAILED,      /* 故障節点 */
    NODE_ISOLATED     /* 隔離節点 (脳裂) */
} NodeRole;

/* Get node role name as string */
const char *node_role_name(NodeRole role);

/* ============================================================================
 * Node Health - 節点健康状態
 * ============================================================================ */

typedef enum {
    HEALTH_HEALTHY,   /* 正常 */
    HEALTH_DEGRADED,  /* 降級 */
    HEALTH_CRITICAL,  /* 危機 */
    HEALTH_DEAD       /* 死亡 */
} NodeHealth;

/* Get node health name as string */
const char *node_health_name(NodeHealth health);

/* ============================================================================
 * HeartbeatMessage - 心跳消息
 * ============================================================================ */

typedef struct {
    uint32_t sequence;           /* 序列号 */
    uint32_t timestamp;          /* 時間戳 */
    NodeRole sender_role;        /* 発送者角色 */
    NodeHealth sender_health;    /* 発送者健康状態 */
    uint32_t data_version;       /* 数据版本号，用于同歩 */
} HeartbeatMessage;

/* ============================================================================
 * ClusterNode - 集群節点
 * ============================================================================ */

typedef struct {
    char name[HA_MAX_NAME_LEN];
    NodeRole role;
    NodeHealth health;
    uint32_t data_version;
    uint32_t last_heartbeat_sent;
    uint32_t last_heartbeat_received;
    uint32_t heartbeat_interval;    /* 心跳間隔(ms) */
    uint32_t heartbeat_timeout;     /* 心跳超時(ms) */
    int missed_heartbeats;
    int max_missed_heartbeats;      /* 最大允許丟失心跳数 */
    
    /* RAID-1 数据鏡像 */
    uint8_t data_store[HA_DATA_STORE_SIZE];  /* 模擬数据存儲 */
    size_t data_size;
    bool data_dirty;                /* 数据是否需要同歩 */
    
    /* Phase 1 集成 */
    bool sandbox_escaped;           /* 是否已完成沙盒逃脱 */
} ClusterNode;

/* ============================================================================
 * HACluster - 高可用集群
 * ============================================================================ */

typedef struct {
    ClusterNode nodes[HA_NODE_COUNT];  /* 双節点 */
    int active_node_index;             /* 当前主節点索引 */
    uint32_t cluster_epoch;            /* 集群紀元(切換次数) */
    uint32_t current_time;             /* 模擬時鐘 */
    bool split_brain_detected;
    int total_failovers;
    struct EventBus *event_bus;
    
    /* 運行状態 */
    bool is_running;
    char cluster_log[HA_MAX_LOG_ENTRIES][HA_MAX_LOG_LEN];
    int log_count;
} HACluster;

/* ============================================================================
 * Cluster Initialization and Management
 * ============================================================================ */

/**
 * ha_cluster_init - 初始化HA集群
 * @cluster: 集群指針
 * @node0_name: 節点0名称
 * @node1_name: 節点1名称
 *
 * 初始化双節点，node[0]=ACTIVE, node[1]=STANDBY
 */
void ha_cluster_init(HACluster *cluster, const char *node0_name, const char *node1_name);

/**
 * ha_cluster_set_event_bus - 設置事件総線
 */
void ha_cluster_set_event_bus(HACluster *cluster, struct EventBus *bus);

/**
 * ha_cluster_start - 啓動集群
 */
bool ha_cluster_start(HACluster *cluster);

/**
 * ha_cluster_stop - 停止集群
 */
bool ha_cluster_stop(HACluster *cluster);

/* ============================================================================
 * Heartbeat Protocol - 心跳協議
 * ============================================================================ */

/**
 * ha_create_heartbeat - 創建心跳消息
 */
void ha_create_heartbeat(const HACluster *cluster, int from_node, HeartbeatMessage *msg);

/**
 * ha_send_heartbeat - 発送心跳
 * @cluster: 集群
 * @from_node: 発送節点索引
 * @return: true if sent successfully
 */
bool ha_send_heartbeat(HACluster *cluster, int from_node);

/**
 * ha_receive_heartbeat - 接收心跳
 * @cluster: 集群
 * @to_node: 接收節点索引
 * @msg: 心跳消息
 * @return: true if received successfully
 */
bool ha_receive_heartbeat(HACluster *cluster, int to_node, const HeartbeatMessage *msg);

/**
 * ha_check_heartbeat_timeout - 検査心跳超時
 * @cluster: 集群
 * @node_index: 節点索引
 * @return: true if timeout detected
 */
bool ha_check_heartbeat_timeout(HACluster *cluster, int node_index);

/* ============================================================================
 * Failover - 故障転移
 * ============================================================================ */

/**
 * ha_trigger_failover - 触発故障転移
 * @cluster: 集群
 * @return: true if failover successful
 *
 * 備節点接管為主節点
 */
bool ha_trigger_failover(HACluster *cluster);

/**
 * ha_simulate_node_failure - 模擬節点故障
 * @cluster: 集群
 * @node_index: 故障節点索引
 * @return: true if simulation successful
 */
bool ha_simulate_node_failure(HACluster *cluster, int node_index);

/**
 * ha_recover_node - 恢復節点
 * @cluster: 集群
 * @node_index: 恢復節点索引
 * @return: true if recovery successful
 */
bool ha_recover_node(HACluster *cluster, int node_index);

/* ============================================================================
 * RAID-1 Data Operations - RAID-1数据操作
 * ============================================================================ */

/**
 * ha_write_data - 写入数据到主節点
 * @cluster: 集群
 * @data: 数据
 * @size: 数据大小
 * @return: true if write successful
 */
bool ha_write_data(HACluster *cluster, const uint8_t *data, size_t size);

/**
 * ha_read_data - 从主節点読取数据
 * @cluster: 集群
 * @buffer: 緩衝区
 * @max_size: 最大読取大小
 * @return: 実際読取大小
 */
size_t ha_read_data(const HACluster *cluster, uint8_t *buffer, size_t max_size);

/**
 * ha_sync_data - 主→備同歩数据
 * @cluster: 集群
 * @return: true if sync successful
 */
bool ha_sync_data(HACluster *cluster);

/**
 * ha_verify_mirror - 験証鏡像一致性
 * @cluster: 集群
 * @return: true if mirrors are consistent
 */
bool ha_verify_mirror(const HACluster *cluster);

/* ============================================================================
 * Split Brain Detection and Resolution - 脳裂検測和処理
 * ============================================================================ */

/**
 * ha_detect_split_brain - 検測脳裂
 * @cluster: 集群
 * @return: true if split brain detected
 *
 * 検測両個節点都認為自己是ACTIVE的状態
 */
bool ha_detect_split_brain(const HACluster *cluster);

/**
 * ha_resolve_split_brain - 解決脳裂
 * @cluster: 集群
 * @return: true if resolution successful
 */
bool ha_resolve_split_brain(HACluster *cluster);

/* ============================================================================
 * State Queries - 状態査詢
 * ============================================================================ */

/**
 * ha_get_node_role - 獲取節点角色
 */
NodeRole ha_get_node_role(const HACluster *cluster, int node_index);

/**
 * ha_get_node_health - 獲取節点健康状態
 */
NodeHealth ha_get_node_health(const HACluster *cluster, int node_index);

/**
 * ha_get_active_node - 獲取当前主節点索引
 */
int ha_get_active_node(const HACluster *cluster);

/**
 * ha_get_failover_count - 獲取故障転移次数
 */
uint32_t ha_get_failover_count(const HACluster *cluster);

/**
 * ha_is_cluster_healthy - 検査集群是否健康
 */
bool ha_is_cluster_healthy(const HACluster *cluster);

/**
 * ha_is_cluster_running - 検査集群是否運行中
 */
bool ha_is_cluster_running(const HACluster *cluster);

/**
 * ha_get_data_version - 獲取数据版本
 */
uint32_t ha_get_data_version(const HACluster *cluster, int node_index);

/* ============================================================================
 * Time Simulation - 時間模擬
 * ============================================================================ */

/**
 * ha_advance_time - 推進模擬時間
 * @cluster: 集群
 * @ms: 要推進的毫秒数
 */
void ha_advance_time(HACluster *cluster, uint32_t ms);

/**
 * ha_get_current_time - 獲取当前模擬時間
 */
uint32_t ha_get_current_time(const HACluster *cluster);

/* ============================================================================
 * Full Simulation - 完整模擬
 * ============================================================================ */

/**
 * ha_run_full_simulation - 運行完整模擬
 * @cluster: 集群
 * @return: true if simulation completed successfully
 *
 * 完整模擬：正常運行 → 主節点故障 → 切換 → 恢復
 */
bool ha_run_full_simulation(HACluster *cluster);

/* ============================================================================
 * Phase 1 Integration - Phase 1集成
 * ============================================================================ */

/**
 * ha_integrate_with_phase1 - 与Phase 1集成
 * @cluster: 集群
 * @return: true if integration successful
 *
 * 調用Phase 1的沙盒逃脱，在逃脱后建立HA集群
 */
bool ha_integrate_with_phase1(HACluster *cluster);

/* ============================================================================
 * Logging - 日志
 * ============================================================================ */

/**
 * ha_log - 添加日志条目
 */
void ha_log(HACluster *cluster, const char *message);

/**
 * ha_get_log_count - 獲取日志条数
 */
int ha_get_log_count(const HACluster *cluster);

/**
 * ha_get_log_entry - 獲取日志条目
 */
const char* ha_get_log_entry(const HACluster *cluster, int index);

/**
 * ha_clear_logs - 清除日志
 */
void ha_clear_logs(HACluster *cluster);

#endif /* PHASE2_HA_CLUSTER_H */
