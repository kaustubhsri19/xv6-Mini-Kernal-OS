#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
    printf(1, "=== MLFQ Status ===\n");
    mlfqstatus();   // system call to kernel
    exit();
}
