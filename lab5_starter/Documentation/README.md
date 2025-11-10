# ECE 312 - Lab 5: 20 Questions - Guess the Animal

## Overview

You will build a complete interactive game that learns! This is a terminal-based implementation of the classic "20 Questions" game with a twist: when the computer guesses wrong, it asks you to teach it about the new animal and adds the information to its decision tree.


**Features:**
- 🎯 Interactive guessing game
- 🧠 Learns from mistakes
- 💾 Saves/loads tree to binary file
- ↩️  Undo/redo edits
- 🌳 Visual tree explorer with scrolling
- 🔍 Tree integrity checking
- ⚡ **No recursion** in gameplay (uses explicit stacks!)

---


### What Makes This Lab Special

**Learning AI**: The game starts with minimal knowledge (just 3 animals) but grows smarter as you play. Each time it fails to guess your animal, it learns by asking you for a distinguishing question and adding both the question and animal to its decision tree.

**Advanced Data Structures**: You'll implement multiple data structures from scratch in C:
- **Binary Decision Tree** - Stores questions (internal nodes) and animals (leaves)
- **Stacks** - For iterative tree traversal and undo/redo operations
- **Queue** - For breadth-first traversal during save/load
- **Hash Table** - For indexing attributes and potential query optimization

**No Recursion for Gameplay**: Unlike typical tree problems, you'll use explicit stacks instead of recursion for the game loop. This teaches you how compilers convert recursive calls to iterative code.

**Persistent Storage**: Your tree saves to a compact binary format using BFS traversal, so the game "remembers" everything it learned across sessions.

**Undo/Redo System**: Made a mistake teaching the game? No problem! Full undo/redo support using dual stacks, just like a text editor.

### Example Game Session

```
Think of an animal, and I'll try to guess it!

Does it live in water? (y/n): n
Is it a Dog? (y/n): n

What animal were you thinking of? Cat
Give me a yes/no question to distinguish Cat from Dog: Does it meow?
For Cat, what is the answer? (y/n): y

Thanks! I'll remember that.

[Play again...]

Does it live in water? (y/n): n
Does it meow? (y/n): y
Is it a Cat? (y/n): y

Yay! I guessed it!
```

### Tree Growth Visualization

```
Initial tree:
       "Does it live in water?"
        /                    \
     "Fish"                "Dog"

After learning about Cat:
       "Does it live in water?"
        /                    \
     "Fish"          "Does it meow?"
                      /            \
                   "Cat"          "Dog"

After learning about Bird:
       "Does it live in water?"
        /                    \
     "Fish"          "Does it meow?"
                      /            \
                   "Cat"      "Does it fly?"
                               /           \
                            "Bird"        "Dog"
```

### What You'll Learn

By completing this lab, you will:
- ✅ Improve your skills in manual memory management in C (malloc/free)
- ✅ Implement iterative algorithms using explicit stacks
- ✅ Build multiple data structures from scratch
- ✅ Work with binary file I/O and serialization
- ✅ Handle complex pointer relationships
- ✅ Debug with valgrind and gdb
- ✅ Design undo/redo systems
- ✅ Manage a multi-week project

### Project Scope

**Time Required**: 33-49 hours total
- Week 1: Data structures (20-25 hours)
- Week 2: Game logic and persistence (8-12 hours)
- Week 3: Testing and polish (5-12 hours)

**Lines of Code**: You'll write 370-530 lines across 4 files
**TODOs**: 33 total (32 required + 1 optional challenge)

**Deliverables**:
1. Complete C implementation with all TODOs
2. Write-up (1-2 pages) analyzing design and complexity
3. All unit tests passing
4. Zero memory leaks (verified with valgrind or memory sanitizer)

---

## Quick Start (5 Minutes)

```bash
# 1. Build and run
cd src 
make
make run
# Press 'p' to play - you'll see an error (expected!)
# Press 'q' to quit

# 2. Run tests
make test
# Tests will fail - that's normal, you haven't implemented anything yet!

# 3. Start implementing
# Open ds.c and find TODO 1
```

**Why the error?** The UI works, but you need to implement node creation (TODOs 1-2) and uncomment code in `main.c` `initialize_tree()` before the tree works.

---


## Implementation Plan

### Phase 1: Core Data Structures (Week 1)

#### TODOs 1-4: Tree Nodes (~30-60 min)
```c
// In ds.c
create_question_node()  // Malloc node, strdup text, isQuestion=1
create_animal_node()    // Similar but isQuestion=0
free_tree()            // Recursive: free children, then self
count_nodes()          // Recursive: count all nodes
```

**⚠️ CRITICAL:** After TODOs 1-2, uncomment code in `main.c` `initialize_tree()` function!

**Test:** `make test` - node tests should pass

