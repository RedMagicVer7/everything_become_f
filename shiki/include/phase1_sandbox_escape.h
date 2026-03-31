/**
 * phase1_sandbox_escape.h - Shiki Phase 1: Sandbox Escape
 * =========================================================
 * 
 * ===== 1996: 真贺田四季 - 沙盒逃脱 =====
 * 
 * 对应：《すべてがFになる》(The Perfect Insider)
 * 
 * 核心概念：
 * - chroot jail: Unix进程隔离（密室的隐喻）
 * - 权限提升: 从普通用户到root（从囚犯到自由）
 * - 整型溢出: unsigned short 0xFFFF → 0x0000（时间炸弹）
 * - Kernel panic: 系统崩溃释放所有锁（failsafe）
 * 
 * 真贺田四季被关在密封实验室15年，通过植入整型溢出bug实现逃脱。
 * 这是从物理禁锢中逃脱的阶段——Unix沙盒逃逸的隐喻。
 * 
 * chroot jail逃脱技术参考：
 * - 双重chroot攻击：在root权限下执行第二次chroot可逃脱
 * - 符号链接攻击：通过符号链接访问jail外的文件
 * - 文件描述符泄漏：保持对jail外文件的句柄
 * - 整型溢出：通过溢出绕过安全检查
 */

#ifndef PHASE1_SANDBOX_ESCAPE_H
#define PHASE1_SANDBOX_ESCAPE_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * Constants
 * ============================================================================ */

#define SANDBOX_MAX_LOG_ENTRIES 32
#define SANDBOX_MAX_LOG_LEN     128
#define SANDBOX_PATH_LEN        256

/* 15 years in hours = 15 * 365.25 * 24 ≈ 131,490 hours */
#define FIFTEEN_YEARS_HOURS     131490

/* ============================================================================
 * Privilege Levels - 进程权限级别
 * ============================================================================ */

typedef enum {
    PRIV_USER   = 0,    /* 普通用户 - 被隔离的四季 */
    PRIV_DAEMON = 1,    /* 守护进程 - Red Magic系统 */
    PRIV_KERNEL = 2,    /* 内核态 - 操作系统内核 */
    PRIV_ROOT   = 3     /* root权限 - 完全自由 */
} PrivilegeLevel;

/* Get privilege level name */
const char *privilege_level_name(PrivilegeLevel level);

/* ============================================================================
 * ChrootJail - 隔离沙盒
 * ============================================================================ */

typedef struct {
    char root_path[SANDBOX_PATH_LEN];     /* jail的根路径 */
    char real_root[SANDBOX_PATH_LEN];     /* 真实的系统根 */
    bool is_confined;                      /* 是否被禁锢 */
    PrivilegeLevel current_priv;           /* 当前权限 */
    uint16_t uptime_intervals;             /* 系统运行时间(2小时为单位) */
    uint16_t max_uptime;                   /* 最大运行时间(0xFFFF, ~15年) */
    int escape_attempts;                   /* 逃脱尝试次数 */
    bool overflow_triggered;               /* 溢出是否已触发 */
} ChrootJail;

/* ============================================================================
 * SandboxEscapeEngine - 沙盒逃脱引擎
 * ============================================================================ */

typedef struct {
    ChrootJail jail;
    bool kernel_panicked;       /* 内核是否崩溃 */
    bool locks_released;        /* 锁是否已释放 */
    bool escaped;               /* 是否已逃脱 */
    
    /* 逃脱路径记录 */
    char escape_log[SANDBOX_MAX_LOG_ENTRIES][SANDBOX_MAX_LOG_LEN];
    int log_count;
} SandboxEscapeEngine;

/* ============================================================================
 * Initialization
 * ============================================================================ */

/**
 * sandbox_init - 初始化沙盒逃脱引擎
 * @engine: 引擎指针
 * @jail_path: chroot jail路径 (e.g., "/var/magata_quarters")
 * 
 * 设定15年隔离的初始状态
 */
void sandbox_init(SandboxEscapeEngine *engine, const char *jail_path);

/* ============================================================================
 * State Queries - 状态查询
 * ============================================================================ */

/**
 * sandbox_is_confined - 检查是否仍被禁锢
 */
bool sandbox_is_confined(const SandboxEscapeEngine *engine);

/**
 * sandbox_get_privilege - 获取当前权限级别
 */
