#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"
#include "proc.h"
#include "procinfo.h"

// Define cpustats_kernel structure here since it's not in a header
struct cpustats_kernel {
  struct spinlock lock;
  uint total_ticks;
  uint idle_ticks;
};

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_getsysinfo(void) {
    struct sysinfo {
        int nproc;
        uint ticks;
    } info;
    
    // For now, return a simple count - this is a placeholder implementation
    // In a full implementation, we would iterate through the process table
    info.nproc = 1; // At least the current process is running
    
    acquire(&tickslock);
    info.ticks = ticks;
    release(&tickslock);
    
    uint a0;
    if(argint(0, (int*)&a0) < 0)
        return -1;
    if(copyout(myproc()->pgdir, a0, (char*)&info, sizeof(info)) < 0)
        return -1;
    return 0;
}

int
sys_detect_deadlock(void) {
    return detect_deadlock();
}

int
sys_mlfqstatus(void) {
  if(mlfq_recorder.recording) {
    // Show summary of recorded data when recording is active
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
    // Show current state when not recording
    print_queues();
  }
  return 0;
}

int
sys_mlfqvisual(void) {
  if(!mlfq_recorder.recording && mlfq_recorder.snapshot_count == 0) {
    cprintf("No MLFQ data recorded. Call mlfqstart() first.\n");
    return -1;
  }
  
  if(mlfq_recorder.snapshot_count == 0) {
    cprintf("Recording started but no transitions captured yet.\n");
    return 0;
  }
  
  cprintf("\n=== MLFQ Visual Recording ===\n");
  cprintf("Showing %d queue transitions\n\n", mlfq_recorder.snapshot_count);
  
  // Show all snapshots
  for(int i = 0; i < mlfq_recorder.snapshot_count; i++) {
    struct mlfq_snapshot *snap = &mlfq_recorder.snapshots[i];
    cprintf("--- Tick %d ---\n", snap->tick);
    
    for(int q = 0; q < NQUEUE; q++) {
      cprintf("Queue %d: %d processes [", q, snap->counts[q]);
      for(int j = 0; j < snap->pid_counts[q] && j < 5; j++) {
        cprintf("%d", snap->pids[q][j]);
        if(j < snap->pid_counts[q] - 1 && j < 4) cprintf(", ");
      }
      if(snap->pid_counts[q] > 5) cprintf("...");
      cprintf("]\n");
    }
    cprintf("\n");
  }
  
  // Show process transitions
  cprintf("=== Process Transitions ===\n");
  // Track each PID through snapshots
  // (implementation showing which processes moved between queues)
  
  return 0;
}

int
sys_mlfqrealtime(void) {
  // Always show current real-time status, regardless of recording state
  print_queues();
  return 0;
}

int
sys_setpriority(void) {
  int priority;
  
  if(argint(0, &priority) < 0)
    return -1;
    
  if(priority < 1 || priority > 3)
    return -1;
    
  struct proc *curproc = myproc();
  if(curproc) {
    curproc->priority = priority;
    // Immediately assign to appropriate queue
    int target_queue = priority - 1;
    curproc->queue_level = target_queue;
    // Set appropriate time slice
    if(target_queue == 0) curproc->time_slice = TIME_SLICE_0;
    else if(target_queue == 1) curproc->time_slice = TIME_SLICE_1;
    else curproc->time_slice = TIME_SLICE_2;
  }
  
  return 0;
}

// Externs for new features
extern struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

extern int current_scheduler_policy;
extern struct spinlock policy_lock;
extern struct cpustats_kernel cpu_stats;
extern void build_wfg(void);
extern int dfs_check_cycle(int);
extern int visited[NPROC];
extern int recursion_stack[NPROC];
extern int cycle[NPROC];
extern int cycle_len;

int
sys_getallprocinfo(void)
{
  uint user_buf_addr; // User-space pointer to an array
  int max_count;        // Max size of user's array
  
  if(argint(0, (int*)&user_buf_addr) < 0 || argint(1, &max_count) < 0) {
    return -1;
  }
  
  // Validate max_count
  if(max_count <= 0 || max_count > NPROC) {
    return -1;
  }
  
  struct procinfo k_info_buf[NPROC];
  struct proc *p;
  int count = 0;

  acquire(&ptable.lock);
  
  for(p = ptable.proc; p < &ptable.proc[NPROC] && count < max_count; p++) {
    if(p->state == UNUSED)
      continue;
    
    k_info_buf[count].pid = p->pid;
    k_info_buf[count].ppid = p->parent ? p->parent->pid : 0;
    k_info_buf[count].state = p->state;
    safestrcpy(k_info_buf[count].name, p->name, 16);
    k_info_buf[count].priority = p->priority;
    k_info_buf[count].mlfq_level = p->mlfq_level;
    k_info_buf[count].start_time = p->start_time;
    k_info_buf[count].cpu_ticks = p->cpu_ticks;
    
    count++;
  }
  
  release(&ptable.lock);

  if(copyout(myproc()->pgdir, user_buf_addr, (char*)k_info_buf, count * sizeof(struct procinfo)) < 0) {
    return -1;
  }
  
  return count;
}

int
sys_getcpustats(void)
{
  uint user_addr;
  struct cpustats k_stats; // Kernel-side temporary copy

  if(argint(0, (int*)&user_addr) < 0)
    return -1;
    
  acquire(&cpu_stats.lock);
  k_stats.total_ticks = cpu_stats.total_ticks;
  k_stats.idle_ticks = cpu_stats.idle_ticks;
  release(&cpu_stats.lock);
  
  if(copyout(myproc()->pgdir, user_addr, (char*)&k_stats, sizeof(k_stats)) < 0)
    return -1;
    
  return 0;
}

int
sys_setscheduler(void)
{
  int policy;

  if(argint(0, &policy) < 0)
    return -1;

  // Validate policy range
  if(policy < 0 || policy > 2) // 0=RR, 1=PBS, 2=MLFQ
    return -1; // Invalid policy number

  acquire(&policy_lock);
  current_scheduler_policy = policy;
  release(&policy_lock);
  
  return 0;
}

int
sys_getscheduler(void)
{
  int policy;
  
  acquire(&policy_lock);
  policy = current_scheduler_policy;
  release(&policy_lock);
  
  return policy;
}

int
sys_getdeadlockinfo(void)
{
  uint user_addr;
  struct deadlockinfo k_info;
  memset(&k_info, 0, sizeof(k_info));

  if(argint(0, (int*)&user_addr) < 0)
    return -1;

  // 1. Build the WFG
  build_wfg();

  // 2. Clear DFS state and search for a cycle
  memset(visited, 0, sizeof(visited));
  memset(recursion_stack, 0, sizeof(recursion_stack));
  cycle_len = 0;

  // Only check processes with valid PIDs
  for(int i = 0; i < NPROC; i++) {
    struct proc *proc = &ptable.proc[i];
    if(proc->state != UNUSED && proc->pid > 0 && proc->pid < NPROC && !visited[proc->pid]) {
      if(dfs_check_cycle(proc->pid)) {
        k_info.found = 1;
        // Copy the cycle array into k_info
        for(int j=0; j < cycle_len && j < NPROC; j++)
          k_info.pids_in_cycle[j] = cycle[j];
        break; // Found one, that's enough
      }
    }
  }

  // 3. Copy result to user
  if(copyout(myproc()->pgdir, user_addr, (char*)&k_info, sizeof(k_info)) < 0)
    return -1;
    
  return 0;
}
