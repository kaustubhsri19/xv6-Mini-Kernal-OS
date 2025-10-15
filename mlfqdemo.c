#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
    int i;
    
    printf(1, "MLFQ Demo: Starting long-running process (PID: %d)\n", getpid());
    
    // Run for a long time with periodic sleeps to show MLFQ behavior
    for(i = 0; i < 20; i++) {
        printf(1, "MLFQ Demo: Step %d - Process should move through queues\n", i);
        
        // Busy work for a bit
        int j;
        for(j = 0; j < 1000000; j++) {
            // CPU intensive work
        }
        
        // Sleep briefly to let other processes run and show MLFQ behavior
        sleep(2);
    }
    
    printf(1, "MLFQ Demo: Process completed\n");
    exit();
}
