#include "types.h"
#include "stat.h"
#include "user.h"
#include "procinfo.h"

// Helper to convert enum procstate to string
const char*
get_state_str(enum procstate state)
{
  static const char *states[] = {
    "UNUSED  ",
    "EMBRYO  ",
    "SLEEPING",
    "RUNNABLE",
    "RUNNING ",
    "ZOMBIE  "
  };
  return states[state];
}

int
main(int argc, char *argv[])
{
  struct procinfo processes[NPROC];
  int count;

  // 1. Make a *single* system call
  count = getallprocinfo(processes, NPROC);
  if(count < 0){
    printf(2, "ps: getallprocinfo failed\n");
    exit();
  }

  // 2. Print header
  printf(1, "PID\tPPID\tSTATE\t\tPRI\tMLFQ\tCPU_TICKS\tNAME\n");

  // 3. Loop through the *local* array and print
  for(int i = 0; i < count; i++){
    printf(1, "%d\t%d\t%s\t%d\t%d\t%d\t\t%s\n",
      processes[i].pid,
      processes[i].ppid,
      get_state_str(processes[i].state),
      processes[i].priority,
      processes[i].mlfq_level,
      processes[i].cpu_ticks,
      processes[i].name
    );
  }

  exit();
}
