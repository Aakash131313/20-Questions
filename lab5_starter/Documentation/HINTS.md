# Implementation Hints

Detailed hints for the trickier TODOs. Try implementing on your own first! 
These hints are intended to provide guidance without showing complete solutions.

## Memory Management Hints

### TODO 1-2: Node Creation
**Key Questions:**
- What's the difference between a question node and an animal node?
- What function can you use to both allocate memory AND copy a string?
- What should the children pointers be initialized to?

**Common Pitfall:** Forgetting to check if `malloc()` returns NULL.

**Hint:** Look up the `strdup()` function - it's your friend! It does `malloc()` + `strcpy()` in one call.

### TODO 3: Free Tree (Recursive)
**Think About:** What happens if you free the parent before the children?

**Critical Order:**
1. What needs to be freed first: parent or children?
2. What needs to be freed first: text or node struct?

**Pattern:** This is a classic post-order tree traversal. What's your base case?

## Stack Implementation Hints

### TODO 6: Stack Push with Resize
**Key Insight:** When the stack is full, you need to grow it dynamically.

**Questions to Consider:**
- How do you check if the stack is full?
- What's a good growth strategy? (Hint: doubling is amortized O(1))
- Which function can resize an existing allocation?

**Common Mistake:** Forgetting that `realloc()` might return a different pointer!

**Skeleton:**
```c
// Check if full
if (/* condition */) {
    // Grow the array
    // Don't forget to update capacity!
}
// Add the new frame
// Update size
```

## Queue Implementation Hints

### TODO 16: Enqueue (Linked List)
**Two Cases to Handle:**
1. Queue is empty (front and rear are both NULL)
2. Queue has items (just add to rear)

**Questions:**
- What should the new node's `next` pointer be?
- When the queue is empty, what should front AND rear point to?
- When the queue isn't empty, which pointer(s) need updating?

**Watch Out:** A queue has TWO ends - don't forget to update both front and rear appropriately!

### TODO 17: Dequeue
**Critical Question:** What happens to `rear` when you dequeue the last item?

**Steps to Think Through:**
1. Save the data BEFORE freeing anything
2. Move the front pointer forward
3. Special case: What if the queue is now empty?
4. Free the old node
5. Update size

**Hint:** If `front` becomes NULL, `rear` should also become NULL.

## Hash Table Hints

### TODO 20: Canonicalize
**Goal:** Convert "Does it meow?" → "does_it_meow"

**Algorithm:**
- Process one character at a time
- Keep letters and numbers (convert to lowercase)
- Replace spaces with underscores
- Skip everything else (punctuation)

**Functions You'll Need:**
- `isalnum()` - checks if alphanumeric
- `isspace()` - checks if whitespace
- `tolower()` - converts to lowercase

**Don't Forget:** Allocate memory for the result and null-terminate!

### TODO 23: Hash Put (Separate Chaining)
**This is the most complex function!** Break it into cases:

**Case 1:** Key already exists in the chain
- Search the chain for matching key
- If found, check if animalId is already in the list
- If not, add it (might need to resize the array)

**Case 2:** Key doesn't exist
- Create a new Entry
- Don't forget to `strdup()` the key!
- Initialize the ID list
- Insert at head of chain (easiest)

**Questions:**
- How do you traverse a linked list?
- When do you know you've found the right Entry?
- How do you insert at the head of a linked list?

### TODO 26: Hash Free
**Challenge:** Each bucket has a linked list - you need to free EVERYTHING.

**Nested Loop Structure:**
- Outer loop: iterate through all buckets
- Inner loop: traverse the chain in each bucket

