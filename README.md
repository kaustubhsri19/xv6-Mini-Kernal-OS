# xv6 Mini Kernel OS - Enhanced Edition

A modified version of MIT's xv6 operating system with advanced scheduling algorithms, system monitoring, and deadlock detection capabilities.

---

## 🚀 Features

### 1. **Multiple Scheduling Algorithms**
- **Round-Robin (RR)** - Default xv6 scheduler
- **Priority-Based Scheduling (PBS)** - Static priority scheduling
- **Multi-Level Feedback Queue (MLFQ)** - Dynamic priority with 5 levels

### 2. **System Monitoring Tools**
- **`top`** - Real-time process monitor with scheduler-aware display
- **`ps`** - Process status viewer
- **CPU statistics tracking** - Total ticks, per-process CPU usage

### 3. **Deadlock Detection**
- Wait-For Graph (WFG) construction
- Cycle detection algorithm
- Real-time deadlock reporting

### 4. **Enhanced System Calls**
- `setscheduler(policy)` - Switch between schedulers at runtime
- `getscheduler()` - Get current scheduler policy
- `getallprocinfo()` - Retrieve all process information
- `getcpustats()` - Get CPU statistics
- `getdeadlockinfo()` - Check for deadlocks

---

## 📋 Prerequisites

- **Linux environment** (WSL recommended for Windows)
- **GCC compiler** with 32-bit support
- **QEMU emulator** (qemu-system-i386)
- **Make** build tool

### Install Dependencies (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install build-essential qemu-system-x86 gcc-multilib
```

---

## 🔨 Build Instructions

### Clean Build
```bash
make clean
make
```

### Run xv6
```bash
make qemu
```

### Run with GDB Debugging
```bash
make qemu-gdb
# In another terminal:
gdb kernel
```

---

## 🎮 Usage Guide

### Switch Schedulers

```bash
# Round-Robin (Policy 0)
$ setsched 0

# Priority-Based (Policy 1)
$ setsched 1

# MLFQ (Policy 2)
$ setsched 2
```

### Monitor System

```bash
# Real-time process monitor (exits after 20 iterations)
$ top

# Process status
$ ps

# Check for deadlocks
$ deadlockinfo
```

### MLFQ Testing & Recording

#### **Method 1: Manual Recording**
```bash
$ setsched 2           # Switch to MLFQ scheduler
$ mlfqstart            # Start recording snapshots
$ mlfqtest1 &          # Launch CPU-intensive test (background)
$ mlfqtest2 &          # Launch another test
$ mlfqtest3 &          # Launch third test
$ sleep 300            # Wait 3 seconds for processes to run
$ mlfqstatus           # View recorded queue statistics
$ mlfqstop             # Stop recording
```

#### **Method 2: Automated Demo**
```bash
$ mlfqrecord           # Automatic: starts recording, runs tests, shows results
```

#### **Method 3: Visual Demonstration**
```bash
$ mlfqvisual           # Full visual demo with 5 worker processes
```

**Note**: MLFQ recording captures snapshots every 100 ticks (1 second). Tests now run longer to ensure visibility in snapshots.

---

## 📊 Scheduler Details

### Round-Robin (RR)
- **Time Slice**: Fixed quantum per process
- **Fairness**: Equal CPU time for all processes
- **Use Case**: General-purpose, simple workloads

### Priority-Based Scheduling (PBS)
- **Static Priority**: Each process has a priority (0-100)
- **Selection**: Highest priority process runs first
- **Use Case**: Real-time systems, priority-critical tasks

### Multi-Level Feedback Queue (MLFQ)
- **5 Priority Levels**: 0 (highest) to 4 (lowest)
- **Time Slices**: {1, 2, 4, 8, 16} ticks per level
- **Demotion**: CPU-intensive processes move to lower queues
- **Priority Boost**: Every 1000 ticks (10 seconds), all processes return to level 0
- **Use Case**: Interactive systems, mixed workloads

#### MLFQ Behavior
```
New Process → Level 0 (1 tick)
   ↓ (uses time slice)
