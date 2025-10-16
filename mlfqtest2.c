#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
    int i, j;
    
    printf(1, "MLFQ Test 2: MEDIUM PRIORITY process started (PID: %d)\n", getpid());
    
    // Set this process to medium priority (2)
    setpriority(2);
    printf(1, "Set priority to 2 (medium)\n");
    
    // CPU intensive loop to consume time slices
    for(i = 0; i < 75000; i++) {
        for(j = 0; j < 8000; j++) {
            // Busy wait to consume CPU time
        }
        if(i % 7500 == 0) {
            printf(1, "MLFQ Test 2 (PID %d): iteration %d\n", getpid(), i);
        }
        // Sleep occasionally to allow other processes to run
        if(i % 15000 == 0) {
            sleep(1);
        }
    }
    
    printf(1, "MLFQ Test 2 (PID %d): completed\n", getpid());
    exit();
}