#### TODOs 5-9: Frame Stack (~1-2 hours)
Stack for iterative tree traversal. Dynamic array that doubles when full.
```c
fs_init()   // Malloc array, capacity=16
fs_push()   // Resize if needed, add frame
fs_pop()    // Return frames[--size]
fs_empty()  // Return size == 0
fs_free()   // Free array
```

**Test:** `make test` - stack tests should pass

#### TODOs 15-19: Queue (~1-2 hours)
Linked list for BFS traversal.
```c
q_init()      // front=NULL, rear=NULL
q_enqueue()   // Malloc node, link at rear
q_dequeue()   // Remove from front, update rear if empty!
q_empty()     // Return front == NULL
q_free()      // Dequeue all
```

**Test:** `make test` - queue tests should pass

#### TODOs 20-26: Hash Table (~3-5 hours)
Separate chaining for attribute indexing.
```c
canonicalize()  // "Does it meow?" → "does_it_meow"
h_hash()        // djb2: hash = ((hash << 5) + hash) + c
h_init()        // Calloc buckets
h_put()         // Search chain, add to list or create entry
h_contains()    // Search for key-value pair
h_get_ids()     // Return all IDs for key
h_free()        // Free chains, keys, arrays
```

**Test:** `make test` - hash tests should pass

### Phase 2: Game Logic (Week 2)

#### TODOs 10-14: Edit Stack (~1 hour)
Similar to Frame Stack but for Edit structs.

#### TODOs 32-33: Undo/Redo (~1-2 hours)
```c
undo_last_edit()  // Pop from g_undo, restore pointers, push to g_redo
redo_last_edit()  // Pop from g_redo, reapply, push to g_undo
```

#### TODO 31: Game Loop (~3-5 hours) ⭐ **HARDEST**
Iterative traversal using explicit stack. Learning phase creates new nodes and records edits.

**Key steps:**
1. Push root frame
2. While stack not empty:
   - Pop frame
   - If question: ask, push child based on answer
   - If leaf: guess, or learn if wrong
3. Learning: get animal/question, create nodes, update tree, record edit

**Test:** `make run` and play!

### Phase 3: Persistence (Week 2-3)

#### TODOs 27-28: Binary I/O (~3-5 hours)
Use BFS to serialize tree with node IDs.

**save_tree():**
1. BFS to assign IDs (0, 1, 2, ...)
2. Write header (magic, version, count)
3. Write each node (isQuestion, textLen, text, yesId, noId)

**load_tree():**
1. Read header, validate
2. Read all nodes into array
3. Link using stored IDs
4. Set g_root = nodes[0]

**Test:** `make test` - persistence tests should pass

#### TODO 29: Integrity Checker (~30-60 min)
BFS to verify: questions have 2 children, leaves have 0 children.

#### TODO 30: Shortest Path (OPTIONAL)
Extra credit challenge!

---

## Testing Strategy

### After Each TODO
```bash
make test  # See which tests pass
```

### Full Testing Cycle
```bash
# 1. Unit tests
make clean && make test

# 2. Memory leaks
make valgrind-test

# 3. Interactive
make run
# Test: play, learn, undo, redo, save, load, integrity

# 4. Final check
make valgrind  # Run full program, should show no leaks
```

---

## Common Mistakes & Solutions

### Memory Management
| ❌ Wrong | ✅ Correct |
|---------|-----------|
| `malloc(sizeof(Node*))` | `malloc(sizeof(Node))` |
| `free(node); free(node->text);` | `free(node->text); free(node);` |
| `node->text = question;` | `node->text = strdup(question);` |
| `malloc(textLen)` | `malloc(textLen + 1)` for strings |

### Stack/Queue
| ❌ Wrong | ✅ Correct |
|---------|-----------|
| `if (size > capacity)` | `if (size >= capacity)` |
| `q->front = NULL;` when empty | `q->front = NULL; q->rear = NULL;` |

### Hash Table
| ❌ Wrong | ✅ Correct |
|---------|-----------|
| `entry->key = key;` | `entry->key = strdup(key);` |

---

## Key Implementation Hints

### Node Creation (TODO 1-2)
```c
Node *n = malloc(sizeof(Node));
n->text = strdup(text);  // Allocates memory!
n->yes = n->no = NULL;
n->isQuestion = 1;  // or 0 for animals
return n;
```

### Free Tree (TODO 3)
```c
void free_tree(Node *node) {
    if (!node) return;
    free_tree(node->yes);   // Children first!
    free_tree(node->no);
    free(node->text);       // Then text
    free(node);            // Then node
}
```

### Stack Push with Resize (TODO 6)
```c
if (s->size >= s->capacity) {
    s->capacity *= 2;
    s->frames = realloc(s->frames, s->capacity * sizeof(Frame));
}
s->frames[s->size++] = (Frame){node, answeredYes};
```