Level 1 (2 ticks)
   ↓ (uses time slice)
Level 2 (4 ticks)
   ↓ (uses time slice)
Level 3 (8 ticks)
   ↓ (uses time slice)
Level 4 (16 ticks)
   ↓ (every 10 seconds)
Priority Boost → Level 0
```

---

## 🖥️ Top Command Output

The `top` command displays different columns based on the active scheduler:

### Round-Robin Mode
```
Scheduler: Round-Robin
PID  STATE      CPU_TICKS  UPTIME(s)  NAME
1    SLEEPING   6          1          init
2    SLEEPING   9          1          sh
```

### Priority-Based Mode
```
Scheduler: Priority-Based
PID  STATE      PRI  CPU_TICKS  UPTIME(s)  NAME
1    SLEEPING   60   6          1          init
2    SLEEPING   60   9          1          sh
```

### MLFQ Mode
```
Scheduler: MLFQ
PID  STATE      MLFQ  CPU_TICKS  UPTIME(s)  NAME
1    SLEEPING   0     6          1          init
2    SLEEPING   0     9          1          sh
3    RUNNING    2     45         0          top
```

---

## 🐛 Deadlock Detection

The system includes a deadlock detection mechanism that:
1. Builds a Wait-For Graph (WFG) from lock dependencies
2. Detects cycles using DFS algorithm
3. Reports deadlocked processes

```bash
$ deadlockinfo
No deadlock detected.

# Or if deadlock exists:
Deadlock detected!
Cycle: PID 5 -> PID 7 -> PID 5
```

---

## 🔌 System Calls Reference

This section provides in-depth documentation of all custom system calls implemented in this project.

### **Scheduler Management**

#### `int setscheduler(int policy)`
**Purpose**: Dynamically switch between scheduling algorithms at runtime.

**Parameters**:
- `policy`: Scheduler policy number
  - `0` = Round-Robin (RR)
  - `1` = Priority-Based Scheduling (PBS)
  - `2` = Multi-Level Feedback Queue (MLFQ)

**Returns**: 
- `0` on success
- `-1` on invalid policy number

**Implementation** (`sysproc.c`):
```c
int sys_setscheduler(void) {
  int policy;
  if(argint(0, &policy) < 0) return -1;
  if(policy < 0 || policy > 2) return -1;  // Validate range
  
  acquire(&policy_lock);
  current_scheduler_policy = policy;  // Global scheduler variable
  release(&policy_lock);
  return 0;
}
```

**Use Case**: 
- Performance testing: Compare scheduler behavior under different workloads
- Dynamic optimization: Switch to MLFQ for interactive tasks, PBS for real-time tasks
- Educational: Demonstrate scheduler differences in real-time

**Example**:
```c
// User program
setscheduler(2);  // Switch to MLFQ
printf("Now using MLFQ scheduler\n");
```

---

#### `int getscheduler(void)`
**Purpose**: Retrieve the currently active scheduler policy.

**Parameters**: None

**Returns**: Current scheduler policy number (0, 1, or 2)

**Implementation** (`sysproc.c`):
```c
int sys_getscheduler(void) {
  int policy;
  acquire(&policy_lock);
  policy = current_scheduler_policy;
  release(&policy_lock);
  return policy;
}
```

**Use Case**:
- Display current scheduler in monitoring tools (`top` command)
- Conditional logic based on active scheduler
- Verification after scheduler switch

**Example**:
```c
int policy = getscheduler();
const char *names[] = {"Round-Robin", "Priority-Based", "MLFQ"};
printf("Current scheduler: %s\n", names[policy]);
```

---

### **Process Information**

#### `int getallprocinfo(struct procinfo *info, int max)`
**Purpose**: Retrieve detailed information about all processes in the system.

**Parameters**:
- `info`: Pointer to array of `struct procinfo`
- `max`: Maximum number of processes to retrieve (typically `NPROC`)

**Returns**: Number of processes retrieved, or `-1` on error

**Structure Definition** (`procinfo.h`):
```c
struct procinfo {
  int pid;                  // Process ID
  enum procstate state;     // UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE
  int priority;             // Priority (for PBS)
  int mlfq_level;           // MLFQ queue level (0-4)
  uint cpu_ticks;           // Total CPU ticks consumed
  uint start_time;          // Process creation time
  char name[16];            // Process name
};
```

**Implementation** (`sysproc.c`):
```c
int sys_getallprocinfo(void) {
  uint user_addr;
  int max_count;
  
  if(argint(0, (int*)&user_addr) < 0) return -1;
  if(argint(1, &max_count) < 0) return -1;
  
  struct procinfo k_info[NPROC];
  int count = 0;
  
  acquire(&ptable.lock);
  for(struct proc *p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if(p->state != UNUSED && count < max_count) {
      k_info[count].pid = p->pid;
      k_info[count].state = p->state;
      k_info[count].priority = p->priority;
      k_info[count].mlfq_level = p->mlfq_level;
      k_info[count].cpu_ticks = p->cpu_ticks;
      k_info[count].start_time = p->start_time;
      strncpy(k_info[count].name, p->name, 16);
      count++;
    }
  }
  release(&ptable.lock);
  
  // Copy to user space
  if(copyout(myproc()->pgdir, user_addr, (char*)k_info, 
             count * sizeof(struct procinfo)) < 0)
    return -1;
    
  return count;
}
```

**Use Case**:
- System monitoring tools (`top`, `ps`)
- Process analysis and debugging
- Scheduler behavior visualization
- Performance profiling

**Example**:
```c
struct procinfo processes[NPROC];
int count = getallprocinfo(processes, NPROC);

