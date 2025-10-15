#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
    int i, j;
    
    printf(1, "MLFQ Test 3: CPU intensive process started (PID: %d)\n", getpid());
    
    // CPU intensive loop to consume time slices
    for(i = 0; i < 100000; i++) {
        for(j = 0; j < 5000; j++) {
            // Busy wait to consume CPU time
        }
        if(i % 10000 == 0) {
            printf(1, "MLFQ Test 3 (PID %d): iteration %d\n", getpid(), i);
        }
        // Sleep occasionally to allow other processes to run
        if(i % 20000 == 0) {
            sleep(1);
        }
    }
    
    printf(1, "MLFQ Test 3 (PID %d): completed\n", getpid());
    exit();
}
