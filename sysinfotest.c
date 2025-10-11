#include "types.h"
#include "stat.h"
#include "user.h"

struct sysinfo {
    int nproc;
    uint ticks;
};

int main(void) {
    struct sysinfo info;
    if(getsysinfo(&info) < 0){
        printf(1, "sysinfotest: failed\n");
        exit();
    }
    printf(1, "Running processes: %d, Uptime: %d ticks\n", info.nproc, info.ticks);
    exit();
}
