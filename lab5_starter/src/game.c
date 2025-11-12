#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "lab5.h"

extern Node *g_root;
extern EditStack g_undo;
extern EditStack g_redo;
extern Hash g_index;


//helpers
int read_yes_no(void) {
    int ch; //inputs char
    while (1) { //waits until y/n
        ch = getch();
        if (ch == 'y' || ch == 'Y') return 1; //yes
        if (ch == 'n' || ch == 'N') return 0; //no
    }
}

    // Safe line input into a fixed buffer with echo on/off
void read_line_at(int row, int col, char *buf, int cap) {
    move(row, col); //positions the cursor
    echo(); //enables echo
    // Read at most cap-1 chars; getnstr writes NULL
    getnstr(buf, cap - 1); //bound the read
    noecho(); //disables the echo
}

static void free_tree_iterative(Node *root) {
    if (!root) return;

    // Use your existing FrameStack to avoid recursion
    FrameStack st; //temp stack
    fs_init(&st);
    fs_push(&st, root, -1); //start with root

    while (!fs_empty(&st)) { //DFS
        Frame f = fs_pop(&st); //pop frame
        Node *n = f.node; //current node
        if (!n) continue; //skip null

        // Push children first so we can free n safely afterward
        if (n->isQuestion) { //enqueue children push yes and push no
            if (n->yes) fs_push(&st, n->yes, -1);
            if (n->no)  fs_push(&st, n->no,  -1);
        }

        // Free text then the node
        if (n->text) free(n->text);
        free(n);
    }

    fs_free(&st);
}

// Register exactly once to run at process exit
static void cleanup_at_exit(void) {
    extern Node *g_root; //gloabl root and free the whole tree, then clear root
    free_tree_iterative(g_root);
    g_root = NULL;
}



// Free all orphaned nodes currently stored in g_redo.
// Each Edit on g_redo holds newQuestion (root of the detached mini-tree) and
// newLeaf (inside that mini-tree). But newQuestion also points to oldLeaf,
// which *is still in the main tree*. So we must detach that pointer first to
// avoid double-free, then free the remaining subtree.
static void drain_redo_and_free_orphans(void) {
    while (!es_empty(&g_redo)) {
        Edit e = es_pop(&g_redo);

        if (e.newQuestion) {
            // Detach the branch that points back to the live oldLeaf.
            if (e.newQuestion->yes == e.oldLeaf) e.newQuestion->yes = NULL;
            if (e.newQuestion->no  == e.oldLeaf) e.newQuestion->no  = NULL;

            // Free the detached mini-tree (this will free newLeaf too).
            free_subtree(e.newQuestion);
        }
        
    }
}

// “cleanup/exit” path in main.c,
// it will also remove any remaining orphans (undo and quit).
void drain_all_edit_orphans_on_exit(void) {
    drain_redo_and_free_orphans();
}


/* TODO 31: Implement play_game
 * Main game loop using iterative traversal with a stack
 * 
 * Key requirements:
 * - Use FrameStack (NO recursion!)
 * - Push frames for each decision point
 * - Track parent and answer for learning
 * 
 * Steps:
 * 1. Initialize and display game UI
 * 2. Initialize FrameStack
 * 3. Push root frame with answeredYes = -1
 * 4. Set parent = NULL, parentAnswer = -1
 * 5. While stack not empty:
 *    a. Pop current frame
 *    b. If current node is a question:
 *       - Display question and get user's answer (y/n)
 *       - Set parent = current node
 *       - Set parentAnswer = answer
 *       - Push appropriate child (yes or no) onto stack
 *    c. If current node is a leaf (animal):
 *       - Ask "Is it a [animal]?"
 *       - If correct: celebrate and break
 *       - If wrong: LEARNING PHASE
 *         i. Get correct animal name from user
 *         ii. Get distinguishing question
 *         iii. Get answer for new animal (y/n for the question)
 *         iv. Create new question node and new animal node
 *         v. Link them: if newAnswer is yes, newQuestion->yes = newAnimal
 *         vi. Update parent pointer (or g_root if parent is NULL)
 *         vii. Create Edit record and push to g_undo
 *         viii. Clear g_redo stack
 *         ix. Update g_index with canonicalized question
 * 6. Free stack
 */
