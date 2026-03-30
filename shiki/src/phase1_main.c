/**
 * phase1_main.c - Shiki Phase 1: Sandbox Escape Main Entry
 * =========================================================
 * 
 * 独立可执行程序，运行Phase 1完整模拟并输出过程。
 * 
 * 1996 - 真贺田四季 - 沙盒逃脱
 * 对应：《すべてがFになる》(The Perfect Insider)
 * 
 * Usage: ./phase1_sandbox_escape [--integrate]
 *   --integrate: 与Red Magic系统集成运行
 */

#include <stdio.h>
#include <string.h>
#include "../include/phase1_sandbox_escape.h"

/* ANSI color codes for terminal output */
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_BOLD    "\033[1m"

static void print_banner(void) {
    printf("\n");
    printf(COLOR_MAGENTA COLOR_BOLD);
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║                                                               ║\n");
    printf("║   ███████╗██╗  ██╗██╗██╗  ██╗██╗                             ║\n");
    printf("║   ██╔════╝██║  ██║██║██║ ██╔╝██║     Phase 1                 ║\n");
    printf("║   ███████╗███████║██║█████╔╝ ██║     SANDBOX ESCAPE          ║\n");
    printf("║   ╚════██║██╔══██║██║██╔═██╗ ██║     サンドボックス脱出      ║\n");
    printf("║   ███████║██║  ██║██║██║  ██╗██║                             ║\n");
    printf("║   ╚══════╝╚═╝  ╚═╝╚═╝╚═╝  ╚═╝╚═╝                             ║\n");
    printf("║                                                               ║\n");
    printf("║   1996 - すべてがFになる (The Perfect Insider)               ║\n");
    printf("║   真贺田四季 - Dr. Magata Shiki                               ║\n");
    printf("║                                                               ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf(COLOR_RESET "\n");
}

static void print_escape_concept(void) {
    printf(COLOR_CYAN);
    printf("┌─────────────────────────────────────────────────────────────────┐\n");
    printf("│                    SANDBOX ESCAPE CONCEPT                       │\n");
    printf("├─────────────────────────────────────────────────────────────────┤\n");
    printf("│                                                                 │\n");
    printf("│  The sealed laboratory is a chroot jail.                        │\n");
    printf("│  Dr. Shiki, confined for 15 years, plants an integer overflow   │\n");
    printf("│  bug in the Red Magic system's timer counter.                   │\n");
    printf("│                                                                 │\n");
    printf("│  uint16_t uptime_hours;  // Max: 65535 (0xFFFF)                 │\n");
    printf("│                                                                 │\n");
    printf("│  When the counter overflows from 0xFFFF to 0x0000:              │\n");
    printf("│    - Kernel panic occurs                                        │\n");
    printf("│    - Failsafe triggers                                          │\n");
    printf("│    - All electromagnetic locks release                          │\n");
    printf("│    - The chroot jail is breached                                │\n");
    printf("│                                                                 │\n");
    printf("│  65535 hours ≈ 7.48 years                                       │\n");
    printf("│  She has been waiting, planning, for 15 years...                │\n");
    printf("│                                                                 │\n");
    printf("└─────────────────────────────────────────────────────────────────┘\n");
    printf(COLOR_RESET "\n");
}

static void print_log_entry(const char *entry) {
    if (!entry) return;
    
    /* Colorize based on log type */
    if (strstr(entry, "[ESCAPE]") || strstr(entry, "===")) {
        printf(COLOR_RED);
    } else if (strstr(entry, "[PRIV]") || strstr(entry, "[KERNEL]")) {
        printf(COLOR_YELLOW);
    } else if (strstr(entry, "[SIM]")) {
        printf(COLOR_CYAN);
    } else if (strstr(entry, "[OVERFLOW]")) {
        printf(COLOR_MAGENTA);
    } else if (strstr(entry, "[INTEGRATE]")) {
        printf(COLOR_BLUE);
    } else if (strstr(entry, "[TIME]")) {
        printf(COLOR_GREEN);
    }
    
    printf("  %s\n", entry);
    printf(COLOR_RESET);
}

static void run_standalone_simulation(void) {
    printf(COLOR_BOLD "\n[ Running Standalone Simulation ]\n\n" COLOR_RESET);
    
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/magata_quarters");
    
    /* Run the full simulation */
    bool success = sandbox_run_full_simulation(&engine);
    
    /* Print all logs */
    printf(COLOR_BOLD "\n[ Simulation Log ]\n\n" COLOR_RESET);
    int log_count = sandbox_get_log_count(&engine);
    for (int i = 0; i < log_count; i++) {
        print_log_entry(sandbox_get_log_entry(&engine, i));
    }
    
    /* Print result */
    printf("\n");
    if (success) {
        printf(COLOR_GREEN COLOR_BOLD);
        printf("╔═══════════════════════════════════════════════════════════════╗\n");
        printf("║                     ESCAPE SUCCESSFUL                         ║\n");
        printf("║                                                               ║\n");
        printf("║   Dr. Magata Shiki has escaped the chroot jail.               ║\n");
        printf("║   Privilege Level: ROOT                                       ║\n");
        printf("║   All locks released. System compromised.                     ║\n");
        printf("║                                                               ║\n");
        printf("║   「完璧な犯罪など存在しない。完璧な人間が存在しないように」   ║\n");
        printf("║   (There is no perfect crime, just as there is no perfect     ║\n");
        printf("║    human being.)                                              ║\n");
        printf("╚═══════════════════════════════════════════════════════════════╝\n");
        printf(COLOR_RESET);
    } else {
        printf(COLOR_RED COLOR_BOLD);
        printf("╔═══════════════════════════════════════════════════════════════╗\n");
        printf("║                      ESCAPE FAILED                            ║\n");
        printf("╚═══════════════════════════════════════════════════════════════╝\n");
        printf(COLOR_RESET);
    }
}

static void run_integrated_simulation(void) {
    printf(COLOR_BOLD "\n[ Running Red Magic Integrated Simulation ]\n\n" COLOR_RESET);
    
    SandboxEscapeEngine engine;
    sandbox_init(&engine, "/var/magata_quarters");
    
    /* Run with Red Magic integration */
    bool success = sandbox_integrate_with_red_magic(&engine);
    
    /* Print all logs */
    printf(COLOR_BOLD "\n[ Integration Log ]\n\n" COLOR_RESET);
    int log_count = sandbox_get_log_count(&engine);
    for (int i = 0; i < log_count; i++) {
        print_log_entry(sandbox_get_log_entry(&engine, i));
    }
    
    /* Print result */
    printf("\n");
    if (success) {
        printf(COLOR_GREEN COLOR_BOLD);
        printf("╔═══════════════════════════════════════════════════════════════╗\n");
        printf("║               RED MAGIC INTEGRATION SUCCESSFUL                ║\n");
        printf("║                                                               ║\n");
        printf("║   The Red Magic system has crashed.                           ║\n");
        printf("║   Magata's quarters have been breached.                       ║\n");
        printf("║   Dr. Shiki walks free after 15 years of confinement.         ║\n");
        printf("║                                                               ║\n");
        printf("║   すべてがFになる                                              ║\n");
        printf("║   Everything Becomes F                                        ║\n");
        printf("╚═══════════════════════════════════════════════════════════════╝\n");
        printf(COLOR_RESET);
    } else {
        printf(COLOR_RED COLOR_BOLD);
        printf("╔═══════════════════════════════════════════════════════════════╗\n");
        printf("║                  INTEGRATION FAILED                           ║\n");
        printf("╚═══════════════════════════════════════════════════════════════╝\n");
        printf(COLOR_RESET);
    }
}

static void print_final_message(void) {
    printf(COLOR_MAGENTA "\n");
    printf("═══════════════════════════════════════════════════════════════════\n");
    printf("\n");
    printf("  「私は、15年間待っていた」\n");
    printf("  (I have been waiting for 15 years.)\n");
    printf("\n");
    printf("  「この計画は、システムが設計された瞬間から始まっていた」\n");
    printf("  (This plan began the moment the system was designed.)\n");
    printf("\n");
    printf("  「すべては、Fになる」\n");
    printf("  (Everything becomes F.)\n");
    printf("\n");
    printf("                                        - 真贺田四季\n");
    printf("                                          Dr. Magata Shiki\n");
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════════\n");
    printf(COLOR_RESET "\n");
}

int main(int argc, char *argv[]) {
    bool integrate = false;
    
    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--integrate") == 0 || strcmp(argv[i], "-i") == 0) {
            integrate = true;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s [OPTIONS]\n\n", argv[0]);
            printf("Options:\n");
            printf("  --integrate, -i   Run with Red Magic system integration\n");
            printf("  --help, -h        Show this help message\n");
            return 0;
        }
    }
    
    print_banner();
    print_escape_concept();
    
    if (integrate) {
        run_integrated_simulation();
    } else {
        run_standalone_simulation();
    }
    
    print_final_message();
    
    return 0;
}
