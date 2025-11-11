#include "types.h"
#include "stat.h"
#include "user.h"
#include "procinfo.h"

int
main(int argc, char *argv[])
{
  struct deadlockinfo info;

  if(getdeadlockinfo(&info) < 0) {
    printf(2, "deadlockinfo: syscall failed\n");
    exit();
  }

  if(info.found) {
    printf(1, "--- DEADLOCK DETECTED ---\n");
    printf(1, "Process cycle: ");
    for(int i = 0; i < NPROC; i++) {
      if(info.pids_in_cycle[i] == 0)
        break;
      printf(1, "%d -> ", info.pids_in_cycle[i]);
    }
    printf(1, "...\n");
  } else {
    printf(1, "No deadlock detected.\n");
  }
  
  exit();
}