void play_game() {
    clear(); //clears the screen
    attron(COLOR_PAIR(5) | A_BOLD); //header and title and end style
    mvprintw(0, 0, "%-80s", " Playing 20 Questions");
    attroff(COLOR_PAIR(5) | A_BOLD);
    
    //prompt and wait for input
    mvprintw(2, 2, "Think of an animal, and I'll try to guess it!");
    mvprintw(3, 2, "Press any key to start...");
    refresh();
    getch();
    
    // TODO: Implement the game loop
    // Initialize FrameStack
    // Push root
    // Loop until stack empty or guess is correct
    // Handle question nodes and leaf nodes differently
    
    FrameStack stack; //traversal stack
    fs_init(&stack); //initialize stack

    static int cleanup_registered = 0; //one time flag, register once and free on exit
    if (!cleanup_registered) {
        atexit(cleanup_at_exit);
        cleanup_registered = 1;
    }
    
    // TODO: Your implementation here
    if (g_root == NULL) { //if empty knowledge
        mvprintw(5, 2, "I don't know any animals yet. Teach me one!");
        mvprintw(7, 2, "Press any key to return...");
        refresh();
        getch();
        fs_free(&stack);
        return;
    }

    //Push root frame with answeredYes = -1
    fs_push(&stack, g_root, -1); //start at root

    Node *parent = NULL;        // parent of current node
    int parentAnswer = -1;      // 1 if we took 'yes' branch from parent, 0 if 'no', -1 for root
    int guessed = 0;            // set to 1 when we guess correctly to end loop

    while (!fs_empty(&stack)) { //main loop
        Frame f = fs_pop(&stack); //gets the frame and moves to cur node
        Node *cur = f.node;

        // Here, we use the "answeredYes" field on the frame to preserve which way we came.
        if (f.answeredYes != -1) {
            // came from a parent
            parentAnswer = f.answeredYes; // 1 = via yes, 0 = via no
        }

        // Question node
        if (cur->isQuestion) { //asks a question
            clear(); //refresh screen
            attron(COLOR_PAIR(5) | A_BOLD);
            mvprintw(0, 0, "%-80s", " Playing 20 Questions");
            attroff(COLOR_PAIR(5) | A_BOLD);

            mvprintw(2, 2, "%s (y/n): ", cur->text); //print question
            refresh();
            int ans = read_yes_no(); //reads yes or no

            // Set parent context for the child we’re about to visit
            parent = cur; //remembers parent and records branch
            parentAnswer = ans ? 1 : 0;

            // Push the chosen child
            Node *next = ans ? cur->yes : cur->no; //selects childs
            if (next == NULL) {
                // Defensive: malformed tree
                mvprintw(4, 2, "Internal error: missing child. Press any key...");
                refresh();
                getch(); //wait and then break
                break;
            }
            // Push with answeredYes showing how we reached next (from parent)
            fs_push(&stack, next, parentAnswer); //push the child with context
            continue; //next iter
        }

        // Leaf (animal guess)
        clear(); //new screen
        attron(COLOR_PAIR(5) | A_BOLD);
        mvprintw(0, 0, "%-80s", " Playing 20 Questions");
        attroff(COLOR_PAIR(5) | A_BOLD);

        mvprintw(2, 2, "Is it a %s? (y/n): ", cur->text); //guess and read y/n
        refresh();
        int correct = read_yes_no();

        if (correct) {
            mvprintw(4, 2, "Yay! I guessed it! Press any key...");
            refresh(); //draw, wait and then mark done and exit loop
            getch();
            guessed = 1;
            break;
        }

        //Learning phase
        // Ask: what animal, what question, and what is the answer for the new animal
        char animal[256] = {0}; //new animal and question buffer
        char question[256] = {0};

        clear(); //new screen
        attron(COLOR_PAIR(5) | A_BOLD);
        mvprintw(0, 0, "%-80s", " Teaching me a new animal");
        attroff(COLOR_PAIR(5) | A_BOLD);

        mvprintw(2, 2, "What animal were you thinking of?"); ///prompt, read animal
        mvprintw(3, 4, "Animal: ");
        refresh();
        read_line_at(3, 13, animal, sizeof(animal));

        mvprintw(5, 2, "Give me a yes/no question to distinguish"); //prompt and input label and then read animal
        mvprintw(6, 4, "Question: ");
        refresh();
        read_line_at(6, 14, question, sizeof(question));

        mvprintw(8, 2, "For %s, what is the answer? (y/n): ", animal);//prompt +input labl
        refresh();
        int newYes = read_yes_no(); // 1 if the new animal answers "yes" to the question

        // Create nodes
        Node *newQ = create_question_node(question); //new quesiton and animal node
        Node *newA = create_animal_node(animal);

        // Wire new question: newQ->yes/newQ->no
        if (newYes) { //if yes
            newQ->yes = newA;
            newQ->no  = cur;   // old wrong guess goes to the opposite branch
        } else { //if no
            newQ->no  = newA;
            newQ->yes = cur;
        }

        // Splice newQ into the tree where 'cur' was
        if (parent == NULL || parentAnswer == -1) { //replaced root
            // We were at root
            g_root = newQ; //new root
        } else { // parent yes link
            if (parentAnswer == 1) parent->yes = newQ;
            else                    parent->no  = newQ; //parent no link
        }

        // Record the edit for undo/redo
        Edit e;
        e.parent       = parent;        // NULL if root
        e.wasYesChild  = (parentAnswer == 1) ? 1 : 0;
        e.oldLeaf      = cur;           // the leaf we replaced
        e.newQuestion  = newQ;          // the question we inserted
        e.newLeaf      = newA;          // the new animal leaf
        es_push(&g_undo, e);
        es_clear(&g_redo);
        //drain_redo_and_free_orphans();    
        // Update index (optional). We canonicalize and h_put with a stable copy inside the hash.
        if (question[0] != '\0') {
            char *key = canonicalize(question);
            // Use a simple ID: we can re-count nodes or leave as 0; tests don’t rely on this.
            // If your h_put() strdup's the key (per TODO 23), it’s safe to free key afterwards.
            h_put(&g_index, key, 0);
            free(key);
        }

        mvprintw(10, 2, "Thanks! I'll remember that. Press any key...");
        refresh();
        getch();

        // After learning once, we end this round.
        guessed = 1;
        break;
    }

    (void)guessed; // suppress unused warning if not used later
    fs_free(&stack);
}

