#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

extern uint ticks;
extern struct spinlock tickslock;

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

// MLFQ recorder instance (declared as extern in proc.h)
struct mlfq_recorder_t mlfq_recorder;

static struct proc *initproc;

int nextpid = 1;

// --- NEW GLOBAL SCHEDULER VARS ---
#define SCHED_RR 0      // Default Round-Robin
#define SCHED_PBS 1     // Priority-Based Scheduler
#define SCHED_MLFQ 2    // Multi-Level Feedback Queue

int current_scheduler_policy = SCHED_MLFQ;  // Start with MLFQ to demonstrate it works
struct spinlock policy_lock;

// MLFQ Global
uint last_boost_tick = 0;

// CPU Stats Global - Full definition here
struct cpustats_kernel {
  struct spinlock lock;
  uint total_ticks;
  uint idle_ticks;
};

struct cpustats_kernel cpu_stats;

// Deadlock Detector Globals
int wfg[NPROC][NPROC];
int visited[NPROC];
int recursion_stack[NPROC];
int cycle[NPROC];
int cycle_len = 0;
// --- END NEW GLOBALS ---
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
statsinit(void)
{
  initlock(&cpu_stats.lock, "cpu_stats");
  cpu_stats.total_ticks = 0;
  cpu_stats.idle_ticks = 0;
}

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
  initlock(&policy_lock, "policy");
  statsinit(); // Initialize new stats structure
  // MLFQ initialization happens per-process in allocproc()
}


void print_queues(void){
  struct proc *p;
  int counts[NQUEUE] = {0};
  int pids[NQUEUE][NPROC];  // Store PIDs for each queue
  int pid_counts[NQUEUE] = {0};  // Count of PIDs stored for each queue
  
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == RUNNABLE && p->queue_level < NQUEUE) {
      counts[p->queue_level]++;
      if(pid_counts[p->queue_level] < NPROC) {
        pids[p->queue_level][pid_counts[p->queue_level]] = p->pid;
        pid_counts[p->queue_level]++;
      }
    }
  }
  release(&ptable.lock);
  
  cprintf("┌─────────────────────────────────┐\n");
  cprintf("│           MLFQ STATUS           │\n");
  cprintf("├─────────────────────────────────┤\n");
  
  for(int i = 0; i < NQUEUE; i++){
    char priority[20];
    if(i == 0) {
      priority[0] = 'H'; priority[1] = 'I'; priority[2] = 'G'; priority[3] = 'H';
      priority[4] = 'E'; priority[5] = 'S'; priority[6] = 'T'; priority[7] = '\0';
    } else if(i == 1) {
      priority[0] = 'M'; priority[1] = 'E'; priority[2] = 'D'; priority[3] = 'I';
      priority[4] = 'U'; priority[5] = 'M'; priority[6] = ' '; priority[7] = '\0';
    } else {
      priority[0] = 'L'; priority[1] = 'O'; priority[2] = 'W'; priority[3] = 'E';
      priority[4] = 'S'; priority[5] = 'T'; priority[6] = ' '; priority[7] = '\0';
    }
    
    cprintf("│ Queue %d (%s): %d processes", i, priority, counts[i]);
    
    if(counts[i] > 0) {
      cprintf(" [PIDs:");
      for(int j = 0; j < pid_counts[i] && j < 4; j++) {  // Show up to 4 PIDs
        cprintf(" %d", pids[i][j]);
      }
      if(counts[i] > 4) cprintf("...");
      cprintf("]");
    }
    
    // Add padding to align the closing bracket
    int padding = 33 - (counts[i] > 0 ? 8 + pid_counts[i] * 3 : 0);
    if(counts[i] > 4) padding -= 3;
    for(int k = 0; k < padding; k++) cprintf(" ");
    cprintf("│\n");
  }
  
  cprintf("└─────────────────────────────────┘\n");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  
  // Initialize MLFQ fields (legacy)
  p->queue_level = 0;              // Start in highest priority queue
  p->time_slice = TIME_SLICE_0;    // Get full time slice
  p->total_runtime = 0;            // No runtime yet
  
  // Initialize new fields
  p->priority = 60;                // Default priority
  p->wait_time = 0;
  p->mlfq_level = 0;               // New processes in highest queue
  p->mlfq_ticks = 0;
  p->start_time = ticks;           // Capture the current time
  p->cpu_ticks = 0;
  p->waiting_on_chan = 0;

  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;
  // add_to_mlfq(p);  // MLFQ disabled temporarily

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));
  
  // Child inherits scheduling parameters
  np->priority = curproc->priority;
  np->mlfq_level = curproc->mlfq_level;

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;
  // add_to_mlfq(np);  // MLFQ disabled temporarily

  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Remove from MLFQ queue
  // remove_from_mlfq(curproc);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Forward declarations for the policy implementations
static void scheduler_rr(struct cpu *c);
static void scheduler_pbs(struct cpu *c);
static void scheduler_mlfq(struct cpu *c);

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns. It loops, doing:
// - choose a process to run
// - swtch to start running that process
// - eventually that process transfers control
//   via swtch back to the scheduler
void
scheduler(void)
{
  struct cpu *c;
  
  // Disable interrupts before calling mycpu()
  pushcli();
  c = mycpu();
  c->proc = 0;
  popcli();

  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Read the current policy.
    // We acquire the lock to ensure we don't read the
    // value mid-write (a "tear") from another CPU.
    acquire(&policy_lock);
    int policy = current_scheduler_policy;
    release(&policy_lock);

    // Dispatch to the correct scheduler
    if(policy == SCHED_PBS) {
      scheduler_pbs(c);
    } else if(policy == SCHED_MLFQ) {
      scheduler_mlfq(c);
    } else {
      scheduler_rr(c); // Default
    }
  }
}