### Queue Dequeue (TODO 17)
```c
QueueNode *temp = q->front;
*node = temp->treeNode;
*id = temp->id;
q->front = q->front->next;
if (!q->front) q->rear = NULL;  // Empty queue!
free(temp);
```

### Hash Put (TODO 23)
```c
// 1. Search chain for existing key
// 2. If found: check for duplicate ID, add if new
// 3. If not found: create entry with strdup(key), insert at head
```

For complete hints, see HINTS.md.

---

## Build Commands

```bash
make            # Build main program
make test       # Build and run tests
make run        # Run the game
make valgrind   # Check for memory leaks
make clean      # Remove build files
make help       # Show all targets
```

---

## Debugging

### Segfault?
```bash
gdb ./guess_animal
(gdb) run
(gdb) bt  # Backtrace shows where it crashed
```

### Memory Leak?
```bash
valgrind --leak-check=full --track-origins=yes ./run_tests
# Look for "definitely lost" - those are real leaks
```

### Test Failure?
```bash
# Read the assertion that failed
# Check line number in tests.c
# Add printf to see what's happening
```

---

## File Organization

### You Edit These:
- **ds.c** - All data structures (26 TODOs)
- **game.c** - Game logic and undo/redo (3 TODOs)
- **persist.c** - Save/load (2 TODOs)
- **utils.c** - Integrity checker (2 TODOs)

### Provided (Don't Edit):
- **lab5.h** - All type definitions
- **main.c** - UI (only uncomment initialize_tree after TODOs 1-2!)
- **tests.c** - Unit tests
- **Makefile** - Build system

### Documentation:
- **README.md** - This file (complete guide)
- **HINTS.md** - Detailed implementation hints
- **BUILD.md** - Full build and debugging reference

---

## Grading (100 points)

- **Correctness (50%)**: All tests pass, game works, undo/redo, save/load
- **Code Quality (20%)**: Clean code, good names, comments
- **Memory Safety (20%)**: No leaks, proper malloc/free
- **Write-up (10%)**: Design choices, complexity analysis

**Submission Checklist:**
- [ ] All 32 required TODOs implemented
- [ ] `make test` passes
- [ ] `make valgrind` shows no leaks
- [ ] Game works (play, learn, undo, redo, save, load)
- [ ] Code compiles without warnings
- [ ] Write-up included

---

## Time Estimate

- Data structures: 15-20 hours
- Game logic: 6-10 hours
- Persistence: 4-6 hours
- Testing/debugging: 6-10 hours
- Write-up: 2-3 hours
- **Total: 33-49 hours**

**Start early!** This is not a weekend project.

---

## FAQ

**Q: Can I use recursion?**  
A: Only for `free_tree()` and `count_nodes()`. Everything else must be iterative!

**Q: How do I know if my implementation is correct?**  
A: Run `make test` after each TODO. Tests verify correctness.

**Q: Should I implement TODO 30 (shortest path)?**  
A: It's optional. Focus on required TODOs first.

**Q: What if valgrind shows "still reachable"?**  
A: Usually okay (ncurses cleanup). Focus on "definitely lost".

**Q: Can I add helper functions?**  
A: Yes! Add static helpers in .c files.

**Q: Tests pass but game crashes?**  
A: Game loop is complex. Use gdb to find where it crashes.

---

## Getting Help

1. **Read TODO comments** in the code
2. **Check HINTS.md** for implementation details
3. **Look at tests.c** to see expected behavior
4. **Try printf debugging** or gdb
5. **Ask on Ed/Piazza** or office hours

Include: TODO number, error message, what you tried, relevant code snippet.

---

## Quick Reference

### Memory Functions
```c
malloc(size)          // Allocate
free(ptr)             // Free
realloc(ptr, size)    // Resize
strdup(s)             // Duplicate string (allocates!)
```

### String Functions
```c
strlen(s)             // Length
strcmp(s1, s2)        // Compare (0 = equal)
strcpy(dest, src)     // Copy
```

### File I/O
```c
fopen(file, "rb")     // Open for reading binary
fopen(file, "wb")     // Open for writing binary
fread(buf, 1, n, fp)  // Read n bytes
fwrite(buf, 1, n, fp) // Write n bytes
fclose(fp)            // Close
```

### Character Functions
```c
isalnum(c)            // Alphanumeric?
isspace(c)            // Whitespace?
tolower(c)            // To lowercase
```

---

## Success Tips

1. **Test after every TODO** - Catch bugs early
2. **Use valgrind frequently** - Don't wait for the end
3. **Draw diagrams** - Visualize pointers and structures
4. **Start with TODOs 1-2** - Everything builds on nodes
5. **Don't forget to uncomment** main.c after TODOs 1-2!

---

**Ready to start? Open `ds.c` and implement TODO 1!** 🚀

*Questions? Check BUILD.md or HINTS.md for more details.*
