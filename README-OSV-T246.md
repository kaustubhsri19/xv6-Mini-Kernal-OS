# OSV-T246 Project Implementation

## Project Summary

This project implements 45% of the total scope for OSV-T246, including:

- **System Call**: Complete implementation of `getsysinfo()` system call
- **Partial MLFQ**: Data structures and queue logic for Multi-Level Feedback Queue scheduler
- **Deadlock Detection**: Design stubs and placeholder functionality

## Features Implemented

### 1. System Call: `getsysinfo()`

- **Purpose**: Returns system information including number of running processes and total system uptime
- **System Call Number**: 22
- **Parameters**: Pointer to `sysinfo` structure
- **Returns**: 0 on success, -1 on failure

#### Structure Definition
```c
struct sysinfo {
    int nproc;    // Number of running processes
    uint ticks;   // System uptime in ticks
};
```

### 2. Partial MLFQ Scheduler

- **Data Structures**: 
  - `struct queue` with circular buffer implementation
  - `mlfq[NQUEUE]` array for 3 priority levels
- **Queue Operations**: `enqueue()`, `dequeue()`, `print_queues()`
- **Current Status**: Data structures initialized, periodic queue state printing
- **TODO**: Complete scheduling algorithm integration

### 3. Deadlock Detection (Stub)

- **Data Structure**: `struct lockgraph` for tracking lock dependencies
- **Function**: `detect_deadlock()` - placeholder implementation
- **Current Status**: Design framework in place
- **TODO**: Implement DFS-based cycle detection algorithm

## How to Build and Run

### Prerequisites
- Linux environment (WSL recommended for Windows)
- GCC compiler
- QEMU emulator

### Build Instructions
```bash
make
```

### Run Instructions
```bash
make qemu
```

## Test Commands

### Test System Call
```bash
$ sysinfotest
```
Expected output: `Running processes: X, Uptime: Y ticks`

### Test Deadlock Detection
```bash
$ dltest
```
Expected output: `Running deadlock detection (stub)... Deadlock detected: NO`

### Test MLFQ (Visual)
- MLFQ queue states are printed every 500 ticks
- Look for "MLFQ State:" messages in the console

## Implementation Details

### Files Modified
- `syscall.h` - Added SYS_getsysinfo definition
- `syscall.c` - Added syscall table entry
- `sysproc.c` - Implemented sys_getsysinfo() function
- `usys.S` - Added SYSCALL(getsysinfo)
- `user.h` - Added function prototype
- `proc.h` - Added MLFQ queue structures
- `proc.c` - Added MLFQ helper functions and scheduler placeholder
- `defs.h` - Added deadlock detection declaration
- `Makefile` - Added new programs and kernel object

### Files Created
- `sysinfotest.c` - User program to test getsysinfo()
- `dltest.c` - User program to test deadlock detection
- `deadlock.c` - Deadlock detection implementation
- `README-OSV-T246.md` - This documentation

## Project Progress

- ✅ **System Call Implementation** (100% complete)
- ⚙️ **MLFQ Scheduler** (45% complete - data structures and basic framework)
- 🧩 **Deadlock Detection** (25% complete - design and stubs)

## Next Steps

1. **Complete MLFQ Scheduling**: Replace round-robin with MLFQ algorithm
2. **Implement Deadlock Detection**: Add DFS-based cycle detection
3. **Add Process Priority Management**: Implement priority boosting/demotion
4. **Add Lock Dependency Tracking**: Monitor lock acquisition patterns

## Notes

- Current implementation maintains system stability
- MLFQ structures are initialized but not actively used for scheduling
- Deadlock detection returns placeholder results
- All existing functionality remains intact

## Troubleshooting

### Common Issues
1. **Compilation Errors**: Ensure all files are properly saved
2. **QEMU Not Found**: Install QEMU or set QEMU path in Makefile
3. **Permission Issues**: Run with appropriate permissions

### Debug Information
- Use `make qemu-gdb` for debugging with GDB
- Check console output for MLFQ state messages
- Monitor system call execution with `sysinfotest`