// Original Round-Robin Scheduler
static void
scheduler_rr(struct cpu *c)
{
  struct proc *p;
  
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if(p->state != RUNNABLE)
      continue;
      
    // Switch to chosen process.
    c->proc = p;
    switchuvm(p);
    p->state = RUNNING;

    swtch(&c->scheduler, p->context);
    switchkvm();

    // Process is done running for now.
    c->proc = 0;
  }
  release(&ptable.lock);
}

// Priority-Based Scheduler with Aging
static void
scheduler_pbs(struct cpu *c)
{
  struct proc *p;
  struct proc *best_p = 0;
  int min_priority = 10000; // A high number

  acquire(&ptable.lock);

  // 1. Find the highest-priority (lowest value) runnable process
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if(p->state == RUNNABLE) {
      if(p->priority < min_priority) {
        min_priority = p->priority;
        best_p = p;
      }
    }
  }

  // 2. If a process was found, perform aging on all *other* runnable processes
  if(best_p) {
    #define AGING_THRESHOLD 50 // Example threshold: 50 ticks
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
      if(p->state == RUNNABLE && p != best_p) {
        // Prevent wait_time overflow
        if(p->wait_time < 10000)
          p->wait_time++;
        if(p->wait_time > AGING_THRESHOLD) {
          if(p->priority > 0) // Don't let priority go below 0
            p->priority--; // Boost priority
          p->wait_time = 0;
          // Re-check if this aged process is now the best
          if(p->priority < best_p->priority)
            best_p = p;
        }
      }
    }
  }

  // 3. Run the best process
  if(best_p) {
    best_p->state = RUNNING;
    best_p->wait_time = 0; // Reset wait time
    c->proc = best_p;
    switchuvm(best_p);
    
    swtch(&c->scheduler, best_p->context);
    switchkvm();
    
    c->proc = 0;
  }

  release(&ptable.lock);
}

// New MLFQ Scheduler
static void
scheduler_mlfq(struct cpu *c)
{
  struct proc *p;
  struct proc *best_p = 0;

  acquire(&ptable.lock);
  
  // 1. Check for Priority Boost
  acquire(&tickslock);
  uint current_ticks = ticks;
  release(&tickslock);
  
  if(current_ticks > last_boost_tick + BOOST_INTERVAL_TICKS) {
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
      if(p->state != UNUSED) {
        p->mlfq_level = 0;
        p->mlfq_ticks = 0;
      }
    }
    last_boost_tick = current_ticks;
  }
  
  // Record MLFQ snapshot every 100 ticks if recording is active
  static uint last_snapshot_tick = 0;
  if(mlfq_recorder.recording && current_ticks >= last_snapshot_tick + 100) {
    record_mlfq_snapshot();
    last_snapshot_tick = current_ticks;
  }
  
  // 2. Find highest-priority runnable process
  // Iterate from Q0 down to Q(NQUEUE-1)
  for(int level = 0; level < NQUEUE; level++) {
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
      if(p->state == RUNNABLE && p->mlfq_level == level) {
        best_p = p;
        goto found; // Found our process
      }
    }
  }
  // No runnable process
  release(&ptable.lock);
  return; 

found:
  // 3. Run the chosen process
  best_p->state = RUNNING;
  c->proc = best_p;
  switchuvm(best_p);
  
  swtch(&c->scheduler, best_p->context);
  switchkvm();
  
  c->proc = 0;
  
  release(&ptable.lock);
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  struct proc *p = myproc();
  p->state = RUNNABLE;
  
  // Remove from current queue and add back to same queue
  // remove_from_mlfq(p);
  // add_to_mlfq(p);
  
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan) {
      // --- DEADLOCK INSTRUMENTATION (WAKEUP) ---
      p->waiting_on_chan = 0; // No longer waiting
      // --- END INSTRUMENTATION ---
      
      p->state = RUNNABLE;
      // add_to_mlfq(p);  // MLFQ disabled temporarily
    }
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING) {
        p->state = RUNNABLE;
        // add_to_mlfq(p);  // MLFQ disabled temporarily
      }
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
    "unused", "embryo", "sleep ", "runble", "run   ", "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}

// MLFQ Recording Functions

void record_mlfq_snapshot(void) {
  if(mlfq_recorder.snapshot_count >= MAX_MLFQ_SNAPSHOTS)
    return;
    
  struct mlfq_snapshot *snap = &mlfq_recorder.snapshots[mlfq_recorder.snapshot_count];
  snap->tick = ticks;
  
  // Reset counts
  for(int i = 0; i < NQUEUE; i++) {
    snap->counts[i] = 0;
    snap->pid_counts[i] = 0;
  }
  
  // Scan process table - capture RUNNABLE and RUNNING processes
  struct proc *p;
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if((p->state == RUNNABLE || p->state == RUNNING) && p->mlfq_level < NQUEUE) {
      int q = p->mlfq_level;
      snap->counts[q]++;
      if(snap->pid_counts[q] < NPROC) {
        snap->pids[q][snap->pid_counts[q]] = p->pid;
        snap->pid_counts[q]++;
      }
    }
  }
  
  mlfq_recorder.snapshot_count++;
}

int sys_mlfqstart(void) {
  acquire(&ptable.lock);
  mlfq_recorder.recording = 1;
  mlfq_recorder.snapshot_count = 0;
  record_mlfq_snapshot();  // Record initial state
  release(&ptable.lock);
  return 0;
}

int sys_mlfqstop(void) {
  acquire(&ptable.lock);
  mlfq_recorder.recording = 0;
  release(&ptable.lock);
  return 0;
}
