#!/bin/bash
# Grep Verification Script for xv6

echo "=== xv6 Grep Command Verification ==="

# Create test files
echo "Creating test files..."
echo "hello world" > test1.txt
echo "Hello World" > test2.txt
echo -e "line1\nline2\nline3" > test3.txt
echo "start here" > test4.txt
echo "not start" > test5.txt
echo "cat bat hat" > test6.txt
echo "a aa aaa" > test7.txt
echo "this is the end" > test8.txt
echo "not end here" > test9.txt
touch empty.txt

echo "Running grep tests..."

echo "Test 1: Basic text search"
echo "hello world" | ./grep "hello"
echo "Expected: hello world"

echo "Test 2: Case sensitive search"
echo "Hello World" | ./grep "hello"
echo "Expected: No output"

echo "Test 3: Multiple lines"
echo -e "line1\nline2\nline3" | ./grep "line2"
echo "Expected: line2"

echo "Test 4: Beginning of line (^)"
echo -e "start\nnot start" | ./grep "^start"
echo "Expected: start"

echo "Test 5: Any character (.)"
echo "cat bat hat" | ./grep ".at"
echo "Expected: cat bat hat"

echo "Test 6: Zero or more (*)"
echo "a aa aaa" | ./grep "a*"
echo "Expected: a aa aaa"

echo "Test 7: End of line ($)"
echo -e "end\nnot end" | ./grep "end$"
echo "Expected: end"

echo "Test 8: File input"
./grep "hello" test1.txt
echo "Expected: hello world"

echo "Test 9: Multiple files"
./grep "line" test3.txt test3.txt
echo "Expected: line1, line2, line3 (twice)"

echo "Test 10: Empty pattern"
echo "test" | ./grep ""
echo "Expected: test"

echo "Test 11: No matches"
echo "hello" | ./grep "world"
echo "Expected: No output"

echo "Test 12: Empty file"
./grep "anything" empty.txt
echo "Expected: No output"

echo "Test 13: No arguments"
./grep
echo "Expected: usage message"

echo "Test 14: Non-existent file"
./grep "test" nonexistent.txt
echo "Expected: error message"

echo "=== Verification Complete ==="

# Cleanup
rm -f test*.txt empty.txt
