#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
    int i, pid;
    int worker_pids[5];
    
    printf(1, "=== MLFQ Visual Test Starting ===\n");
    printf(1, "Time slices: Queue 0=5 ticks, Queue 1=10 ticks, Queue 2=20 ticks\n");
    printf(1, "Spawning worker processes...\n\n");
    
    // Fork 5 worker processes
    for(i = 0; i < 5; i++) {
        pid = fork();
        if(pid == 0) {
            // Child process - run CPU intensive work
            int j, k;
            printf(1, "Worker %d (PID %d) started\n", i+1, getpid());
            
            // CPU intensive loop with periodic output
            for(j = 0; j < 100000; j++) {
                // Busy work
                for(k = 0; k < 10000; k++) {
                    // CPU intensive work
                }
                
                // Print progress every 10000 iterations
                if(j % 10000 == 0) {
                    printf(1, "Worker %d (PID %d): Iteration %d\n", i+1, getpid(), j);
                }
                
                // Sleep occasionally to allow other processes to run
                if(j % 20000 == 0) {
                    sleep(1);
                }
            }
            
            printf(1, "Worker %d (PID %d): Completed!\n", i+1, getpid());
            exit();
        } else if(pid > 0) {
            // Parent process - store child PID
            worker_pids[i] = pid;
        } else {
            printf(1, "Fork failed for worker %d\n", i+1);
        }
    }
    
    printf(1, "\nAll workers spawned. Monitoring MLFQ status...\n\n");
    
    // Parent process - monitor MLFQ status
    for(i = 0; i < 10; i++) {
        printf(1, "[T=%ds] MLFQ Status:\n", i*3);
        mlfqstatus();
        printf(1, "\n");
        
        // Wait 3 seconds before next check
        sleep(3);
    }
    
    printf(1, "=== MLFQ Visual Test Complete ===\n");
    printf(1, "Expected behavior:\n");
    printf(1, "- All processes start in Queue 0 (highest priority)\n");
    printf(1, "- CPU-intensive processes move to Queue 1 after 5 ticks\n");
    printf(1, "- Processes move to Queue 2 after 10 more ticks\n");
    printf(1, "- Queue 0 processes always run before Queue 1/2\n");
    
    exit();
}
