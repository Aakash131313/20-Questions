#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lab5.h"

static int edit_is_applied(const Edit *e) {
    if (e->newQuestion == NULL) return 0; //if no new node, it just returns nothing
    if (e->parent == NULL) return g_root == e->newQuestion; //compares it with the root if there is no parent
    if (e->wasYesChild)    return e->parent->yes == e->newQuestion; //if it was a yes side child, compares it with that
    return e->parent->no  == e->newQuestion; //all other cases covered so compares no-child
}

static void free_detached_edit(Edit *e) {
    // frees if it is detached from an undo - kept getting valgrind errors
    if (!edit_is_applied(e)) {
        // if newQuestion was detached by undo
        if (e->newQuestion) { //frees the newQuestion subtree
            // its other child (newLeaf) is ours to free explicitly
            if (e->newLeaf) { //frees the new created leaf
                if (e->newLeaf->text) free(e->newLeaf->text);
                free(e->newLeaf); //frees every edited part of new leaf and the the leaf node itself
                e->newLeaf = NULL;
            }
            if (e->newQuestion->text) free(e->newQuestion->text); //goes back and frees the question
            free(e->newQuestion);
            e->newQuestion = NULL;
        }
    }
}

/* ========== Node Functions ========== */

/* TODO 1: Implement create_question_node
 * - Allocate memory for a Node structure
 * - Use strdup() to copy the question string (heap allocation)
 * - Set isQuestion to 1
 * - Initialize yes and no pointers to NULL
 * - Return the new node
 */
Node *create_question_node(const char *question) {
    // TODO: Implement this function
    if (!question) return NULL;  //First check to see if valid question
    Node *n = (Node *)malloc(sizeof(Node)); //allocate size for a node
    if (!n) return NULL; //if malloc fails, returns NULL
    n->text = strdup(question); //copies question into the text of the node
    if (!n->text) { //if text ptr doesn't exist it frees the node to avoid errors in empty pointers
        free(n);
        return NULL;
    }
    n->isQuestion = 1; //sets the node to question mode
    n->yes = NULL; //initialize yes and no ptrs and returns the node
    n->no = NULL;
    return n;
}

/* TODO 2: Implement create_animal_node
 * - Similar to create_question_node but set isQuestion to 0
 * - This represents a leaf node with an animal name
 */
Node *create_animal_node(const char *animal) {
    // TODO: Implement this function
    if (!animal) return NULL;
    Node *n = (Node *)malloc(sizeof(Node)); //allocates memory
    if (!n) return NULL;
    n->text = strdup(animal); //puts the animal input into the text of the node
    if (!n->text) {
        free(n); //if strdup fails it frees it so nomemory leaks
        return NULL;
    }
    n->isQuestion = 0; //sets it to animal node
    n->yes = NULL; //initializes its children
    n->no = NULL;
    return n;
}

/* TODO 3: Implement free_tree (recursive)
 * - This is one of the few recursive functions allowed
 * - Base case: if node is NULL, return
 * - Recursively free left subtree (yes)
 * - Recursively free right subtree (no)
 * - Free the text string
 * - Free the node itself
 * IMPORTANT: Free children before freeing the parent!
 */
void free_tree(Node *node) {
    // TODO: Implement this function
    if (!node) return; //base case if node doesn't exist
    free_tree(node->yes); //frees children, then the text, then the node itself
    free_tree(node->no);
    free(node->text);
    free(node);
}

/* TODO 4: Implement count_nodes (recursive)
 * - Base case: if root is NULL, return 0
 * - Return 1 + count of left subtree + count of right subtree
 */
int count_nodes(Node *root) {
    // TODO: Implement this function
    //recursively counts the nodes going down the yes first and then the nos
    if (!root) return 0;
    return 1 + count_nodes(root->yes) + count_nodes(root->no);
    return 0;
}

/* ========== Frame Stack (for iterative tree traversal) ========== */

