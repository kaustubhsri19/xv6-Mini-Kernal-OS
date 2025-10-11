#include "types.h"
#include "stat.h"
#include "user.h"

int main(void){
    printf(1, "Running deadlock detection (stub)...\n");
    int result = detect_deadlock();
    printf(1, "Deadlock detected: %s\n", result ? "YES" : "NO");
    exit();
}
