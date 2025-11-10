# Build and Debugging Reference

Complete reference for building, testing, and debugging. See README.md for implementation guide.

## Prerequisites & Installation

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential libncurses5-dev libncursesw5-dev valgrind
```

### macOS
If you don't have Homebrew, do this first: 
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

```bash
brew install gcc make ncurses valgrind
```

### Windows (WSL)
Use Windows Subsystem for Linux, then follow Ubuntu instructions.

---

## Make Targets

```bash
make              # Build main program (default)
make all          # Same as make
make tests        # Build test suite
make test         # Build and run tests
make run          # Build and run game
make clean        # Remove all build files
make valgrind     # Run game with memory leak detection
make valgrind-test # Run tests with memory leak detection
make help         # Show all targets
```

---

## Testing Workflow

### 1. Unit Testing (After Each TODO)
```bash
make test
```

**Expected progression:**
```
After TODOs 1-4:  ✓ Node tests passed
After TODOs 5-9:  ✓ Stack tests passed
After TODOs 10-14: ✓ Edit stack tests passed
After TODOs 15-19: ✓ Queue tests passed
After TODO 20-21:  ✓ Canonicalization tests passed
After TODOs 22-26: ✓ Hash table tests passed
After TODOs 27-28: ✓ Persistence tests passed
After TODO 29:     ✓ Integrity tests passed
```

### 2. Memory Leak Testing
```bash
make valgrind-test
```

**Good output:**
```
HEAP SUMMARY:
    in use at exit: 0 bytes in 0 blocks
  total heap usage: 247 allocs, 247 frees, 12,456 bytes allocated

All heap blocks were freed -- no leaks are possible
ERROR SUMMARY: 0 errors from 0 contexts
```

**Bad output (example):**
```
HEAP SUMMARY:
    in use at exit: 512 bytes in 8 blocks
  total heap usage: 150 allocs, 142 frees, 8,456 bytes allocated

512 bytes in 8 blocks are definitely lost
```

### 3. Interactive Testing
```bash
make run
```

**Test scenarios:**
1. Correct guess (answer questions, it guesses right)
2. Wrong guess → learn new animal
3. Undo the learning (press 'u')
4. Redo the learning (press 'r')
5. Save tree (press 's')
6. Quit and reload (press 'q', then `make run`, press 'l')
7. Check integrity (press 'i')

### 4. Final Valgrind Check
```bash
make valgrind
# Play briefly, save, quit
# Check output for leaks
```

---

## Understanding Test Failures

### Failed Test Example
```
Testing Frame Stack...
run_tests: tests.c:25: test_stack: Assertion `s.size == 2' failed.
Aborted (core dumped)
```

**What it means:**
- Test failed at line 25 in tests.c
- Expected `s.size` to be 2, but it wasn't
- Check your `fs_push()` implementation
- Look at tests.c line 25 to see what it's testing

### Common Test Patterns
```c
assert(fs_empty(&s));          // Should be empty
assert(s.size == 2);           // Should have 2 elements
assert(f.node == &dummy1);     // Pointers should match
assert(!q_empty(&q));          // Should NOT be empty
```

---

## Common Build Errors

### Error: "undefined reference to `function_name`"
```
/usr/bin/ld: game.o: undefined reference to `fs_push'
```
**Solution:** You haven't implemented `fs_push()` yet, or there's a typo.

### Error: "lab5.h: No such file or directory"
```
ds.c:5:10: fatal error: lab5.h: No such file or directory
```
**Solution:** Make sure you're in the correct directory. Run `ls` and verify lab5.h exists.

### Error: "ncurses.h: No such file or directory"
```
main.c:4:10: fatal error: ncurses.h: No such file or directory
```
**Solution:** Install ncurses library (see Prerequisites above).

### Warning: "unused variable"
```
ds.c:45:9: warning: unused variable 'i' [-Wunused-variable]
```
**Solution:** Remove unused variables or use them. Fix all warnings before submission.

### Warning: "implicit declaration of function"
```
ds.c:67:5: warning: implicit declaration of function 'strdup'
```
**Solution:** Add `#include <string.h>` at the top of the file.

