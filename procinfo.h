#ifndef _PROCINFO_H_
#define _PROCINFO_H_

#include "param.h"

// Forward declaration of enum procstate (defined in proc.h)
// Don't redefine it here to avoid conflicts
#ifndef PROCSTATE_DEFINED
#define PROCSTATE_DEFINED
enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };
#endif

// User-space-safe structure for process info
struct procinfo {
  int pid;
  int ppid;
  enum procstate state;
  char name[16];
  int priority;           // From PBS
  int mlfq_level;         // From MLFQ
  uint start_time;
  uint cpu_ticks;
};

// User-space-safe structure for CPU stats
// Must match cpustats in proc.c, but without the lock
struct cpustats {
  uint total_ticks;
  uint idle_ticks;
};

// User-space-safe structure for deadlock info
struct deadlockinfo {
  int found;
  int pids_in_cycle[NPROC];
};

#endif // _PROCINFO_H_