for(int i = 0; i < count; i++) {
  printf("PID %d: %s (MLFQ level %d)\n", 
         processes[i].pid, processes[i].name, processes[i].mlfq_level);
}
```

---

#### `int getcpustats(struct cpustats *stats)`
**Purpose**: Retrieve system-wide CPU statistics.

**Parameters**:
- `stats`: Pointer to `struct cpustats`

**Returns**: `0` on success, `-1` on error

**Structure Definition**:
```c
struct cpustats {
  uint total_ticks;   // Total system uptime in ticks (100 ticks = 1 second)
  uint idle_ticks;    // CPU idle time (not currently tracked)
};
```

**Implementation** (`sysproc.c`):
```c
int sys_getcpustats(void) {
  uint user_addr;
  if(argint(0, (int*)&user_addr) < 0) return -1;
  
  struct cpustats k_stats;
  acquire(&cpu_stats.lock);
  k_stats.total_ticks = cpu_stats.total_ticks;
  k_stats.idle_ticks = 0;  // Not tracked due to interrupt safety
  release(&cpu_stats.lock);
  
  if(copyout(myproc()->pgdir, user_addr, (char*)&k_stats, 
             sizeof(struct cpustats)) < 0)
    return -1;
    
  return 0;
}
```

**Use Case**:
- Calculate system uptime
- Compute process uptime: `(total_ticks - start_time) / 100` seconds
- Monitor system load over time
- Performance benchmarking

**Example**:
```c
struct cpustats stats;
getcpustats(&stats);
printf("System uptime: %d seconds\n", stats.total_ticks / 100);
```

---

### **MLFQ Recording & Visualization**

#### `int mlfqstart(void)`
**Purpose**: Start recording MLFQ queue snapshots for analysis.

**Parameters**: None

**Returns**: `0` on success

**Implementation** (`proc.c`):
```c
int sys_mlfqstart(void) {
  acquire(&ptable.lock);
  mlfq_recorder.recording = 1;           // Enable recording
  mlfq_recorder.snapshot_count = 0;      // Reset counter
  record_mlfq_snapshot();                // Record initial state
  release(&ptable.lock);
  return 0;
}
```

**Behavior**:
- Enables periodic snapshot recording (every 100 ticks)
- Captures process distribution across MLFQ queues
- Stores up to `MAX_MLFQ_SNAPSHOTS` (1000) snapshots

**Use Case**:
- Analyze MLFQ behavior over time
- Verify process demotion through queues
- Educational demonstrations
- Performance analysis

---

#### `int mlfqstop(void)`
**Purpose**: Stop recording MLFQ snapshots.

**Parameters**: None

**Returns**: `0` on success

**Implementation** (`proc.c`):
```c
int sys_mlfqstop(void) {
  acquire(&ptable.lock);
  mlfq_recorder.recording = 0;  // Disable recording
  release(&ptable.lock);
  return 0;
}
```

**Use Case**: Stop recording after sufficient data collected

---

#### `int mlfqstatus(void)`
**Purpose**: Display recorded MLFQ statistics.

**Parameters**: None

**Returns**: `0` on success

**Implementation** (`sysproc.c`):
```c
int sys_mlfqstatus(void) {
  if(mlfq_recorder.recording) {
    cprintf("=== MLFQ Recorded Status ===\n");
    cprintf("Total snapshots: %d\n", mlfq_recorder.snapshot_count);
    
    // Calculate max processes seen in each queue
    int max_counts[NQUEUE] = {0};
    for(int i = 0; i < mlfq_recorder.snapshot_count; i++) {
      for(int q = 0; q < NQUEUE; q++) {
        if(mlfq_recorder.snapshots[i].counts[q] > max_counts[q])
          max_counts[q] = mlfq_recorder.snapshots[i].counts[q];
      }
    }
    
    cprintf("Max processes observed:\n");
    for(int q = 0; q < NQUEUE; q++) {
      cprintf("  Queue %d: %d processes\n", q, max_counts[q]);
    }
  } else {
    print_queues();  // Show current state if not recording
  }
  return 0;
}
```

**Output Example**:
```
=== MLFQ Recorded Status ===
Total snapshots: 5
Max processes observed:
  Queue 0: 5 processes
  Queue 1: 3 processes
  Queue 2: 2 processes
