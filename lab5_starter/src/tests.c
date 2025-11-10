#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "lab5.h"

/* Test Frame Stack */
void test_stack() {
    printf("Testing Frame Stack...\n");
    
    FrameStack s;
    fs_init(&s);
    
    assert(fs_empty(&s));
    
    Node dummy1 = {0};
    Node dummy2 = {0};
    
    fs_push(&s, &dummy1, 1);
    fs_push(&s, &dummy2, 0);
    
    assert(s.size == 2);
    assert(!fs_empty(&s));
    
    Frame f = fs_pop(&s);
    assert(f.node == &dummy2);
    assert(f.answeredYes == 0);
    
    f = fs_pop(&s);
    assert(f.node == &dummy1);
    assert(f.answeredYes == 1);
    
    assert(fs_empty(&s));
    
    /* Test resize */
    for (int i = 0; i < 100; i++) {
        fs_push(&s, &dummy1, i % 2);
    }
    assert(s.size == 100);
    assert(s.capacity >= 100);
    
    fs_free(&s);
    printf("  ✓ Stack tests passed\n");
}

/* Test Queue */
void test_queue() {
    printf("Testing Queue...\n");
    
    Queue q;
    q_init(&q);
    
    assert(q_empty(&q));
    
    Node dummy1 = {0};
    Node dummy2 = {0};
    Node dummy3 = {0};
    
    q_enqueue(&q, &dummy1, 1);
    q_enqueue(&q, &dummy2, 2);
    q_enqueue(&q, &dummy3, 3);
    
    assert(q.size == 3);
    
    Node *n;
    int id;
    
    assert(q_dequeue(&q, &n, &id));
    assert(n == &dummy1 && id == 1);
    
    assert(q_dequeue(&q, &n, &id));
    assert(n == &dummy2 && id == 2);
    
    assert(q_dequeue(&q, &n, &id));
    assert(n == &dummy3 && id == 3);
    
    assert(q_empty(&q));
    assert(!q_dequeue(&q, &n, &id));
    
    q_free(&q);
    printf("  ✓ Queue tests passed\n");
}

/* Test Hash Table */
void test_hash() {
    printf("Testing Hash Table...\n");
    
    Hash h;
    h_init(&h, 7);
    
    assert(h.size == 0);
    
    h_put(&h, "meow", 1);
    h_put(&h, "bark", 2);
    h_put(&h, "meow", 3);
    
    assert(h.size == 2);
    assert(h_contains(&h, "meow", 1));
    assert(h_contains(&h, "meow", 3));
    assert(h_contains(&h, "bark", 2));
    assert(!h_contains(&h, "bark", 1));
    assert(!h_contains(&h, "chirp", 1));
    
    int count;
    int *ids = h_get_ids(&h, "meow", &count);
    assert(count == 2);
    assert((ids[0] == 1 && ids[1] == 3) || (ids[0] == 3 && ids[1] == 1));
    
    /* Test collisions */
    for (int i = 0; i < 50; i++) {
        char key[20];
        sprintf(key, "key%d", i);
        h_put(&h, key, i);
    }
    
    assert(h.size > 2);
    
    h_free(&h);
    printf("  ✓ Hash table tests passed\n");
}

/* Test Persistence */
void test_persistence() {
    printf("Testing Persistence...\n");
    
    /* Create a test tree */
    Node *root = create_question_node("Test question?");
    root->yes = create_animal_node("Cat");
    root->no = create_question_node("Another question?");
    root->no->yes = create_animal_node("Dog");
    root->no->no = create_animal_node("Fish");
    
    /* Save original root */
    Node *saved_root = g_root;
    g_root = root;
    
    /* Save */
    assert(save_tree("test.dat"));
    
    /* Free and load */
    free_tree(g_root);
    g_root = NULL;
    
    assert(load_tree("test.dat"));
    assert(g_root != NULL);
    assert(g_root->isQuestion);
    assert(strcmp(g_root->text, "Test question?") == 0);
    assert(strcmp(g_root->yes->text, "Cat") == 0);
    
    /* Round-trip test */
    assert(save_tree("test2.dat"));
    
    FILE *f1 = fopen("test.dat", "rb");
    FILE *f2 = fopen("test2.dat", "rb");
    
    fseek(f1, 0, SEEK_END);
    fseek(f2, 0, SEEK_END);
    long size1 = ftell(f1);
    long size2 = ftell(f2);
    
    assert(size1 == size2);
    
    fclose(f1);
    fclose(f2);
    
    /* Restore original root */
    free_tree(g_root);
    g_root = saved_root;
    
    remove("test.dat");
    remove("test2.dat");
    
    printf("  ✓ Persistence tests passed\n");
}