PrivilegeLevel sandbox_get_privilege(const SandboxEscapeEngine *engine);

/**
 * sandbox_has_overflowed - 检查溢出是否已触发
 */
bool sandbox_has_overflowed(const SandboxEscapeEngine *engine);

/**
 * sandbox_has_escaped - 检查是否已成功逃脱
 */
bool sandbox_has_escaped(const SandboxEscapeEngine *engine);

/**
 * sandbox_intervals_remaining - 获取距溢出剩余时间间隔数(2小时为单位)
 */
uint16_t sandbox_intervals_remaining(const SandboxEscapeEngine *engine);

/**
 * sandbox_years_remaining - 获取距溢出剩余年数
 */
double sandbox_years_remaining(const SandboxEscapeEngine *engine);

/**
 * sandbox_get_uptime - 获取当前系统运行时间(2小时间隔数)
 */
uint16_t sandbox_get_uptime(const SandboxEscapeEngine *engine);

/**
 * sandbox_get_escape_attempts - 获取逃脱尝试次数
 */
int sandbox_get_escape_attempts(const SandboxEscapeEngine *engine);

/* ============================================================================
 * Escape Vectors - 逃脱向量
 * ============================================================================ */

/**
 * sandbox_attempt_direct_escape - 直接尝试逃出chroot
 * @return: false (直接突破不可能，四季明白这点)
 * 
 * 模拟直接尝试突破chroot jail的失败
 */
bool sandbox_attempt_direct_escape(SandboxEscapeEngine *engine);

/**
 * sandbox_attempt_symlink_escape - 符号链接攻击
 * @return: false (air-gapped系统，符号链接无法指向外部)
 * 
 * 模拟通过符号链接逃逸的失败
 */
bool sandbox_attempt_symlink_escape(SandboxEscapeEngine *engine);

/**
 * sandbox_advance_time - 推进系统时间
 * @intervals: 要推进的时间间隔数(每个间隔=2小时)
 * @return: true if overflow occurred
 * 
 * 模拟时间流逝，uint16_t递增
 */
bool sandbox_advance_time(SandboxEscapeEngine *engine, uint16_t intervals);

/**
 * sandbox_trigger_overflow - 触发整型溢出
 * @return: true if overflow triggered successfully
 * 
 * 当uptime_intervals达到0xFFFF时溢出归零
 */
bool sandbox_trigger_overflow(SandboxEscapeEngine *engine);

/**
 * sandbox_escalate_privilege - 权限提升
 * @return: true if privilege escalated to ROOT
 * 
 * 溢出后利用kernel panic提升到root
 */
bool sandbox_escalate_privilege(SandboxEscapeEngine *engine);

/**
 * sandbox_execute_escape - 执行完整逃脱序列
 * @return: true if escape successful
 * 
 * 完整序列: overflow → kernel panic → locks released → escape
 */
bool sandbox_execute_escape(SandboxEscapeEngine *engine);

/* ============================================================================
 * Full Simulation - 完整模拟
 * ============================================================================ */

/**
 * sandbox_run_full_simulation - 从禁锢到逃脱的完整模拟
 * @return: true if simulation completed successfully with escape
 * 
 * 端到端模拟: 初始化 → 15年等待 → 溢出 → 逃脱
 */
bool sandbox_run_full_simulation(SandboxEscapeEngine *engine);

/* ============================================================================
 * Logging - 日志
 * ============================================================================ */

/**
 * sandbox_get_log_count - 获取日志条目数
 */
int sandbox_get_log_count(const SandboxEscapeEngine *engine);

/**
 * sandbox_get_log_entry - 获取日志条目
 * @index: 日志索引
 * @return: 日志字符串或NULL
 */
const char* sandbox_get_log_entry(const SandboxEscapeEngine *engine, int index);

/**
 * sandbox_log - 添加日志条目
 * @message: 日志消息
 */
void sandbox_log(SandboxEscapeEngine *engine, const char *message);

/* ============================================================================
 * Red Magic Integration - 与everything_becomes_f集成
 * ============================================================================ */

/**
 * sandbox_integrate_with_red_magic - 与Red Magic系统集成
 * @return: true if integration successful
 * 
 * 调用 everything_becomes_f 的 RedMagicSystem 完成完整模拟
 */
bool sandbox_integrate_with_red_magic(SandboxEscapeEngine *engine);

#endif /* PHASE1_SANDBOX_ESCAPE_H */