**For Each Entry, Free:**
1. The key string (it was strdup'd)
2. The ids array
3. The Entry struct itself

**Critical Trick:** Save `next` pointer before freeing the current Entry!

## Game Logic Hints

### TODO 31: Play Game (Iterative Traversal)
**Big Picture:** This replaces recursive tree traversal with explicit stack manipulation.

**The Pattern:**
1. Push root onto stack
2. While stack not empty:
   - Pop a frame
   - If question node: ask, push chosen child
   - If leaf node: guess, possibly learn

**For Learning New Animals:**
- You need to remember the PARENT node - how?
- You need to create TWO new nodes (question + new animal)
- You need to link them based on the answer
- You need to update the parent (or root if at top)
- Don't forget to record this as an Edit!

**Hint About Parent Tracking:**
Before entering the loop, initialize:
```c
Node *parent = NULL;
int parentAnswer = -1;
```

When you visit a question node, update these. When you need to learn, they tell you where to insert!

## Persistence Hints

### TODO 27: Save Tree (BFS with ID Assignment)
**Two-Phase Approach:**

**Phase 1:** Use BFS to assign each node a unique ID (0, 1, 2, ...)
- Create an array to map IDs to Node pointers
- Use your Queue to do BFS
- Assign IDs as you discover nodes

**Phase 2:** Write the file
- Header: magic number, version, count
- For each node in ID order:
  - Write: isQuestion, textLen, text (no null terminator!)
  - Find IDs of children (search your mapping array)
  - Write: yesId, noId (-1 if NULL)

**Questions:**
- Why do we need to assign IDs?
- How do you find the ID for a given Node pointer?
- What data types should you use for the binary format?

### TODO 28: Load Tree
**Inverse of Save:**

**Phase 1:** Read header and allocate arrays
- Read magic number (validate it!)
- Read version and count
- Allocate: nodes array, yesIds array, noIds array

**Phase 2:** Read all nodes
- For each node, read its data
- Create the Node struct
- Store in the nodes array
- **Don't link children yet!** (You don't have all nodes loaded)

**Phase 3:** Link the nodes
- Now that all nodes exist, connect them
- Use the saved ID arrays to set yes/no pointers

**Critical Detail:** Text in the file has NO null terminator. You must add it!

## Utility Hints

### TODO 29: Check Integrity
**Rules to Enforce:**
- Question nodes must have BOTH children (yes and no)
- Leaf nodes must have NO children

**Algorithm:**
- Use BFS to visit every node
- For each node, check the rules
- If any rule is violated, return 0 (invalid)

**Hint:** You can use your Queue for this!

## Undo/Redo Hints

### TODO 32: Undo
**Conceptual Understanding:**
An Edit records: "We replaced oldLeaf with newQuestion at this location"