/* TODO 5: Implement fs_init
 * - Allocate initial array of frames (start with capacity 16)
 * - Set size to 0
 * - Set capacity to 16
 */
void fs_init(FrameStack *s) {
    // TODO: Implement this function
    s->capacity = 16; //initializes capacity to 16
    s->size = 0; //initializes to empty stack
    s->frames = (Frame *)malloc(sizeof(Frame) * s->capacity); //allocates memory for the stack
}

/* TODO 6: Implement fs_push
 * - Check if size >= capacity
 *   - If so, double the capacity and reallocate the array
 * - Store the node and answeredYes in frames[size]
 * - Increment size
 */
void fs_push(FrameStack *s, Node *node, int answeredYes) {
    // TODO: Implement this function
    if (s->size >= s->capacity) { //if it needs to grow, it can
        int newcap = s->capacity * 2; //allocating twice capacity if needed
        Frame *newframes = (Frame *)realloc(s->frames, sizeof(Frame) * newcap);
        if (!newframes) { //if it doesn't allocate then just return
            // Allocation failed; do not change state
            return;
        }
        s->frames = newframes; //change to the new allocated memory and capacity
        s->capacity = newcap;
    }
    s->frames[s->size].node = node; //sets the node
    s->frames[s->size].answeredYes = answeredYes; //sets the flag
    s->size += 1; //grows the size of the stack
}

/* TODO 7: Implement fs_pop
 * - Decrement size
 * - Return the frame at frames[size]
 * Note: No need to check if empty - caller should use fs_empty() first
 */
Frame fs_pop(FrameStack *s) {
    Frame dummy = {NULL, -1}; //if empty
    if (s->size == 0) return dummy; //return something if stack empty
    s->size -= 1; //reduce size
    return s->frames[s->size]; //return the top frame
    // TODO: Implement this function
}

/* TODO 8: Implement fs_empty
 * - Return 1 if size == 0, otherwise return 0
 */
int fs_empty(FrameStack *s) {
    // TODO: Implement this function
    return s->size == 0 ? 1 : 0; //1 if its empty, 0 if not
}

/* TODO 9: Implement fs_free
 * - Free the frames array
 * - Set frames pointer to NULL
 * - Reset size and capacity to 0
 */
void fs_free(FrameStack *s) {
    // TODO: Implement this function
    free(s->frames); //frees the buffer
    s->frames = NULL; //sets to null to avoid errors down the line
    s->size = 0; //resets the size and capacity
    s->capacity = 0;
}

/* ========== Edit Stack (for undo/redo) ========== */

/* TODO 10: Implement es_init
 * Similar to fs_init but for Edit structs
 */
void es_init(EditStack *s) {
    // TODO: Implement this function
    s->capacity = 16; //initialize capacity
    s->size = 0; //empty the stack
    s->edits = (Edit *)malloc(sizeof(Edit) * s->capacity); //allocates memory for edit stack
}

/* TODO 11: Implement es_push
 * Similar to fs_push but for Edit structs
 * - Check capacity and resize if needed
 * - Add edit to array and increment size
 */
void es_push(EditStack *s, Edit e) {
    // TODO: Implement this function
    if (s->size >= s->capacity) { //if it needs to grow, realloc with double the capacity
        int newcap = s->capacity * 2;
        Edit *newedits = (Edit *)realloc(s->edits, sizeof(Edit) * newcap);
        if (!newedits) {  //just in case the realloc doesn't wok
            return;
        }
        s->edits = newedits; //change to the new buffer, capacity, store the edit and grow the size
        s->capacity = newcap;
    }
    s->edits[s->size] = e;
    s->size += 1;
}

/* TODO 12: Implement es_pop
 * Similar to fs_pop but for Edit structs
 */
Edit es_pop(EditStack *s) {
    Edit dummy = (Edit){0}; //if empty
    if (s->size == 0) return dummy; //returns dummy if its empty = no error
    s->size -= 1; //reduces size and returns the top edit frame 
    return s->edits[s->size];
    // TODO: Implement this function
    
}

