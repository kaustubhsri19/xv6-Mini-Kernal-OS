#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"
#include "proc.h"
#include "x86.h"

struct lockgraph {
    int waitfor[NPROC][NPROC];
} lgraph;

int detect_deadlock(void){
    // TODO: Implement DFS-based cycle detection later
    // Placeholder return
    return 0;
}
