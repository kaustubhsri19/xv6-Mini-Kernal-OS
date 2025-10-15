#!/bin/bash
# MLFQ Test Script
# This script demonstrates MLFQ scheduler behavior

echo "=== MLFQ Scheduler Test ==="
echo "Time slices: Queue 0=5 ticks, Queue 1=10 ticks, Queue 2=20 ticks"
echo ""

echo "1. Initial state (only init process):"
mlfqstatus
echo ""

echo "2. Starting 3 MLFQ test processes..."
mlfqtest1 &
mlfqtest2 &
mlfqtest3 &
echo ""

echo "3. MLFQ status after starting processes:"
sleep 1
mlfqstatus
echo ""

echo "4. Starting additional CPU-intensive processes..."
stressfs &
wc README &
echo ""

echo "5. MLFQ status with more processes:"
sleep 1
mlfqstatus
echo ""

echo "6. Running forktest to create more processes..."
forktest &
echo ""

echo "7. Final MLFQ status:"
sleep 2
mlfqstatus
echo ""

echo "=== Test Complete ==="
echo "Processes should distribute across queues as they consume time slices"
echo "New processes start in Queue 0, then move to Queue 1, then Queue 2"