```

**Use Case**:
- Verify MLFQ is working correctly
- Analyze queue distribution patterns
- Educational demonstrations

---

### **Deadlock Detection**

#### `int getdeadlockinfo(struct deadlockinfo *info)`
**Purpose**: Check for deadlocks and retrieve cycle information.

**Parameters**:
- `info`: Pointer to `struct deadlockinfo`

**Returns**: `0` on success, `-1` on error

**Structure Definition**:
```c
struct deadlockinfo {
  int deadlock_detected;      // 1 if deadlock found, 0 otherwise
  int cycle_length;           // Number of processes in cycle
  int cycle_pids[NPROC];      // PIDs in deadlock cycle
};
```

**Implementation** (`sysproc.c`):
```c
int sys_getdeadlockinfo(void) {
  uint user_addr;
  struct deadlockinfo k_info;
  memset(&k_info, 0, sizeof(k_info));
  
  if(argint(0, (int*)&user_addr) < 0) return -1;
  
  // 1. Build Wait-For Graph (WFG)
  struct wfg_node wfg[NPROC];
  build_wfg(wfg);
  
  // 2. Detect cycle using DFS
  int cycle[NPROC];
  int cycle_len = detect_cycle_dfs(wfg, cycle);
  
  if(cycle_len > 0) {
    k_info.deadlock_detected = 1;
    k_info.cycle_length = cycle_len;
    for(int i = 0; i < cycle_len; i++) {
      k_info.cycle_pids[i] = cycle[i];
    }
  }
  
  // 3. Copy to user space
  if(copyout(myproc()->pgdir, user_addr, (char*)&k_info, 
             sizeof(struct deadlockinfo)) < 0)
    return -1;
    
  return 0;
}
```

**Algorithm**:
1. **Build WFG**: For each process, identify which lock it's waiting for and who holds it
2. **DFS Cycle Detection**: Traverse graph looking for back edges (cycles)
3. **Report Cycle**: Return PIDs involved in deadlock

**Use Case**:
- Detect resource deadlocks in multi-threaded programs
- System debugging and diagnosis
- Educational: Demonstrate deadlock conditions
- Automatic deadlock recovery systems

**Example**:
```c
struct deadlockinfo info;
getdeadlockinfo(&info);

