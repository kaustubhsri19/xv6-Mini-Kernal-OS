#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
    int i, j;
    
    printf(1, "MLFQ Test 1: CPU-intensive process started (PID: %d)\n", getpid());
    printf(1, "Starting in MLFQ Queue 0 (will demote based on CPU usage)\n");
    
    // CPU intensive loop to consume time slices
    for(i = 0; i < 100000; i++) {
        for(j = 0; j < 1000; j++) {
            // Busy wait to consume CPU time
        }
        if(i % 10000 == 0) {
            printf(1, "MLFQ Test 1 (PID %d): iteration %d\n", getpid(), i);
        }
        // Sleep occasionally to allow other processes to run
        if(i % 25000 == 0) {
            sleep(10);
        }
    }
    
    printf(1, "MLFQ Test 1 (PID %d): completed\n", getpid());
    exit();
}
