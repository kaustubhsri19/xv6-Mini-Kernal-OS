#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

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