if(info.deadlock_detected) {
  printf("Deadlock detected!\n");
  printf("Cycle: ");
  for(int i = 0; i < info.cycle_length; i++) {
    printf("PID %d -> ", info.cycle_pids[i]);
  }
  printf("PID %d\n", info.cycle_pids[0]);
} else {
  printf("No deadlock detected.\n");
}
```

---

### **Priority Management**

#### `int setpriority(int priority)`
**Purpose**: Set the priority of the calling process (for PBS scheduler).

**Parameters**:
- `priority`: Priority value (0-100, lower = higher priority)

**Returns**: `0` on success, `-1` on error

**Implementation** (`sysproc.c`):
```c
int sys_setpriority(void) {
  int priority;
  if(argint(0, &priority) < 0) return -1;
  
  if(priority < 0 || priority > 100) return -1;  // Validate range
  
  struct proc *p = myproc();
  acquire(&ptable.lock);
  p->priority = priority;
  release(&ptable.lock);
  
  return 0;
}
```

**Use Case**:
- Assign priorities to critical processes
- Test Priority-Based Scheduler behavior
- Real-time task management
- Process importance classification

**Example**:
```c
// High priority process
setpriority(1);   // Highest priority

// Low priority background task
setpriority(99);  // Lowest priority
```

---

### **System Call Summary Table**

| System Call | Purpose | Returns | Use Case |
|-------------|---------|---------|----------|
| `setscheduler(policy)` | Switch scheduler | 0/-1 | Dynamic scheduler selection |
| `getscheduler()` | Get current scheduler | policy | Monitoring, verification |
| `getallprocinfo(info, max)` | Get all process info | count/-1 | System monitoring (`top`, `ps`) |
| `getcpustats(stats)` | Get CPU statistics | 0/-1 | Uptime, performance metrics |
| `mlfqstart()` | Start MLFQ recording | 0 | Begin analysis session |
| `mlfqstop()` | Stop MLFQ recording | 0 | End analysis session |
| `mlfqstatus()` | Show MLFQ statistics | 0 | View recorded data |
| `getdeadlockinfo(info)` | Check for deadlocks | 0/-1 | Deadlock detection |
| `setpriority(pri)` | Set process priority | 0/-1 | Priority management |

---

## 📁 Project Structure

### Modified Core Files
- `proc.c` - Scheduler implementations (RR, PBS, MLFQ)
- `proc.h` - Process structure with scheduler fields
- `trap.c` - Timer interrupt handling, MLFQ demotion logic
- `syscall.c/h` - New system call registrations
- `sysproc.c` - System call implementations
- `param.h` - System constants (FSSIZE, BOOST_INTERVAL_TICKS)

### New User Programs
- `top.c` - Process monitor
- `ps.c` - Process status
- `setsched.c` - Scheduler switcher
- `deadlockinfo.c` - Deadlock checker
- `mlfqvisual.c` - MLFQ demonstration
- `mlfqtest1/2/3.c` - CPU-intensive tests
- `mlfqstatus.c` - MLFQ queue status
- `mlfqdemo.c` - Simple MLFQ demo

### Configuration
- `Makefile` - Build configuration with all user programs
- `param.h` - Tunable parameters:
  - `FSSIZE = 2500` - Filesystem size in blocks
  - `BOOST_INTERVAL_TICKS = 1000` - MLFQ priority boost interval

---

## 🔧 Configuration & Tuning

### Filesystem Size
Edit `param.h`:
```c
#define FSSIZE 2500  // Increase if adding more programs
```

### MLFQ Parameters
Edit `param.h`:
```c
#define BOOST_INTERVAL_TICKS 1000  // Priority boost interval (10 seconds)
```

Edit `trap.c` for time slices:
```c
int time_slices[] = {1, 2, 4, 8, 16};  // Per-level time slices
```

### Default Scheduler
Edit `proc.c`:
```c
int current_scheduler_policy = SCHED_MLFQ;  // 0=RR, 1=PBS, 2=MLFQ
```

---

## 🐞 Troubleshooting

### Build Errors

**Issue**: `mkfs: read sector error`
- **Cause**: WSL file I/O timing issue
- **Fix**: Retry logic already implemented in `mkfs.c`

**Issue**: Missing programs in filesystem
- **Cause**: FSSIZE too small
- **Fix**: Increase FSSIZE in `param.h` (currently 2500)

### Runtime Issues

**Issue**: Kernel panic on `mycpu()`
- **Cause**: Interrupts enabled when calling `mycpu()`
- **Fix**: Already fixed - interrupts disabled before `mycpu()` calls

**Issue**: MLFQ levels not changing
- **Cause**: Priority boost too frequent
- **Fix**: BOOST_INTERVAL_TICKS set to 1000 (10 seconds)

**Issue**: Huge uptime values
- **Cause**: Integer underflow in uptime calculation
- **Fix**: Already fixed with wraparound protection

**Issue**: `mlfqstatus` shows 0 processes
- **Cause**: Snapshots not recorded periodically, test programs finish too fast
- **Fix**: 
  - Periodic snapshot recording added (every 100 ticks in MLFQ scheduler)
  - Test programs extended to run longer (100,000 iterations)
  - Use `mlfqstart` before running tests, wait 3+ seconds, then check `mlfqstatus`

---

## 📈 Performance Notes

### Process States
- **`init` (PID 1)**: System initialization, usually SLEEPING
- **`sh` (PID 2)**: Shell, SLEEPING when waiting for input
- **`top`**: RUNNING when displaying, SLEEPING between updates
- **Zombie processes**: Completed but not yet reaped by parent

### CPU Usage
- `cpu_ticks`: Total ticks process has been RUNNING
- `total_ticks`: System uptime in ticks (100 ticks = 1 second)
- `uptime`: Process uptime = (total_ticks - start_time) / 100

---

## 🎯 Testing Scenarios

### Test Scheduler Switching
```bash
$ setsched 0 && top  # Round-Robin
$ setsched 1 && top  # Priority-Based
$ setsched 2 && top  # MLFQ
```

### Test MLFQ Demotion
```bash
$ setsched 2
$ top
# Watch MLFQ column: 0 → 1 → 2 → 3 → 4 → 0 (boost)
```

### Test Deadlock Detection
```bash
$ dltest           # Run deadlock test program
$ deadlockinfo     # Check for deadlocks
```

### Stress Test
```bash
$ mlfqtest1 &
$ mlfqtest2 &
$ mlfqtest3 &
$ top
# Observe multiple processes competing for CPU
```

---

## 📝 Known Limitations

1. **Priority boost resets all processes** - MLFQ boost affects all processes simultaneously
2. **No I/O priority** - I/O-bound processes treated same as CPU-bound
3. **Fixed time slices** - Not dynamically adjusted based on workload
4. **Single CPU scheduling** - Multi-core scheduling not implemented
5. **Deadlock detection overhead** - May impact performance in lock-heavy workloads

---

## 🤝 Contributing

This is an educational project based on MIT's xv6. Feel free to:
- Experiment with scheduler parameters
- Add new scheduling algorithms
- Improve deadlock detection
- Enhance monitoring tools

---

## 📚 References

- [MIT xv6 Documentation](https://pdos.csail.mit.edu/6.828/2018/xv6.html)
- [xv6 Book](https://pdos.csail.mit.edu/6.828/2018/xv6/book-rev11.pdf)
- MLFQ Algorithm: "Operating Systems: Three Easy Pieces" by Remzi H. Arpaci-Dusseau

---

## 📄 License

Based on MIT's xv6, which is licensed under the MIT License.

---

## ✅ Build Status

- **Filesystem**: 2500 blocks (all 31 programs included)
- **Schedulers**: RR, PBS, MLFQ (all working)
- **System Calls**: 34 total (including custom ones)
- **User Programs**: 31 programs in filesystem
- **Status**: ✅ Fully functional

---

**Last Updated**: November 2025
