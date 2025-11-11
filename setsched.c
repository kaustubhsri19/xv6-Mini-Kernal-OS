#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  if(argc != 2){
    printf(2, "Usage: setsched <policy_num>\n");
    printf(2, "  0: Round-Robin\n");
    printf(2, "  1: Priority-Based Scheduler\n");
    printf(2, "  2: MLFQ Scheduler\n");
    exit();
  }

  // Validate argument is not null
  if(argv[1] == 0){
    printf(2, "Error: Invalid argument.\n");
    exit();
  }

  int policy = atoi(argv[1]);

  if(policy < 0 || policy > 2){
    printf(2, "Error: Invalid policy number.\n");
    exit();
  }

  if(setscheduler(policy) < 0){
    printf(2, "Error: setscheduler system call failed.\n");
    exit();
  }

  printf(1, "Scheduler policy set to %d.\n", policy);
  exit();
}