/* TODO 13: Implement es_empty
 * Return 1 if size == 0, otherwise 0
 */
int es_empty(EditStack *s) {
    // TODO: Implement this function
    return s->size == 0 ? 1 : 0; //1 if empty, 0 if not
}

/* TODO 14: Implement es_clear
 * - Set size to 0 (don't free memory, just reset)
 * - This is used to clear the redo stack when a new edit is made
 */
void es_clear(EditStack *s) {
    // TODO: Implement this function
    for (int i = 0; i < s->size; i++) { //goes through all stored edits
        free_detached_edit(&s->edits[i]); //frees if detached
                                          // this is what made the difference for valgrind errors
    }
    s->size = 0; //clears and sets size to 0

}

void es_free(EditStack *s) {
    for (int i = 0; i < s->size; i++) { //frees any detached edits from undos so no mem leaks
        free_detached_edit(&s->edits[i]);
    }
    free(s->edits); //frees buffer, resets size and capacity
    s->edits = NULL;
    s->size = 0;
    s->capacity = 0;
}

void free_edit_stack(EditStack *s) {
    es_free(s); //free stack
}

/* ========== Queue (for BFS traversal) ========== */

/* TODO 15: Implement q_init
 * - Set front and rear to NULL
 * - Set size to 0
 */
void q_init(Queue *q) {
    // TODO: Implement this function
    q->front = NULL; //initializes queue front, rear, and size
    q->rear = NULL;
    q->size = 0;
}

/* TODO 16: Implement q_enqueue
 * - Allocate a new QueueNode
 * - Set its treeNode and id fields
 * - Set its next pointer to NULL
 * - If queue is empty (rear == NULL):
 *   - Set both front and rear to the new node
 * - Otherwise:
 *   - Link rear->next to the new node
 *   - Update rear to point to the new node
 * - Increment size
 */
void q_enqueue(Queue *q, Node *node, int id) {
    // TODO: Implement this function
    QueueNode *qn = (QueueNode *)malloc(sizeof(QueueNode)); //allocates memory for the new node
    if (!qn) return; //just in case
    qn->treeNode = node; //sets the node
    qn->id = id; //sets the id
    qn->next = NULL; //added to the back of queue so next pointer is null

    if (q->rear == NULL) { //if its empty
        q->front = qn; //the head is the new node
        q->rear = qn; //the tail is the new node
    } else {
        q->rear->next = qn; //if not it is linked at the tail 
        q->rear = qn; //the tail to this node
    }
    q->size += 1; //increase size of the queue
}

/* TODO 17: Implement q_dequeue
 * - If queue is empty (front == NULL), return 0
 * - Save the front node's data to output parameters (*node, *id)
 * - Save front in a temp variable
 * - Move front to front->next
 * - If front is now NULL, set rear to NULL too
 * - Free the temp node
 * - Decrement size
 * - Return 1
 */
int q_dequeue(Queue *q, Node **node, int *id) {
    // TODO: Implement this function

    //takes the head temporarily, outputs the node, outputs the id, advances the hea
    //checks if queue is empty and clears the tail if so, then frees the old head, decreases the size
    //and returns 1 if it succeeds
    if (q->front == NULL) return 0;
    QueueNode *tmp = q->front;
    if (node) *node = tmp->treeNode;
    if (id) *id = tmp->id;
    q->front = tmp->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }
    free(tmp);
    q->size -= 1;
    return 1;
}

/* TODO 18: Implement q_empty
 * Return 1 if size == 0, otherwise 0
 */
int q_empty(Queue *q) {
    // TODO: Implement this function
    return q->size == 0 ? 1 : 0; //1 if empty
    
}

/* TODO 19: Implement q_free
 * - Dequeue all remaining nodes
 * - Use a loop with q_dequeue until queue is empty
 */
