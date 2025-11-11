#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
  printf(1, "\n");
  printf(1, "========================================\n");
  printf(1, "    MLFQ DEMONSTRATION & RECORDING\n");
  printf(1, "========================================\n\n");
  
  printf(1, "This demo will:\n");
  printf(1, "  1. Start MLFQ recording\n");
  printf(1, "  2. Launch 3 CPU-intensive worker processes\n");
  printf(1, "  3. Track how they move through MLFQ queues\n");
  printf(1, "  4. Show you the results\n\n");
  
  printf(1, "MLFQ Queue Levels:\n");
  printf(1, "  Queue 0: Highest priority (5 tick time slice)\n");
  printf(1, "  Queue 1: Medium priority  (10 tick time slice)\n");
  printf(1, "  Queue 2: Lowest priority  (20 tick time slice)\n\n");
  
  printf(1, "Starting recording...\n");
  mlfqstart();
  
  printf(1, "Launching 3 worker processes...\n");
  int child_pids[3];
  
  // Fork CPU-intensive processes
  for(int i = 0; i < 3; i++) {
    int pid = fork();
    if(pid == 0) {
      // Child process - do CPU intensive work
      printf(1, "  Worker %d (PID %d) started in Queue 0\n", i+1, getpid());
      
      for(int j = 0; j < 100000; j++) {
        for(int k = 0; k < 1000; k++) {
          // Busy work to consume CPU time
        }
      }
      
      printf(1, "  Worker %d (PID %d) completed\n", i+1, getpid());
      exit();
    } else {
      child_pids[i] = pid;
    }
  }
  
  printf(1, "\nWorkers running... (this will take a few seconds)\n");
  
  // Wait for children
  for(int i = 0; i < 3; i++) {
    wait();
  }
  
  mlfqstop();
  
  printf(1, "\n");
  printf(1, "========================================\n");
  printf(1, "         RECORDING COMPLETE!\n");
  printf(1, "========================================\n\n");
  
  printf(1, "RESULTS:\n");
  printf(1, "--------\n\n");
  
  mlfqstatus();  // Show summary
  
  printf(1, "\n");
  printf(1, "EXPLANATION:\n");
  printf(1, "------------\n");
  printf(1, "Each worker started in Queue 0 (highest priority).\n");
  printf(1, "After consuming their time slice, they were demoted:\n");
  printf(1, "  Queue 0 (5 ticks) -> Queue 1 (10 ticks) -> Queue 2 (20 ticks)\n\n");
  printf(1, "The numbers above show the maximum processes observed\n");
  printf(1, "in each queue during the recording period.\n\n");
  
  printf(1, "========================================\n\n");
  
  exit();
}
