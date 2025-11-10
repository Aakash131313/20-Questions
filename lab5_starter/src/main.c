/*
 * main.c - Main program with ncurses UI
 * 
 * This file is COMPLETE and provided to you.
 * You should NOT modify this file except for ONE thing:
 * 
 * AFTER implementing TODOs 1-2 (create_question_node and create_animal_node),
 * scroll down to the initialize_tree() function and UNCOMMENT the code there.
 * 
 * The program will compile and run immediately, showing the UI.
 * However, the tree won't be initialized until you:
 * 1. Implement TODOs 1-2 in ds.c
 * 2. Uncomment the initialization code in initialize_tree() below
 * 
 * Until then, pressing 'p' to play will show an error message.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ncurses.h>
#include "lab5.h"

/* Global root node */
Node *g_root = NULL;

/* Global undo/redo stacks */
EditStack g_undo = {NULL, 0, 0};
EditStack g_redo = {NULL, 0, 0};

/* Global attribute index */
Hash g_index = {NULL, 0, 0};

/* GUI Colors */
#define COLOR_HEADER 1
#define COLOR_QUESTION 2
#define COLOR_SUCCESS 3
#define COLOR_ERROR 4
#define COLOR_INFO 5

void init_gui() {
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(1);
    
    init_pair(COLOR_HEADER, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLOR_QUESTION, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_SUCCESS, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_ERROR, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_INFO, COLOR_WHITE, COLOR_BLUE);
}

void draw_box(int y, int x, int h, int w, const char *title) {
    attron(COLOR_PAIR(COLOR_HEADER) | A_BOLD);
    mvhline(y, x, ACS_HLINE, w);
    mvhline(y + h - 1, x, ACS_HLINE, w);
    mvvline(y, x, ACS_VLINE, h);
    mvvline(y, x + w - 1, ACS_VLINE, h);
    mvaddch(y, x, ACS_ULCORNER);
    mvaddch(y, x + w - 1, ACS_URCORNER);
    mvaddch(y + h - 1, x, ACS_LLCORNER);
    mvaddch(y + h - 1, x + w - 1, ACS_LRCORNER);
    
    if (title) {
        mvprintw(y, x + 2, "[ %s ]", title);
    }
    attroff(COLOR_PAIR(COLOR_HEADER) | A_BOLD);
}

void display_header() {
    attron(COLOR_PAIR(COLOR_INFO) | A_BOLD);
    mvprintw(0, 0, "%-80s", " ECE 312 Lab 5: 20 Questions - Guess the Animal");
    attroff(COLOR_PAIR(COLOR_INFO) | A_BOLD);
}

void display_menu() {
    int row = LINES - 3;
    attron(COLOR_PAIR(COLOR_HEADER));
    mvprintw(row, 2, "[P]lay | [V]iew Tree | [U]ndo | [R]edo | [S]ave | [L]oad | [I]ntegrity | [Q]uit");
    attroff(COLOR_PAIR(COLOR_HEADER));
}

char *get_input(int y, int x, const char *prompt) {
    static char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    attron(COLOR_PAIR(COLOR_QUESTION));
    mvprintw(y, x, "%s", prompt);
    attroff(COLOR_PAIR(COLOR_QUESTION));
    echo();
    curs_set(1);
    mvgetnstr(y, x + strlen(prompt), buffer, 254);
    noecho();
    return buffer;
}

int get_yes_no(int y, int x, const char *prompt) {
    char *input;
    while (1) {
        input = get_input(y, x, prompt);
        if (tolower(input[0]) == 'y') return 1;
        if (tolower(input[0]) == 'n') return 0;
        attron(COLOR_PAIR(COLOR_ERROR));
        mvprintw(y + 1, x, "Please enter 'y' or 'n'");
        attroff(COLOR_PAIR(COLOR_ERROR));
        refresh();
    }
}

void show_message(const char *msg, int is_error) {
    int color = is_error ? COLOR_ERROR : COLOR_SUCCESS;
    attron(COLOR_PAIR(color) | A_BOLD);
    mvprintw(LINES - 5, 2, "%-76s", msg);
    attroff(COLOR_PAIR(color) | A_BOLD);
    refresh();
    napms(1500);
    mvprintw(LINES - 5, 2, "%-76s", "");
}

void initialize_tree() {
    /* Note: This function is intentionally left empty initially.
     * Once you implement create_question_node() and create_animal_node()
     * in ds.c (TODOs 1-2), uncomment the code below to create the initial tree.
     */
    
    /* UNCOMMENT THIS CODE AFTER IMPLEMENTING TODOs 1-2:
    */
    
    if (g_root != NULL) {
        free_tree(g_root);
    }
    
    Node *water = create_question_node("Does it live in water?");
    water->yes = create_animal_node("Fish");
    water->no = create_animal_node("Dog");
    g_root = water;
    
    h_free(&g_index);
    h_init(&g_index, 31);
    
    
}

int main() {
    init_gui();
    
    /* Initialize undo/redo stacks FIRST */
    g_undo.edits = NULL;
    g_undo.size = 0;
    g_undo.capacity = 0;
    es_init(&g_undo);
    
    g_redo.edits = NULL;
    g_redo.size = 0;
    g_redo.capacity = 0;
    es_init(&g_redo);
    
    initialize_tree();
    
    int running = 1;
    while (running) {
        clear();
        display_header();
        draw_box(2, 1, LINES - 6, COLS - 2, "Game Status");
        display_menu();
        
        mvprintw(4, 3, "Tree nodes: %d", g_root ? count_nodes(g_root) : 0);
        mvprintw(5, 3, "Undo stack: %d | Redo stack: %d", g_undo.size, g_redo.size);
        
        if (g_root == NULL) {
            attron(COLOR_PAIR(COLOR_ERROR));
            mvprintw(7, 3, "Tree not initialized! Implement TODOs 1-2 and uncomment code in main.c");
            attroff(COLOR_PAIR(COLOR_ERROR));
        } else {
            mvprintw(7, 3, "Choose an option:");
        }
        refresh();
        
        int ch = getch();
        
        switch (tolower(ch)) {
            case 'p':
                if (g_root == NULL) {
                    show_message("Error: Tree not initialized! Implement TODOs 1-2 first.", 1);
                } else {
                    play_game();
                }
            case 'v':
                draw_tree();
                break;
            case 'u':
                if (undo_last_edit()) {
                    show_message("Undo successful!", 0);
                } else {
                    show_message("Nothing to undo!", 1);
                }
                break;
            case 'r':
                if (redo_last_edit()) {
                    show_message("Redo successful!", 0);
                } else {
                    show_message("Nothing to redo!", 1);
                }
                break;
            case 's':
                if (g_root == NULL) {
                    show_message("Error: No tree to save! Initialize tree first.", 1);
                } else if (save_tree("animals.dat")) {
                    show_message("Tree saved successfully!", 0);
                } else {
                    show_message("Error saving tree!", 1);
                }
                break;
            case 'l':
                if (load_tree("animals.dat")) {
                    show_message("Tree loaded successfully!", 0);
                } else {
                    show_message("Error loading tree!", 1);
                }
                break;
            case 'i':
                if (g_root == NULL) {
                    show_message("Error: No tree to check! Initialize tree first.", 1);
                } else if (check_integrity()) {
                    show_message("Tree integrity check passed!", 0);
                } else {
                    show_message("Tree integrity check failed!", 1);
                }
                break;
            case 'q':
                running = 0;
                break;
        }
    }
    
    endwin();
    free_tree(g_root);
    free_edit_stack(&g_undo);
    free_edit_stack(&g_redo);
    h_free(&g_index);
    
    return 0;
}