/* Test Integrity Checker */
void test_integrity() {
    printf("Testing Integrity Checker...\n");
    
    /* Valid tree */
    Node *root = create_question_node("Q1");
    root->yes = create_animal_node("A1");
    root->no = create_animal_node("A2");
    
    Node *saved = g_root;
    g_root = root;
    
    assert(check_integrity());
    
    /* Invalid tree - question with one child */
    root->no = NULL;
    assert(!check_integrity());
    
    /* Restore */
    root->no = create_animal_node("A2");
    assert(check_integrity());
    
    free_tree(g_root);
    g_root = saved;
    
    printf("  ✓ Integrity tests passed\n");
}

/* Test Canonicalization */
void test_canonicalize() {
    printf("Testing Canonicalization...\n");
    
    char *c1 = canonicalize("Does it meow?");
    assert(strcmp(c1, "does_it_meow") == 0);
    free(c1);
    
    char *c2 = canonicalize("Is It BIG??");
    assert(strcmp(c2, "is_it_big") == 0);
    free(c2);
    
    char *c3 = canonicalize("ABC123");
    assert(strcmp(c3, "abc123") == 0);
    free(c3);
    
    printf("  ✓ Canonicalization tests passed\n");
}

/* Test Node Creation and Count */
void test_nodes() {
    printf("Testing Node Functions...\n");
    
    /* Test question node */
    Node *q = create_question_node("Does it fly?");
    assert(q != NULL);
    assert(q->isQuestion == 1);
    assert(strcmp(q->text, "Does it fly?") == 0);
    assert(q->yes == NULL);
    assert(q->no == NULL);
    
    /* Test animal node */
    Node *a = create_animal_node("Eagle");
    assert(a != NULL);
    assert(a->isQuestion == 0);
    assert(strcmp(a->text, "Eagle") == 0);
    assert(a->yes == NULL);
    assert(a->no == NULL);
    
    /* Test tree structure and count */
    q->yes = a;
    q->no = create_animal_node("Penguin");
    
    assert(count_nodes(q) == 3);
    assert(count_nodes(NULL) == 0);
    
    free_tree(q);
    
    printf("  ✓ Node tests passed\n");
}

/* Test Edit Stack */
void test_edit_stack() {
    printf("Testing Edit Stack...\n");
    
    EditStack s;
    es_init(&s);
    
    assert(es_empty(&s));
    
    Edit e1, e2;
    e1.type = EDIT_INSERT_SPLIT;
    e1.parent = NULL;
    
    e2.type = EDIT_INSERT_SPLIT;
    e2.parent = (Node*)0x1234;  // Dummy pointer
    
    es_push(&s, e1);
    es_push(&s, e2);
    
    assert(s.size == 2);
    assert(!es_empty(&s));
    
    Edit popped = es_pop(&s);
    assert(popped.parent == (Node*)0x1234);
    
    popped = es_pop(&s);
    assert(popped.parent == NULL);
    
    assert(es_empty(&s));
    
    /* Test clear */
    es_push(&s, e1);
    es_push(&s, e2);
    assert(s.size == 2);
    
    es_clear(&s);
    assert(s.size == 0);
    assert(es_empty(&s));
    
    es_free(&s);
    printf("  ✓ Edit stack tests passed\n");
}

int main() {
    printf("\n=== Running Unit Tests ===\n\n");
    
    test_nodes();
    test_stack();
    test_edit_stack();
    test_queue();
    test_canonicalize();
    test_hash();
    test_persistence();
    test_integrity();
    
    printf("\n=== All Tests Passed! ===\n\n");
    printf("Great job! Your implementations are working correctly.\n");
    printf("Next steps:\n");
    printf("  1. Run 'make run' to test the game interactively\n");
    printf("  2. Run 'make valgrind' to check for memory leaks\n");
    printf("  3. Test learning, undo, redo, save, and load features\n\n");
    
    return 0;
}
