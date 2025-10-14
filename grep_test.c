#include "types.h"
#include "stat.h"
#include "user.h"

// Test the regex matching functions
int match(char*, char*);
int matchhere(char*, char*);
int matchstar(int, char*, char*);

void test_regex() {
    printf(1, "Testing regex patterns...\n");
    
    // Test basic matching
    if(match("hello", "hello world")) {
        printf(1, "✓ Basic match works\n");
    } else {
        printf(1, "✗ Basic match failed\n");
    }
    
    // Test beginning of line
    if(match("^start", "start here")) {
        printf(1, "✓ Beginning match works\n");
    } else {
        printf(1, "✗ Beginning match failed\n");
    }
    
    // Test any character
    if(match(".at", "cat")) {
        printf(1, "✓ Any character match works\n");
    } else {
        printf(1, "✗ Any character match failed\n");
    }
    
    // Test zero or more
    if(match("a*", "aaa")) {
        printf(1, "✓ Zero or more match works\n");
    } else {
        printf(1, "✗ Zero or more match failed\n");
    }
    
    // Test end of line
    if(match("end$", "this is the end")) {
        printf(1, "✓ End match works\n");
    } else {
        printf(1, "✗ End match failed\n");
    }
    
    printf(1, "Regex testing complete.\n");
}

int main() {
    test_regex();
    exit();
}
