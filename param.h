#define NPROC        64  // maximum number of processes
#define KSTACKSIZE 4096  // size of per-process kernel stack
#define NCPU          8  // maximum number of CPUs
#define NOFILE       16  // open files per process
#define NFILE       100  // open files per system
#define NINODE       50  // maximum number of active i-nodes
#define NDEV         10  // maximum major device number
#define ROOTDEV       1  // device number of file system root disk
#define MAXARG       32  // max exec arguments
#define MAXOPBLOCKS  10  // max # of blocks any FS op writes
#define LOGSIZE      (MAXOPBLOCKS*3)  // max data blocks in on-disk log
#define NBUF         (MAXOPBLOCKS*3)  // size of disk block cache
#define FSSIZE       2500  // size of file system in blocks (increased to fit all programs)

// MLFQ constants
#define NQUEUE 3
#define TIME_SLICE_0 5
#define TIME_SLICE_1 10  
#define TIME_SLICE_2 20
#define BOOST_INTERVAL_TICKS 1000 // Priority boost every 500 ticks (5 seconds)
#define MAX_MLFQ_SNAPSHOTS 1000  // Increased from 200 to 1000

