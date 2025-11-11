#include "types.h"
#include "stat.h"
#include "user.h"
#include "procinfo.h"

// ANSI escape codes
#define CLEAR_SCREEN "\033[2J"
#define CURSOR_HOME "\033[H"
#define COLOR_GREEN "\033[32m"
#define COLOR_RED "\033[31m"
#define COLOR_RESET "\033[0m"

const char*
get_state_str_top(enum procstate state)
{
  static const char *states[] = {
    "UNUSED  ",
    "EMBRYO  ",
    "SLEEPING",
    "RUNNABLE",
    "RUNNING ",
    "ZOMBIE  "
  };
  return states[state];
}

int
main(int argc, char *argv[])
{
  struct procinfo processes[NPROC];
  struct cpustats stats, old_stats;
  int iterations = 0;
  int max_iterations = 5; // Run for 20 iterations then exit
  
  // Get initial stats to avoid divide-by-zero
  if(getcpustats(&old_stats) < 0) {
    printf(2, "top: getcpustats failed\n");
    exit();
  }

  while(iterations < max_iterations) {
    iterations++;
    // 1. Clear Screen & Move Cursor to Home
    printf(1, CLEAR_SCREEN CURSOR_HOME);

    // 2. Get Data
    if(getcpustats(&stats) < 0) {
      printf(2, "top: getcpustats failed\n");
      break;
    }
    int count = getallprocinfo(processes, NPROC);
    if(count < 0) {
      printf(2, "top: getallprocinfo failed\n");
      break;
    }

    // 3. Get current scheduler policy
    int policy = getscheduler();
    const char *policy_names[] = {"Round-Robin", "Priority-Based", "MLFQ"};
    
    // 4. Render Header
    printf(1, "xv6-project-top\n");
    printf(1, "Scheduler: %s\n", policy_names[policy]);
    printf(1, "Total Processes: %d\n", count);
    printf(1, "Total Ticks: %d\n\n", stats.total_ticks);
    
    // Display appropriate column headers based on scheduler
    if(policy == 2) { // MLFQ
      printf(1, "PID\tSTATE\t\tMLFQ\tCPU_TICKS\tUPTIME(s)\tNAME\n");
    } else if(policy == 1) { // PBS
      printf(1, "PID\tSTATE\t\tPRI\tCPU_TICKS\tUPTIME(s)\tNAME\n");
    } else { // RR
      printf(1, "PID\tSTATE\t\tCPU_TICKS\tUPTIME(s)\tNAME\n");
    }
    printf(1, "----------------------------------------------------------------\n");

    // 5. Render Process List
    for(int i = 0; i < count; i++){
      // Highlight RUNNING processes in green
      if(processes[i].state == RUNNING)
        printf(1, COLOR_GREEN);

      // Display appropriate columns based on scheduler
      // Calculate uptime safely (handle wraparound)
      int uptime_secs = 0;
      if(stats.total_ticks >= processes[i].start_time) {
        uptime_secs = (stats.total_ticks - processes[i].start_time) / 100;
      }
      
      if(policy == 2) { // MLFQ - show MLFQ level
        printf(1, "%d\t%s\t%d\t%d\t\t%d\t\t%s\n",
          processes[i].pid,
          get_state_str_top(processes[i].state),
          processes[i].mlfq_level,
          processes[i].cpu_ticks,
          uptime_secs,
          processes[i].name
        );
      } else if(policy == 1) { // PBS - show priority
        printf(1, "%d\t%s\t%d\t%d\t\t%d\t\t%s\n",
          processes[i].pid,
          get_state_str_top(processes[i].state),
          processes[i].priority,
          processes[i].cpu_ticks,
          uptime_secs,
          processes[i].name
        );
      } else { // RR - no priority or MLFQ
        printf(1, "%d\t%s\t%d\t\t%d\t\t%s\n",
          processes[i].pid,
          get_state_str_top(processes[i].state),
          processes[i].cpu_ticks,
          uptime_secs,
          processes[i].name
        );
      }
      
      printf(1, COLOR_RESET); // Reset color after each line
    }
    
    // 6. Save current stats for next delta calculation
    old_stats = stats;

    // 7. Sleep for 1 second (100 ticks in xv6)
    sleep(100);
  }

  exit();
}
