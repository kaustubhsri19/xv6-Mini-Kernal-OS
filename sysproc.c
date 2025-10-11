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
    print_queues();
    return 0;
}