---

## Debugging with GDB

### Basic GDB Session
```bash
gdb ./run_tests
(gdb) run                    # Start program
(gdb) bt                     # Backtrace (shows where it crashed)
(gdb) quit                   # Exit
```

### Useful GDB Commands
```
break ds.c:45          # Set breakpoint at line 45
break fs_push          # Set breakpoint at function
run                    # Start program
continue               # Continue after breakpoint
step                   # Step into function
next                   # Step over function
print variable         # Print variable value
print *node            # Print struct contents
info locals            # Show local variables
```

### Example Debugging Session
```bash
$ gdb ./run_tests
(gdb) break test_stack
Breakpoint 1 at 0x1234: file tests.c, line 15.
(gdb) run
Starting program: ./run_tests

Breakpoint 1, test_stack () at tests.c:15
15          fs_init(&s);
(gdb) next
16          assert(fs_empty(&s));
(gdb) print s.size
$1 = 0
(gdb) print s.capacity
$2 = 16
(gdb) continue
```

### Conditional Breakpoints
```
break fs_push if s->size > 10
# Only stops when condition is true
```

### Watchpoints
```
watch s.size
# Stops whenever s.size changes
```

---

## Debugging with Valgrind

### Basic Usage
```bash
valgrind ./run_tests
```

### Detailed Options
```bash
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         ./run_tests
```

**What each option does:**
- `--leak-check=full`: Detailed leak information
- `--show-leak-kinds=all`: Show all types of leaks
- `--track-origins=yes`: Track where uninitialized values come from
- `--verbose`: Extra debugging information

### Interpreting Valgrind Output

#### Definitely Lost (BAD!)
```
512 bytes in 8 blocks are definitely lost
   at malloc (vg_replace_malloc.c:309)
   by create_question_node (ds.c:15)
```
**Meaning:** You malloc'd but never freed. Fix: Call free somewhere.

#### Still Reachable (Usually OK)
```
1,024 bytes in 4 blocks are still reachable
```
**Meaning:** Memory not freed but still referenced. Often from library cleanup (ncurses). Usually safe to ignore.

#### Invalid Read/Write (BAD!)
```
Invalid write of size 4
   at fs_push (ds.c:45)
```
**Meaning:** Buffer overflow or use-after-free. Fix: Check array bounds and don't use freed memory.

#### Uninitialized Value (BAD!)
```
Conditional jump or move depends on uninitialised value(s)
   at q_empty (ds.c:89)
```
**Meaning:** Using variable before setting it. Fix: Initialize all variables.

---

## Printf Debugging

### Adding Debug Prints
```c
void fs_push(FrameStack *s, Node *node, int answeredYes) {
    printf("DEBUG: fs_push called, size=%d, capacity=%d\n", 
           s->size, s->capacity);
    
    if (s->size >= s->capacity) {
        printf("DEBUG: Resizing from %d to %d\n", 
               s->capacity, s->capacity * 2);
        s->capacity *= 2;
        s->frames = realloc(s->frames, s->capacity * sizeof(Frame));
    }
    
    // ... rest of function
}
```

**Remember:** Remove debug prints before submission!

### Printing Structures
```c
void debug_print_node(Node *n) {
    if (!n) {
        printf("Node: NULL\n");
        return;
    }
    printf("Node: %s, isQuestion=%d, yes=%p, no=%p\n",
           n->text, n->isQuestion, n->yes, n->no);
}
```

---

## Testing Individual Functions

### Option 1: Modify tests.c
```c
int main() {
    printf("\n=== Running Unit Tests ===\n\n");
    
    // Comment out tests you don't want
    // test_nodes();
    test_stack();  // Only run this one
    // test_queue();
    
    return 0;
}
```

