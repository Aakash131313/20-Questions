#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include "lab5.h"

extern Node *g_root;

#define MAX_DISPLAY_LINES 1000
#define COLOR_TREE_Q 6
#define COLOR_TREE_A 7

typedef struct DisplayLine {
    char *text;
    int indent;
    int isQuestion;
} DisplayLine;

static DisplayLine *lines = NULL;
static int line_count = 0;
static int line_capacity = 0;

void add_display_line(const char *text, int indent, int isQuestion) {
    if (line_count >= line_capacity) {
        line_capacity = line_capacity ? line_capacity * 2 : 100;
        lines = realloc(lines, line_capacity * sizeof(DisplayLine));
    }
    
    lines[line_count].text = strdup(text);
    lines[line_count].indent = indent;
    lines[line_count].isQuestion = isQuestion;
    line_count++;
}

void build_tree_display(Node *node, int depth, const char *prefix, int isYesBranch) {
    if (node == NULL) return;
    
    char line[256];
    char branch[64];
    
    if (depth == 0) {
        snprintf(line, sizeof(line), "ROOT: %s", node->text);
    } else {
        snprintf(branch, sizeof(branch), "%s", isYesBranch ? "[YES]" : "[NO]");
        snprintf(line, sizeof(line), "%s%s %s", prefix, branch, node->text);
    }
    
    add_display_line(line, depth, node->isQuestion);
    
    if (node->isQuestion) {
        char new_prefix[256];
        snprintf(new_prefix, sizeof(new_prefix), "%s  ", prefix);
        
        if (node->yes) {
            build_tree_display(node->yes, depth + 1, new_prefix, 1);
        }
        if (node->no) {
            build_tree_display(node->no, depth + 1, new_prefix, 0);
        }
    }
}

void draw_tree() {
    if (g_root == NULL) {
        clear();
        attron(COLOR_PAIR(5) | A_BOLD);
        mvprintw(0, 0, "%-80s", " Tree Visualization");
        attroff(COLOR_PAIR(5) | A_BOLD);
        
        attron(COLOR_PAIR(4));
        mvprintw(3, 2, "Error: No tree to display!");
        attroff(COLOR_PAIR(4));
        mvprintw(5, 2, "Press any key to return...");
        refresh();
        getch();
        return;
    }
    
    /* Initialize color pairs if not already done */
    init_pair(COLOR_TREE_Q, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_TREE_A, COLOR_GREEN, COLOR_BLACK);
    
    /* Build display lines */
    line_count = 0;
    build_tree_display(g_root, 0, "", 0);
    
    int scroll_offset = 0;
    int max_lines = LINES - 6;
    int running = 1;
    
    while (running) {
        clear();
        
        /* Header */
        attron(COLOR_PAIR(5) | A_BOLD);
        mvprintw(0, 0, "%-80s", " Tree Visualization");
        attroff(COLOR_PAIR(5) | A_BOLD);
        
        /* Draw box */
        int box_height = LINES - 4;
        int box_width = COLS - 2;
        attron(COLOR_PAIR(1));
        mvhline(2, 1, ACS_HLINE, box_width);
        mvhline(2 + box_height - 1, 1, ACS_HLINE, box_width);
        mvvline(2, 1, ACS_VLINE, box_height);
        mvvline(2, box_width, ACS_VLINE, box_height);
        mvaddch(2, 1, ACS_ULCORNER);
        mvaddch(2, box_width, ACS_URCORNER);
        mvaddch(2 + box_height - 1, 1, ACS_LLCORNER);
        mvaddch(2 + box_height - 1, box_width, ACS_LRCORNER);
        attroff(COLOR_PAIR(1));
        
        /* Display tree lines */
        for (int i = 0; i < max_lines && (i + scroll_offset) < line_count; i++) {
            int line_idx = i + scroll_offset;
            DisplayLine *dl = &lines[line_idx];
            
            int color = dl->isQuestion ? COLOR_TREE_Q : COLOR_TREE_A;
            int attr = dl->isQuestion ? A_BOLD : A_NORMAL;
            
            attron(COLOR_PAIR(color) | attr);
            
            /* Truncate line if too long */
            char display[256];
            strncpy(display, dl->text, sizeof(display) - 1);
            display[sizeof(display) - 1] = '\0';
            
            if (strlen(display) > (size_t)(COLS - 6)) {
                display[COLS - 9] = '.';
                display[COLS - 8] = '.';
                display[COLS - 7] = '.';
                display[COLS - 6] = '\0';
            }
            
            mvprintw(3 + i, 3, "%s", display);
            attroff(COLOR_PAIR(color) | attr);
        }
        
        /* Status bar */
        attron(COLOR_PAIR(1));
        mvprintw(LINES - 2, 2, "Lines %d-%d of %d | Use UP/DOWN arrows or j/k to scroll | Q to exit",
                 scroll_offset + 1,
                 (scroll_offset + max_lines < line_count) ? scroll_offset + max_lines : line_count,
                 line_count);
        attroff(COLOR_PAIR(1));
        
        /* Legend */
        attron(COLOR_PAIR(COLOR_TREE_Q) | A_BOLD);
        mvprintw(LINES - 1, 2, "YELLOW=Questions");
        attroff(COLOR_PAIR(COLOR_TREE_Q) | A_BOLD);
        
        attron(COLOR_PAIR(COLOR_TREE_A));
        mvprintw(LINES - 1, 22, "GREEN=Animals");
        attroff(COLOR_PAIR(COLOR_TREE_A));
        
        refresh();
        
        /* Handle input */
        int ch = getch();
        switch (ch) {
            case KEY_UP:
            case 'k':
                if (scroll_offset > 0) scroll_offset--;
                break;
            case KEY_DOWN:
            case 'j':
                if (scroll_offset + max_lines < line_count) scroll_offset++;
                break;
            case KEY_PPAGE:  /* Page Up */
                scroll_offset -= max_lines;
                if (scroll_offset < 0) scroll_offset = 0;
                break;
            case KEY_NPAGE:  /* Page Down */
                scroll_offset += max_lines;
                if (scroll_offset + max_lines > line_count) {
                    scroll_offset = line_count - max_lines;
                    if (scroll_offset < 0) scroll_offset = 0;
                }
                break;
            case 'q':
            case 'Q':
                running = 0;
                break;
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < line_count; i++) {
        free(lines[i].text);
    }
    if (lines) {
        free(lines);
        lines = NULL;
    }
    line_count = 0;
    line_capacity = 0;
}
