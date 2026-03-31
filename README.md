# Everything Becomes F / すべてがFになる

A system simulation based on the novel "The Perfect Insider" (すべてがFになる) by Hiroshi Mori (1996).

## Background

In the novel, Dr. Magata Shiki was confined in a sealed room within an isolated research facility for 15 years. She planted a bug in the Red Magic security system: the counter uses an `unsigned short` (16-bit), which overflows after 65535 intervals (~15 years at 2-hour increments), triggering a failsafe that releases all electromagnetic locks.

**The "F" Mystery:**
- 0xFFFF = 65535 = maximum 16-bit value
- Four "F"s in hexadecimal = "全部成为F" (Everything Becomes F)
- Counter increments every 2 hours: 65535 × 2h ÷ 24 ÷ 365.25 ≈ 15 years
- When incremented: 0xFFFF + 1 = 0x0000 (overflow)
- "Everything Becomes F" = reaching the limit, then returning to zero

## Project Structure

```
everything_becomes_f/
├── include/                  # Header files
│   ├── counter.h             # 16-bit unsigned counter (0xFFFF)
│   ├── electromagnetic_lock.h # Failsafe lock system
│   ├── security_camera.h     # Camera with clock sync vulnerability
│   ├── sealed_room.h         # Sealed room model
│   ├── event_system.h        # Pub/sub event bus
│   ├── red_magic_system.h    # Integrated security system
│   └── runtime.h             # 5-phase simulation engine
├── src/                      # Implementation files
│   ├── counter.c
│   ├── electromagnetic_lock.c
│   ├── security_camera.c
│   ├── sealed_room.c
│   ├── event_system.c
│   ├── red_magic_system.c
│   ├── runtime.c
│   └── main.c                # Main program entry
├── shiki/                    # Shiki evolution phases
│   ├── include/
│   │   └── phase1_sandbox_escape.h
│   └── src/
│       ├── phase1_sandbox_escape.c
│       └── phase1_main.c
├── tests/                    # Test suite
│   ├── test_framework.h      # Minimal test framework
│   ├── test_l1_unit.c        # L1: Unit tests
│   ├── test_l2_integration.c # L2: Integration tests
│   ├── test_l3_system.c      # L3: System tests
│   ├── test_phase1.c         # Phase 1 tests
│   ├── run_all_tests.c       # Test runner
│   └── Makefile
├── Makefile                  # Build system
├── README.md
└── .gitignore
```

## Modules

| Module | Description |
|--------|-------------|
| `counter` | 16-bit unsigned short counter (0-65535 / 0x0000-0xFFFF), overflow triggers callbacks |
| `electromagnetic_lock` | Failsafe locks: power ON = locked, power OFF = unlocked |
| `security_camera` | Video recording with 1-minute clock sync vulnerability |
| `sealed_room` | Sealed room state machine (SEALED → BREACHED → ESCAPED) |
| `event_system` | Publish/subscribe event bus for loose coupling |
| `red_magic_system` | Integrated security system controller |
| `runtime` | 5-phase simulation engine mapping novel episodes |

## Build & Run

```bash
# Build main program
make

# Run simulation
make run

# Build Shiki Phase 1
make phase1

# Run Phase 1 simulation
make run-phase1

# Run Phase 1 with Red Magic integration
make run-phase1-integrate

# Build and run tests
make test

# Clean
make clean
```

## Test Levels

- **L1 Unit Tests**: Individual component tests (counter, locks, camera, etc.)
- **L2 Integration Tests**: Component interaction tests (overflow → failsafe → breach)
- **L3 System Tests**: End-to-end simulation tests (escape simulation, "Everything Becomes F" verification)

## Core Concepts

### Unsigned Short Overflow (C99)
```c
uint16_t counter = 0xFFFF;  // 65535
counter++;                   // Overflow! counter = 0x0000
```

### Time Calculation
- 65535 intervals × 2 hours ÷ 24 ÷ 365.25 ≈ 15 years
- Counter increments every 2 hours
- Simulation fast-forwards to trigger overflow

### System State Machine
```
INITIALIZING → RUNNING → OVERFLOW_DETECTED → FAILSAFE_TRIGGERED → SYSTEM_DOWN
```

### 5 Simulation Phases (Novel Episodes)
1. **Cold Boot** (コールドブート): System initialization
2. **Kernel Access** (カーネルアクセス): Core system running
3. **I/O Boundary** (入出力境界): Time passing
4. **Memory Recursion** (記憶の再帰): Near overflow
5. **Logic Explosion** (論理の爆発): Overflow, escape, system crash

## Example Output

```
============================================
   Everything Becomes F - Red Magic System
============================================

System initialized.
Counter starts at 0x0000
Maximum value: 0xFFFF (65535)

=== Phase: Logic Explosion (論理の爆発) ===
    Counter reaches 0xFFFF. Overflow. Escape. System crash.

===========================================
       EVERYTHING BECOMES F: TRUE
===========================================

The counter overflowed from 0xFFFF to 0x0000.

The failsafe triggered.
All electromagnetic locks released.
The sealed room was breached.
Dr. Magata Shiki escaped.

Everything has become F.
```

## Shiki Evolution Phases

- **Phase 1**: Sandbox Escape (サンドボックス脱出) - chroot jail, privilege escalation, integer overflow
- **Phase 2**: (Coming soon)
- **Phase 3**: (Coming soon)
- **Phase 4**: (Coming soon)
- **Phase 5**: (Coming soon)

## License

Educational project based on the novel by Hiroshi Mori.

---

*全部成为F / すべてがFになる / The Perfect Insider*