### Option 2: Write Mini Test
```c
// test_my_function.c
#include "lab5.h"
#include <stdio.h>

int main() {
    FrameStack s;
    fs_init(&s);
    
    printf("Empty? %d (expected: 1)\n", fs_empty(&s));
    
    Node dummy = {0};
    fs_push(&s, &dummy, 1);
    
    printf("Size: %d (expected: 1)\n", s.size);
    printf("Empty? %d (expected: 0)\n", fs_empty(&s));
    
    Frame f = fs_pop(&s);
    printf("Node matches? %d (expected: 1)\n", f.node == &dummy);
    
    fs_free(&s);
    return 0;
}
```

Compile and run:
```bash
gcc -g test_my_function.c ds.c -o test_my_function
./test_my_function
```

---

## Core Dump Analysis

If program crashes with "Segmentation fault (core dumped)":

```bash
# Enable core dumps (if needed)
ulimit -c unlimited

# Run program (crashes)
./run_tests

# Analyze core dump
gdb ./run_tests core

# In gdb
(gdb) bt        # See where it crashed
(gdb) frame 3   # Jump to frame 3
(gdb) print var # Print variables at crash site
```

---

## Memory Debugging Checklist

When you have memory issues:

### Segmentation Faults
- [ ] NULL pointer dereference?
- [ ] Array out of bounds?
- [ ] Use-after-free (accessing freed memory)?
- [ ] Stack overflow (too much recursion)?
- [ ] Uninitialized pointer?

### Memory Leaks
- [ ] Every malloc has a matching free?
- [ ] Freed all text strings in nodes?
- [ ] Freed all hash table keys?
- [ ] Freed all queue nodes?
- [ ] Freed stack/queue arrays?

### Logic Errors
- [ ] Off-by-one errors in loops?
- [ ] Incorrect pointer updates?
- [ ] Wrong comparison (= vs ==)?
- [ ] Missing break in switch?

---

## Performance Profiling

### Timing Your Code
```bash
time ./run_tests
# Shows: real (wall clock), user (CPU), sys (system) time
```

### If Tests Are Slow
- Check for infinite loops
- Check for O(n²) operations
- Profile with:
```bash
valgrind --tool=callgrind ./run_tests
# Generates callgrind.out.PID
# Analyze with kcachegrind
```

---

## Submission Checklist

Before submitting, verify:

```bash
# 1. Clean build
make clean
make

# 2. All tests pass
make test
# Should see: === All Tests Passed! ===

# 3. No memory leaks
make valgrind-test
# Should see: All heap blocks were freed

# 4. No compiler warnings
make clean
make 2>&1 | grep -i warning
# Should be empty or minimal

# 5. Interactive testing
make run
# Test: play, learn, undo, redo, save, load, integrity

# 6. Final memory check
make valgrind
# Play briefly, quit, check for leaks

# 7. Verify all files present
ls *.c *.h Makefile
# Should see all required files
```

---

## Quick Command Reference

```bash
# Development cycle
make clean && make test          # Build and test
make clean && make valgrind-test # Check leaks

# Find TODOs
grep -n "TODO" *.c

# Count your code
wc -l ds.c game.c persist.c utils.c

# Check for common mistakes
grep "malloc(sizeof(" *.c       # Look for sizeof(Type*)
grep "free.*free" *.c           # Look for double frees

# Watch a file for changes (useful with auto-compile)
watch -n 1 make test
```

---

## Additional Resources

- **Man pages**: `man malloc`, `man strcmp`, `man fwrite`, etc.
- **GDB tutorial**: https://www.cs.cmu.edu/~gilpin/tutorial/
- **Valgrind manual**: https://valgrind.org/docs/manual/quick-start.html
- **C reference**: https://en.cppreference.com/w/c

---

*For implementation details, see README.md. For hints, see HINTS.md.*
