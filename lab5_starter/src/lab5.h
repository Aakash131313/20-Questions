#ifndef LAB5_H
#define LAB5_H

#include <stdint.h>

/* ========== Tree Node ========== */
typedef struct Node {
    char *text;
    struct Node *yes;
    struct Node *no;
    int isQuestion;
} Node;

/* Node constructors */
Node *create_question_node(const char *question);
Node *create_animal_node(const char *animal);
void free_tree(Node *node);
int count_nodes(Node *root);

/* ========== Stack for Gameplay ========== */
typedef struct Frame {
    Node *node;
    int answeredYes;  /* -1 unset, 0 no, 1 yes */
} Frame;

typedef struct {
    Frame *frames;
    int size;
    int capacity;
} FrameStack;

void fs_init(FrameStack *s);
void fs_push(FrameStack *s, Node *node, int answeredYes);
Frame fs_pop(FrameStack *s);
int fs_empty(FrameStack *s);
void fs_free(FrameStack *s);

/* ========== Edit/Undo/Redo ========== */
typedef enum {
    EDIT_INSERT_SPLIT
} EditType;

typedef struct {
    EditType type;
    Node *parent;
    int wasYesChild;  /* 1=yes branch, 0=no branch, -1=root */
    Node *oldLeaf;
    Node *newQuestion;
    Node *newLeaf;
} Edit;

typedef struct {
    Edit *edits;
    int size;
    int capacity;
} EditStack;

void es_init(EditStack *s);
void es_push(EditStack *s, Edit e);
Edit es_pop(EditStack *s);
int es_empty(EditStack *s);
void es_clear(EditStack *s);
void es_free(EditStack *s);
void free_edit_stack(EditStack *s);

extern EditStack g_undo;
extern EditStack g_redo;
extern Node *g_root;

int undo_last_edit();
int redo_last_edit();

/* ========== Queue for BFS ========== */
typedef struct QueueNode {
    Node *treeNode;
    int id;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front;
    QueueNode *rear;
    int size;
} Queue;

void q_init(Queue *q);
void q_enqueue(Queue *q, Node *node, int id);
int q_dequeue(Queue *q, Node **node, int *id);
int q_empty(Queue *q);
void q_free(Queue *q);

/* ========== Hash Table ========== */
typedef struct IdList {
    int *ids;
    int count;
    int capacity;
} IdList;

typedef struct Entry {
    char *key;
    IdList vals;
    struct Entry *next;
} Entry;

typedef struct {
    Entry **buckets;
    int nbuckets;
    int size;
} Hash;

extern void h_init(Hash *h, int nbuckets);
extern unsigned h_hash(const char *s);
extern int h_put(Hash *h, const char *key, int animalId);
extern int h_contains(const Hash *h, const char *key, int animalId);
extern int *h_get_ids(const Hash *h, const char *key, int *outCount);
extern void h_free(Hash *h);
extern char *canonicalize(const char *s);
extern int get_yes_no(int y, int x, const char *prompt);
extern char *get_input(int y, int x, const char *prompt);

extern Hash g_index;

/* ========== Persistence ========== */
int save_tree(const char *filename);
int load_tree(const char *filename);

/* ========== Utilities ========== */
int check_integrity();
void find_shortest_path(const char *animal1, const char *animal2);

/* ========== Gameplay ========== */
void play_game();

/* ========== Visualization ========== */
void draw_tree();

#endif