/* TODO 32: Implement undo_last_edit
 * Undo the most recent tree modification
 * 
 * Steps:
 * 1. Check if g_undo stack is empty, return 0 if so
 * 2. Pop edit from g_undo
 * 3. Restore the tree structure:
 *    - If edit.parent is NULL:
 *      - Set g_root = edit.oldLeaf
 *    - Else if edit.wasYesChild:
 *      - Set edit.parent->yes = edit.oldLeaf
 *    - Else:
 *      - Set edit.parent->no = edit.oldLeaf
 * 4. Push edit to g_redo stack
 * 5. Return 1
 * 
 * Note: We don't free newQuestion/newLeaf because they might be redone
 */
int undo_last_edit() {
    // TODO: Implement this function
    if (es_empty(&g_undo)) return 0;
    Edit e = es_pop(&g_undo); //pop last edit

    // restore tree pointer
    if (e.parent == NULL) { //root replacement
        g_root = e.oldLeaf; //restore
    } else if (e.wasYesChild) { //on yes side restore that link and same with no side
        e.parent->yes = e.oldLeaf;
    } else {
        e.parent->no = e.oldLeaf;
    }

    // IMPORTANT: break the link from the detached newQuestion to oldLeaf
    if (e.newQuestion) {
        if (e.newQuestion->yes == e.oldLeaf) e.newQuestion->yes = NULL;
        if (e.newQuestion->no  == e.oldLeaf) e.newQuestion->no  = NULL;
    }

    es_push(&g_redo, e); //move to redo stack
    return 1;
}

/* TODO 33: Implement redo_last_edit
 * Redo a previously undone edit
 * 
 * Steps:
 * 1. Check if g_redo stack is empty, return 0 if so
 * 2. Pop edit from g_redo
 * 3. Reapply the tree modification:
 *    - If edit.parent is NULL:
 *      - Set g_root = edit.newQuestion
 *    - Else if edit.wasYesChild:
 *      - Set edit.parent->yes = edit.newQuestion
 *    - Else:
 *      - Set edit.parent->no = edit.newQuestion
 * 4. Push edit back to g_undo stack
 * 5. Return 1
 */
int redo_last_edit() {
    // TODO: Implement this function
    if (es_empty(&g_redo)) return 0;
    Edit e = es_pop(&g_redo); //pop redo edit

    // restore the detached link to oldLeaf
    if (e.newQuestion) {
        if (e.newQuestion->yes == NULL && e.newQuestion->no != e.oldLeaf) {
            e.newQuestion->yes = e.oldLeaf; //reattach on yes
        } else if (e.newQuestion->no == NULL && e.newQuestion->yes != e.oldLeaf) {
            e.newQuestion->no = e.oldLeaf; //reattach on no
        }
        // (If neither child is NULL, the link was never broken; leave as is.)
    }

    if (e.parent == NULL) { //reapply at root
        g_root = e.newQuestion; //root -> newquest
    } else if (e.wasYesChild) { //reapply on yes
        e.parent->yes = e.newQuestion; //parent yes link
    } else {
        e.parent->no = e.newQuestion; //parent no link
    }

    es_push(&g_undo, e); //back to undo stack
    return 1;
}