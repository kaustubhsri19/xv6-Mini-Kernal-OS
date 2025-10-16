#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
  printf(1, "Starting MLFQ recording...\n");
  mlfqstart();
  
  // Fork CPU-intensive processes
  for(int i = 0; i < 3; i++) {
    int pid = fork();
    if(pid == 0) {
      // CPU intensive work
      for(int j = 0; j < 100000; j++) {
        for(int k = 0; k < 1000; k++) {
          // Busy work
        }
      }
      exit();
    }
  }
  
  // Wait for children
  for(int i = 0; i < 3; i++) {
    wait();
  }
  
  mlfqstop();
  printf(1, "\nRecording complete!\n\n");
  
  mlfqstatus();  // Show summary
  printf(1, "\n");
  mlfqvisual();  // Show detailed transitions
  
  exit();
}
