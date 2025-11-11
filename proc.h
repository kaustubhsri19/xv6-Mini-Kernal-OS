// Forward declaration
struct spinlock;

// Per-CPU state
struct cpu {
  uchar apicid;                // Local APIC ID
  struct context *scheduler;   // swtch() here to enter scheduler
  struct taskstate ts;         // Used by x86 to find stack for interrupt
  struct segdesc gdt[NSEGS];   // x86 global descriptor table
  volatile uint started;       // Has the CPU started?
  int ncli;                    // Depth of pushcli nesting.
  int intena;                  // Were interrupts enabled before pushcli?
  struct proc *proc;           // The process running on this cpu or null
};

extern struct cpu cpus[NCPU];
extern int ncpu;

// Define enum procstate with include guard
#ifndef PROCSTATE_DEFINED
#define PROCSTATE_DEFINED
enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };
#endif

//PAGEBREAK: 17
// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and allocproc() manipulates it.
struct context {
  uint edi;
  uint esi;
  uint ebx;
  uint ebp;
  uint eip;
};

// Per-process state
struct proc {
  uint sz;                     // Size of process memory (bytes)
  pde_t* pgdir;                // Page table
  char *kstack;                // Bottom of kernel stack for this process
  enum procstate state;        // Process state
  int pid;                     // Process ID
  struct proc *parent;         // Parent process
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // swtch() here to run process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)
  
  // -- FIELDS FOR PRIORITY-BASED SCHEDULER (PBS) --
  int priority;                // Static priority, set by set_priority()
  int wait_time;               // Ticks spent in RUNNABLE, used for PBS aging
  
  // -- FIELDS FOR MULTI-LEVEL FEEDBACK QUEUE (MLFQ) --
  int mlfq_level;              // Current queue level (e.g., 0-4)
  int mlfq_ticks;              // Ticks executed at current level
  int queue_level;             // Legacy: Current priority queue (0 = highest)
  int time_slice;              // Legacy: Remaining time slice
  int total_runtime;           // Legacy: Total ticks consumed
  
  // -- FIELDS FOR PERFORMANCE MONITORING (top) --
  uint start_time;             // Kernel ticks at process creation (allocproc)
  uint cpu_ticks;              // Total ticks this process has been RUNNING
  
  // -- FIELD FOR DEADLOCK DETECTION --
  void *waiting_on_chan;       // The 'chan' this process is sleeping on
};

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap

// MLFQ recording structure
struct mlfq_snapshot {
  int tick;                      // System tick when recorded
  int counts[NQUEUE];            // Process count per queue
  int pids[NQUEUE][NPROC];       // PIDs in each queue
  int pid_counts[NQUEUE];        // Number of PIDs per queue
};

// MLFQ recorder structure type
struct mlfq_recorder_t {
  int recording;                           // 1 if recording, 0 otherwise
  int snapshot_count;                      // Number of snapshots taken
  struct mlfq_snapshot snapshots[MAX_MLFQ_SNAPSHOTS];
};

// External declaration (defined in proc.c)
extern struct mlfq_recorder_t mlfq_recorder;

// Global CPU stats structure (defined in proc.c)
// We declare it as an incomplete type here to avoid circular dependencies
struct cpustats_kernel;

extern struct cpustats_kernel cpu_stats;
extern struct spinlock policy_lock;
extern int current_scheduler_policy;

// Function declarations
void print_queues(void);
void record_mlfq_snapshot(void);
