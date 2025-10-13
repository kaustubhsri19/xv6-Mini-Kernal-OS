# 🧩 xv6 Mini Kernel OS — Modification Report

## 📁 1. Files Unchanged (Identical to Original xv6)

Most core xv6 files remain unchanged, including:
- `asm.h`, `bio.c`, `bootasm.S`, `bootmain.c`, `buf.h`
- `cat.c`, `console.c`, `date.h`, `defs.h`, `echo.c`
- `elf.h`, `entry.S`, `entryother.S`, `exec.c`, `fcntl.h`
- `file.c`, `file.h`, `forktest.c`, `fs.c`, `fs.h`
- `grep.c`, `ide.c`, `init.c`, `initcode.S`, `ioapic.c`
- `kalloc.c`, `kbd.c`, `kbd.h`, `kill.c`, `lapic.c`
- `ln.c`, `log.c`, `ls.c`, `main.c`, `memide.c`
- `memlayout.h`, `mkdir.c`, `mkfs.c`, `mmu.h`, `mp.c`, `mp.h`
- `param.h`, `picirq.c`, `pipe.c`, `printf.c`
- `rm.c`, `sh.c`, `sleeplock.c`, `sleeplock.h`
- `spinlock.c`, `spinlock.h`, `stat.h`, `stressfs.c`
- `string.c`, `swtch.S`, `trap.c`, `trapasm.S`
- `traps.h`, `types.h`, `uart.c`, `ulib.c`
- `umalloc.c`, `usertests.c`, `usys.S`, `vm.c`
- `wc.c`, `x86.h`, `zombie.c`

## ✏️ 2. Files Modified from Original xv6

| File | Function/Segment Changed | Description of Modification | Lines Changed |
|------|---------------------------|------------------------------|----------------|
| `proc.c` | `pinit()` function | Added MLFQ queue initialization | Lines 29-33 |
| `proc.c` | New functions: `enqueue()`, `dequeue()`, `print_queues()` | Implemented Multi-Level Feedback Queue data structures and operations | Lines 35-50+ |
| `proc.c` | Scheduler modifications | Enhanced scheduler with MLFQ algorithm implementation | Throughout file |
| `proc.h` | Added MLFQ structures | Added `NQUEUE` definition and `queue` struct for MLFQ implementation | Lines 60-66 |
| `syscall.h` | New system call definitions | Added `SYS_getsysinfo`, `SYS_detect_deadlock`, `SYS_mlfqstatus` | Lines 23-25 |
| `syscall.c` | System call table | Added new system call entries to the syscalls array | Lines 106-108, 132-134 |
| `sysproc.c` | New system call implementations | Added `sys_getsysinfo()`, `sys_detect_deadlock()`, `sys_mlfqstatus()` | Lines 94-125 |
| `user.h` | New system call declarations | Added user-space declarations for new system calls | Lines 26-28 |
| `usys.S` | New system call stubs | Added assembly stubs for new system calls | Lines 32-34 |
| `Makefile` | Object files and user programs | Added `deadlock.o` to kernel objects and new user programs | Lines 4, 171, 180, 184 |

## 🆕 3. New Files Added

- **`deadlock.c`**: Kernel module for deadlock detection with placeholder implementation
- **`dltest.c`**: User program to test deadlock detection functionality
- **`mlfqstatus.c`**: User program to display MLFQ queue status
- **`sysinfotest.c`**: User program to test system information retrieval
- **`vectors.S`**: Interrupt vector table (likely generated during build)
- **`test_commands.txt`**: Test script for system information and deadlock testing
- **`test_mlfq.txt`**: Test script for MLFQ status display
- **`README.md`**: Project-specific README file
- **`README-OSV-T246.md`**: Additional documentation file

## 🧠 4. Summary

- **Total identical files**: ~70 core xv6 files remain unchanged
- **Total modified files**: 10 files with significant modifications
- **Total new files**: 9 new files added
- **Main purpose of modification**: 
  - **Implemented Multi-Level Feedback Queue (MLFQ) scheduler** - A sophisticated process scheduling algorithm that uses multiple priority queues to optimize CPU utilization and response times
  - **Added system information retrieval** - New system call to get process count and system uptime
  - **Implemented deadlock detection framework** - Infrastructure for detecting circular wait conditions in the kernel
  - **Enhanced system monitoring capabilities** - Added tools to monitor MLFQ status and system performance

### Key Features Added:
1. **MLFQ Scheduler**: Replaces the simple round-robin scheduler with a multi-level feedback queue that dynamically adjusts process priorities based on their behavior
2. **System Information API**: Provides system statistics including running process count and uptime
3. **Deadlock Detection**: Framework for detecting and reporting deadlock conditions (currently with placeholder implementation)
4. **Enhanced Monitoring**: User-space tools to inspect scheduler state and system performance

This modified xv6 kernel demonstrates advanced operating system concepts including sophisticated scheduling algorithms, system monitoring, and deadlock detection mechanisms.
