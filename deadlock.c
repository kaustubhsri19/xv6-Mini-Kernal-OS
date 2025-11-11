#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "spinlock.h"
#include "proc.h"
#include "sleeplock.h"

// Globals defined in proc.c
extern struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

extern int wfg[NPROC][NPROC];
extern int visited[NPROC];
extern int recursion_stack[NPROC];
extern int cycle[NPROC];
extern int cycle_len;

// Build the Wait-For Graph
void
build_wfg(void)
{
  struct proc *p;
  memset(wfg, 0, sizeof(wfg));

  // Must hold ptable lock to safely read proc states
  // This is a simplified, racy-read version
  // A robust version would need to lock all sleeplocks
  acquire(&ptable.lock);
  
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    // We only care about processes that are sleeping on a channel
    if(p->state != SLEEPING || p->waiting_on_chan == 0)
      continue;

    // This process 'p' is a "waiter".
    // We assume the channel is a sleeplock.
    struct sleeplock *lk = (struct sleeplock*)p->waiting_on_chan;

    // Racy read of the holder's PID
    // Note: lk->pid is protected by lk->lk, which we do not hold.
    // This is a tradeoff for a non-invasive detector.
    int holder_pid = lk->pid; 

    if(holder_pid > 0 && holder_pid != p->pid) {
      // Map PIDs to array indices with proper bounds checking
      // PIDs start at 1, array indices start at 0
      if (p->pid > 0 && p->pid < NPROC && holder_pid > 0 && holder_pid < NPROC)
        wfg[p->pid][holder_pid] = 1; // p->pid waits for holder_pid
    }
  }
  
  release(&ptable.lock);
}

// DFS Cycle Check
int
dfs_check_cycle(int u)
{
  // Bounds check
  if(u < 0 || u >= NPROC)
    return 0;
    
  visited[u] = 1;
  recursion_stack[u] = 1; // Mark as "gray"
  
  // Prevent cycle array overflow
  if(cycle_len < NPROC)
    cycle[cycle_len++] = u;

  for(int v = 0; v < NPROC; v++) {
    if(wfg[u][v]) { // If there is an edge u -> v
      if(!visited[v]) {
        if(dfs_check_cycle(v)) // Recurse
          return 1;
      } else if(recursion_stack[v]) {
        // CYCLE DETECTED!
        return 1;
      }
    }
  }
  
  recursion_stack[u] = 0; // Mark as "black"
  if(cycle_len > 0)
    cycle_len--;
  return 0;
}

// Legacy function for compatibility
int detect_deadlock(void){
    // TODO: Implement DFS-based cycle detection later
    // Placeholder return
    return 0;
}