void q_free(Queue *q) {
    // TODO: Implement this function
    Node *n = NULL; //temporary node pointer
    int id = 0; //id to set all of the queue to
    while (q_dequeue(q, &n, &id)) { //dequeues everything with NULL and id=0
        /* nothing else to do */
    }
}

/* ========== Hash Table ========== */

/* TODO 20: Implement canonicalize
 * Convert a string to canonical form for hashing:
 * - Convert to lowercase
 * - Keep only alphanumeric characters
 * - Replace spaces with underscores
 * - Remove punctuation
 * Example: "Does it meow?" -> "does_it_meow"
 * 
 * Steps:
 * - Allocate result buffer (strlen(s) + 1)
 * - Iterate through input string
 * - For each character:
 *   - If alphanumeric: add lowercase version to result
 *   - If whitespace: add underscore
 *   - Otherwise: skip it
 * - Null-terminate result
 * - Return the new string
 */
char *canonicalize(const char *s) {
    // TODO: Implement this function
    if (!s) return NULL;
    size_t len = strlen(s);
    char *out = (char *)malloc(len + 1); //worst-case memory allocation
    if (!out) return NULL;
    //writes the index, scans the input, reads the bytes and keeps letters and digits
    //makes a lowercase copy and maps spaces with underscore
    size_t j = 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (isalnum(c)) {
            out[j++] = (char)tolower(c);
        } else if (isspace(c)) {
            out[j++] = '_';
        } else {
            /* skip punctuation and other symbols */
        }
    }
    out[j] = '\0'; //terminator
    return out;
}

/* TODO 21: Implement h_hash (djb2 algorithm)
 * unsigned hash = 5381;
 * For each character c in the string:
 *   hash = ((hash << 5) + hash) + c;  // hash * 33 + c
 * Return hash
 */
unsigned h_hash(const char *s) {
    // TODO: Implement this function
    unsigned hash = 5381u; //this is the seed
    if (!s) return hash;
    for (const unsigned char *p = (const unsigned char *)s; *p; ++p) {
        hash = ((hash << 5) + hash) + *p; //hash *33 +c
    }
    return hash;
}

/* TODO 22: Implement h_init
 * - Allocate buckets array using calloc (initializes to NULL)
 * - Set nbuckets field
 * - Set size to 0
 */
void h_init(Hash *h, int nbuckets) {
    // TODO: Implement this function
    h->nbuckets = nbuckets > 0 ? nbuckets : 1; //bucket counts
    h->size = 0; //initializes it
    h->buckets = (Entry **)calloc((size_t)h->nbuckets, sizeof(Entry *)); //zeroes the buckets
}

/* TODO 23: Implement h_put
 * Add animalId to the list for the given key
 * 
 * Steps:
 * 1. Compute bucket index: idx = h_hash(key) % nbuckets
 * 2. Search the chain at buckets[idx] for an entry with matching key
 * 3. If found:
 *    - Check if animalId already exists in the vals list
 *    - If yes, return 0 (no change)
 *    - If no, add animalId to vals.ids array (resize if needed), return 1
 * 4. If not found:
 *    - Create new Entry with strdup(key)
 *    - Initialize vals with initial capacity (e.g., 4)
 *    - Add animalId as first element
 *    - Insert at head of chain (buckets[idx])
 *    - Increment h->size
 *    - Return 1
 */
