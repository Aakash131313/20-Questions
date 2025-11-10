#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lab5.h"

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
    if (!question) return NULL;
    Node *n = (Node *)malloc(sizeof(Node));
    if (!n) return NULL;
    n->text = strdup(question);
    if (!n->text) {
        free(n);
        return NULL;
    }
    n->isQuestion = 1;
    n->yes = NULL;
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
    Node *n = (Node *)malloc(sizeof(Node));
    if (!n) return NULL;
    n->text = strdup(animal);
    if (!n->text) {
        free(n);
        return NULL;
    }
    n->isQuestion = 0;
    n->yes = NULL;
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
    if (!node) return;
    free_tree(node->yes);
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
    s->capacity = 16;
    s->size = 0;
    s->frames = (Frame *)malloc(sizeof(Frame) * s->capacity);
}

/* TODO 6: Implement fs_push
 * - Check if size >= capacity
 *   - If so, double the capacity and reallocate the array
 * - Store the node and answeredYes in frames[size]
 * - Increment size
 */
void fs_push(FrameStack *s, Node *node, int answeredYes) {
    // TODO: Implement this function
    if (s->size >= s->capacity) {
        int newcap = s->capacity * 2;
        Frame *newframes = (Frame *)realloc(s->frames, sizeof(Frame) * newcap);
        if (!newframes) {
            // Allocation failed; do not change state
            return;
        }
        s->frames = newframes;
        s->capacity = newcap;
    }
    s->frames[s->size].node = node;
    s->frames[s->size].answeredYes = answeredYes;
    s->size += 1;
}

/* TODO 7: Implement fs_pop
 * - Decrement size
 * - Return the frame at frames[size]
 * Note: No need to check if empty - caller should use fs_empty() first
 */
Frame fs_pop(FrameStack *s) {
    Frame dummy = {NULL, -1};
    if (s->size == 0) return dummy;
    s->size -= 1;
    return s->frames[s->size];
    // TODO: Implement this function
}

/* TODO 8: Implement fs_empty
 * - Return 1 if size == 0, otherwise return 0
 */
int fs_empty(FrameStack *s) {
    // TODO: Implement this function
    return s->size == 0 ? 1 : 0;
    return 1;
}

/* TODO 9: Implement fs_free
 * - Free the frames array
 * - Set frames pointer to NULL
 * - Reset size and capacity to 0
 */
void fs_free(FrameStack *s) {
    // TODO: Implement this function
    free(s->frames);
    s->frames = NULL;
    s->size = 0;
    s->capacity = 0;
}

/* ========== Edit Stack (for undo/redo) ========== */

/* TODO 10: Implement es_init
 * Similar to fs_init but for Edit structs
 */
void es_init(EditStack *s) {
    // TODO: Implement this function
    s->capacity = 16;
    s->size = 0;
    s->edits = (Edit *)malloc(sizeof(Edit) * s->capacity);
}

/* TODO 11: Implement es_push
 * Similar to fs_push but for Edit structs
 * - Check capacity and resize if needed
 * - Add edit to array and increment size
 */
void es_push(EditStack *s, Edit e) {
    // TODO: Implement this function
    if (s->size >= s->capacity) {
        int newcap = s->capacity * 2;
        Edit *newedits = (Edit *)realloc(s->edits, sizeof(Edit) * newcap);
        if (!newedits) {
            return;
        }
        s->edits = newedits;
        s->capacity = newcap;
    }
    s->edits[s->size] = e;
    s->size += 1;
}

/* TODO 12: Implement es_pop
 * Similar to fs_pop but for Edit structs
 */
Edit es_pop(EditStack *s) {
    Edit dummy = (Edit){0};
    if (s->size == 0) return dummy;
    s->size -= 1;
    return s->edits[s->size];
    // TODO: Implement this function
    
}

/* TODO 13: Implement es_empty
 * Return 1 if size == 0, otherwise 0
 */
int es_empty(EditStack *s) {
    // TODO: Implement this function
    return s->size == 0 ? 1 : 0;
}

/* TODO 14: Implement es_clear
 * - Set size to 0 (don't free memory, just reset)
 * - This is used to clear the redo stack when a new edit is made
 */
void es_clear(EditStack *s) {
    // TODO: Implement this function
    s->size = 0;

}

void es_free(EditStack *s) {
    free(s->edits);
    s->edits = NULL;
    s->size = 0;
    s->capacity = 0;
}

void free_edit_stack(EditStack *s) {
    es_free(s);
}

/* ========== Queue (for BFS traversal) ========== */

/* TODO 15: Implement q_init
 * - Set front and rear to NULL
 * - Set size to 0
 */
void q_init(Queue *q) {
    // TODO: Implement this function
    q->front = NULL;
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
    QueueNode *qn = (QueueNode *)malloc(sizeof(QueueNode));
    if (!qn) return;
    qn->treeNode = node;
    qn->id = id;
    qn->next = NULL;

    if (q->rear == NULL) {
        q->front = qn;
        q->rear = qn;
    } else {
        q->rear->next = qn;
        q->rear = qn;
    }
    q->size += 1;
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
    return q->size == 0 ? 1 : 0;
    
}

/* TODO 19: Implement q_free
 * - Dequeue all remaining nodes
 * - Use a loop with q_dequeue until queue is empty
 */
void q_free(Queue *q) {
    // TODO: Implement this function
    Node *n = NULL;
    int id = 0;
    while (q_dequeue(q, &n, &id)) {
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
    return NULL;
}

/* TODO 21: Implement h_hash (djb2 algorithm)
 * unsigned hash = 5381;
 * For each character c in the string:
 *   hash = ((hash << 5) + hash) + c;  // hash * 33 + c
 * Return hash
 */
unsigned h_hash(const char *s) {
    // TODO: Implement this function
    return 0;
}

/* TODO 22: Implement h_init
 * - Allocate buckets array using calloc (initializes to NULL)
 * - Set nbuckets field
 * - Set size to 0
 */
void h_init(Hash *h, int nbuckets) {
    // TODO: Implement this function
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
    return 0;
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
    return 0;
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
    *outCount = 0;
    return NULL;
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
}
