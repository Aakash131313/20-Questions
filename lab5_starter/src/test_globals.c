/*
 * test_globals.c
 * 
 * This file defines the global variables needed by the test suite.
 * These same variables are defined in main.c for the actual game,
 * but tests.c needs its own copy to avoid linking conflicts.
 * 
 * DO NOT MODIFY THIS FILE.
 */

#include <stdio.h>
#include <stdlib.h>
#include "lab5.h"

/* Global tree root */
Node *g_root = NULL;

/* Global undo/redo stacks */
EditStack g_undo = {NULL, 0, 0};
EditStack g_redo = {NULL, 0, 0};

/* Global attribute index */
Hash g_index = {NULL, 0, 0};
