#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
    int i, pid;
    
    printf(1, "\n");
    printf(1, "========================================\n");
    printf(1, "    MLFQ VISUAL DEMONSTRATION\n");
    printf(1, "========================================\n");
    printf(1, "Time slices: Queue 0=5 ticks, Queue 1=10 ticks, Queue 2=20 ticks\n");
    printf(1, "Spawning 5 worker processes...\n");
    printf(1, "========================================\n\n");
    
    // Fork 5 worker processes
    for(i = 0; i < 5; i++) {
        pid = fork();
        if(pid == 0) {
            // Child process - run CPU intensive work
            int j, k;
            
            // CPU intensive loop with minimal output to avoid conflicts
            for(j = 0; j < 200000; j++) {
                // Busy work
                for(k = 0; k < 5000; k++) {
                    // CPU intensive work
                }
                
                // Sleep occasionally to allow other processes to run
                if(j % 50000 == 0) {
                    sleep(1);
                }
            }
            
            exit();
        } else if(pid < 0) {
            printf(1, "Fork failed for worker %d\n", i+1);
        }
    }
    
    printf(1, "All workers spawned! Monitoring MLFQ status...\n\n");
    
    // Parent process - monitor MLFQ status with clear formatting
    for(i = 0; i < 8; i++) {
        printf(1, "--- Time: %d seconds ---\n", i*2);
        mlfqrealtime();  // Use real-time status instead of recorded data
        printf(1, "\n");
        
        // Wait 2 seconds before next check
        sleep(2);
    }
    
    printf(1, "========================================\n");
    printf(1, "    MLFQ DEMONSTRATION COMPLETE\n");
    printf(1, "========================================\n");
    printf(1, "Key Observations:\n");
    printf(1, "• All processes started in Queue 0 (highest priority)\n");
    printf(1, "• CPU-intensive processes demoted to lower queues\n");
    printf(1, "• Higher priority queues run before lower priority queues\n");
    printf(1, "• Time slices: 5→10→20 ticks as processes demote\n");
    printf(1, "========================================\n\n");
    
    exit();
}