int h_put(Hash *h, const char *key, int animalId) {
    // TODO: Implement this function
    if (!h || !key) return 0;
    unsigned idx = h_hash(key) % (unsigned)h->nbuckets;

    Entry *e = h->buckets[idx]; //head of chain
    while (e) { //searches chain for key, if present no change
        if (strcmp(e->key, key) == 0) {
            // check for duplicate
            for (int i = 0; i < e->vals.count; ++i) {
                if (e->vals.ids[i] == animalId) {
                    return 0; // no change
                }
            }
            // add new id and checks if need more memory
            if (e->vals.count >= e->vals.capacity) {
                int newcap = e->vals.capacity > 0 ? e->vals.capacity * 2 : 4;
                int *newids = (int *)realloc(e->vals.ids, sizeof(int) * newcap);
                if (!newids) return 0;
                e->vals.ids = newids;
                e->vals.capacity = newcap;
            }
            e->vals.ids[e->vals.count++] = animalId; //adds id
            return 1;
        }
        e = e->next; //next entry
    }

    // not found, create new entry
    //copies the key, sets capacity and stores id
    Entry *ne = (Entry *)malloc(sizeof(Entry));
    if (!ne) return 0;
    ne->key = strdup(key);
    if (!ne->key) {
        free(ne);
        return 0;
    }
    ne->vals.ids = (int *)malloc(sizeof(int) * 4);
    if (!ne->vals.ids) {
        free(ne->key);
        free(ne);
        return 0;
    }
    ne->vals.capacity = 4;
    ne->vals.count = 1;
    ne->vals.ids[0] = animalId;
    //inserts at the head, updates bucket, adds to size
    ne->next = h->buckets[idx];
    h->buckets[idx] = ne;
    h->size += 1;
    return 1;
}

/* TODO 24: Implement h_contains
 * Check if the hash table contains the given key-animalId pair
 * 
 * Steps:
 * 1. Compute bucket index
 * 2. Search the chain for matching key
 * 3. If found, search vals.ids array for animalId
 * 4. Return 1 if found, 0 otherwise
 */
int h_contains(const Hash *h, const char *key, int animalId) {
    // TODO: Implement this function
    if (!h || !key) return 0; //bucketindex below
    unsigned idx = h_hash(key) % (unsigned)h->nbuckets;
    Entry *e = h->buckets[idx];
    while (e) {
        if (strcmp(e->key, key) == 0) {
            for (int i = 0; i < e->vals.count; ++i) {
                if (e->vals.ids[i] == animalId) return 1; //if found
            }
            return 0; //key found, id not found
        }
        e = e->next;
    }
    return 0; //not found
}

/* TODO 25: Implement h_get_ids
 * Return pointer to the ids array for the given key
 * Set *outCount to the number of ids
 * Return NULL if key not found
 * 
 * Steps:
 * 1. Compute bucket index
 * 2. Search chain for matching key
 * 3. If found:
 *    - Set *outCount = vals.count
 *    - Return vals.ids
 * 4. If not found:
 *    - Set *outCount = 0
 *    - Return NULL
 */
int *h_get_ids(const Hash *h, const char *key, int *outCount) {
    // TODO: Implement this function
   if (outCount) *outCount = 0;
    if (!h || !key) return NULL;
    unsigned idx = h_hash(key) % (unsigned)h->nbuckets;
    Entry *e = h->buckets[idx];
    while (e) { //scans the table, if key matches returns the array
        if (strcmp(e->key, key) == 0) {
            if (outCount) *outCount = e->vals.count;
            return e->vals.ids;
        }
        e = e->next;
    }
    return NULL; //not found
}

/* TODO 26: Implement h_free
 * Free all memory associated with the hash table
 * 
 * Steps:
 * - For each bucket:
 *   - Traverse the chain
 *   - For each entry:
 *     - Free the key string
 *     - Free the vals.ids array
 *     - Free the entry itself
 * - Free the buckets array
 * - Set buckets to NULL, size to 0
 */
void h_free(Hash *h) {
    // TODO: Implement this function
    if (!h || !h->buckets) {
        return;
    }
    for (int i = 0; i < h->nbuckets; ++i) { //walks through the chain, saves the next to keep going, frees the key
        //frees id buffer, and entry
        Entry *e = h->buckets[i];
        while (e) {
            Entry *next = e->next;
            free(e->key);
            free(e->vals.ids);
            free(e);
            e = next;
        }
    }
    free(h->buckets); //free the whole array
    h->buckets = NULL;
    h->size = 0;
    h->nbuckets = 0;
}