**To Undo:**
- Reverse it: put oldLeaf BACK at that location
- Save the Edit to the redo stack (don't destroy it!)

**Questions:**
- How do you know if you're updating root vs. a branch?
- How do you know if it was the yes child or no child?

**Important:** Do NOT free the nodes! They might be redone.

### TODO 33: Redo
**This is the mirror of Undo:**
- Take from redo stack
- Reapply the change (put newQuestion back)
- Save to undo stack

## Common Pitfalls and Solutions

### Pitfall 1: Memory Leak in strdup
**Problem:** `strdup()` allocates memory that YOU must free!

Whenever you call:
```c
char *canon = canonicalize(question);
```

You later need:
```c
free(canon);
```

### Pitfall 2: Queue Front/Rear Consistency
**Problem:** When dequeuing the last element, both front AND rear must become NULL.

Test: After dequeue, if `front == NULL`, what should `rear` be?

### Pitfall 3: Hash Table Duplicate Detection
**Problem:** Before adding an animalId to the list, CHECK if it's already there!

Loop through existing IDs first. Only add if not found.

### Pitfall 4: Binary File Text Handling
**Problem:** Binary files don't include null terminators for strings.

When reading:
```c
char *text = malloc(textLen + 1);  // +1 for '\0'
fread(text, 1, textLen, fp);
text[textLen] = '\0';  // YOU add this!
```

When writing:
```c
fwrite(node->text, 1, textLen, fp);  // Don't write '\0'
```

### Pitfall 5: Pointer Update Order
**Problem:** Updating pointers in wrong order can lose data.

Always save what you need BEFORE changing pointers:
```c
// Wrong:
q->front = q->front->next;
free(/* oops, lost reference! */);

// Right:
QueueNode *temp = q->front;
q->front = q->front->next;
free(temp);
```

### Pitfall 6: Parent Tracking in Game Loop
**Problem:** Without tracking the parent, you can't insert new nodes!

**Solution:** Maintain variables outside the loop:
- `Node *parent` - the last question node visited
- `int parentAnswer` - which branch was taken

Update these EVERY time you visit a question node.

## Testing Tips

### Test Each Phase Independently
Build incrementally! After implementing:
- Node functions → compile ds.c
- Stack functions → run stack tests
- Queue functions → run queue tests
- Continue pattern...

Don't try to implement everything then test!

### Add Debug Output
Temporarily add prints:
```c
printf("DEBUG: size=%d, capacity=%d\n", s->size, s->capacity);
```

Remove before submission!

### Use Valgrind Early
```bash
valgrind --leak-check=full ./run_tests
```

Look for:
- "All heap blocks were freed" ✓ GOOD
- "definitely lost: X bytes" ✗ BAD - find and fix!

### Draw Pictures
For tree operations, literally draw the tree structure on paper:
- Before the operation
- After the operation
- What pointers changed?

## Strategy for Success

### Start Simple
Implement in this order (easiest to hardest):
1. Node creation/destruction
2. Stack operations
3. Node counting (easy recursion practice)
4. Queue operations
5. Hash table
6. Game logic (hardest!)
7. Persistence
8. Undo/redo

### Read the Starter Code
Before implementing TODOs:
- Read the header file - understand the data structures
- Look at how structs are defined
- See what's already implemented
- Understand what each TODO needs to return

### Think Before Coding
For each TODO:
1. What are the inputs?
2. What should be returned?
3. What are the edge cases? (NULL? empty? full?)
4. What needs to be allocated/freed?
5. Sketch the algorithm in comments first!

### Test Incrementally
After each TODO:
```bash
make
# Fix compiler errors
./run_tests
# Fix test failures
valgrind ./run_tests
# Fix memory leaks
```

Don't move to the next TODO until the current one works!

## Debugging Strategies

### Segmentation Fault?
**Common Causes:**
- Dereferencing NULL pointer
- Array out of bounds
- Using freed memory
- Stack overflow (infinite recursion)

**How to Find:**
```bash
gdb ./program
run
# After crash:
backtrace  # See where it crashed
print variable_name  # Check values
```

### Logic Error?
**Strategies:**
- Add printf statements to trace execution
- Check your edge cases (empty tree? single node?)
- Draw the data structure state on paper
- Step through with GDB: `break function_name`, then `step`

### Memory Leak?
**Find It:**
```bash
valgrind --leak-check=full --show-leak-kinds=all ./program
```

**Common Sources:**
- Forgot to free strdup'd strings
- Forgot to free malloc'd arrays
- Forgot to free tree nodes
- Forgot to free hash entries

## Algorithm References

If you're stuck, research these concepts:

### Data Structures:
- Dynamic arrays (for Stack)
- Linked lists (for Queue and Hash chains)
- Binary trees (for game tree)
- Hash tables with separate chaining

### Algorithms:
- Tree traversal (pre-order, post-order)
- Breadth-First Search (BFS)
- Depth-First Search (DFS)
- Stack-based iterative tree traversal

### Patterns:
- Producer-consumer (Queue)
- Undo/Redo pattern (Command pattern)
- Serialization/Deserialization
- Two-phase commit (for save/load)

## Final Checklist

Before claiming you're done:

- [ ] Does it compile without warnings? (`gcc -Wall -Wextra`)
- [ ] Do all tests pass? (`./run_tests`)
- [ ] Zero memory leaks? (`valgrind`)
- [ ] Can you play a game successfully?
- [ ] Can you learn multiple animals?
- [ ] Does undo work? Redo?
- [ ] Can you save and reload?
- [ ] Does tree viewer work?
- [ ] Is your code commented?
- [ ] Did you test edge cases?

## When You're Really Stuck

1. **Re-read this file** - carefully!
2. **Check office hours** - TAs are there to help
3. **Talk to classmates** - discuss concepts (not code)
4. **Search documentation** - man pages, C reference
5. **Review lecture notes** - algorithms were covered!

Remember: The struggle is part of learning. If it were easy, you wouldn't learn as much!

**Good luck! Start early, test often, and don't give up!**
