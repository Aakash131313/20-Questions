#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "lab5.h"

extern Node *g_root;

#define MAGIC 0x41544C35  /* "ATL5" */
#define VERSION 1

typedef struct {
    Node *node;
    int id;
} NodeMapping;


//helpers
static int find_id(NodeMapping *map, int mcount, Node *n) {
    for (int i = 0; i < mcount; i++) { //goes through and scans the mapping
        if (map[i].node == n) return map[i].id; //returns id if matched
    }
    return -1; //means not found
}

static int ensure_map_capacity(NodeMapping **map, int *cap, int need) {
    if (need <= *cap) return 1; //checks if there is enough space
    int newcap = (*cap == 0) ? 64 : *cap; //both starts or grows the capacity
    while (newcap < need) newcap *= 2;
    NodeMapping *tmp = realloc(*map, (size_t)newcap * sizeof(NodeMapping)); //resizing
    if (!tmp) return 0;
    *map = tmp; //installs the new buffer
    *cap = newcap; //updates the cap
    return 1;
}

/* TODO 27: Implement save_tree
 * Save the tree to a binary file using BFS traversal
 * 
 * Binary format:
 * - Header: magic (4 bytes), version (4 bytes), nodeCount (4 bytes)
 * - For each node in BFS order:
 *   - isQuestion (1 byte)
 *   - textLen (4 bytes)
 *   - text (textLen bytes, no null terminator)
 *   - yesId (4 bytes, -1 if NULL)
 *   - noId (4 bytes, -1 if NULL)
 * 
 * Steps:
 * 1. Return 0 if g_root is NULL
 * 2. Open file for writing binary ("wb")
 * 3. Initialize queue and NodeMapping array
 * 4. Use BFS to assign IDs to all nodes:
 *    - Enqueue root with id=0
 *    - Store mapping[0] = {g_root, 0}
 *    - While queue not empty:
 *      - Dequeue node and id
 *      - If node has yes child: add to mappings, enqueue with new id
 *      - If node has no child: add to mappings, enqueue with new id
 * 5. Write header (magic, version, nodeCount)
 * 6. For each node in mapping order:
 *    - Write isQuestion, textLen, text bytes
 *    - Find yes child's id in mappings (or -1)
 *    - Find no child's id in mappings (or -1)
 *    - Write yesId, noId
 * 7. Clean up and return 1 on success
 */
int save_tree(const char *filename) {
    // TODO: Implement this function
    // This is complex - break it into smaller steps
    // You'll need to use the Queue functions you implemented
    if (!g_root) return 0;

    FILE *fp = fopen(filename, "wb"); //open to write
    if (!fp) return 0;

    //  BFS to assign ids
    Queue q; //queue for BFS
    q_init(&q);

    NodeMapping *map = NULL; //dynamically mapping array
    int mcap = 0, mcount = 0;

    if (!ensure_map_capacity(&map, &mcap, 1)) { fclose(fp); q_free(&q); return 0; } //ensures the slot

    q_enqueue(&q, g_root, 0); //sets map root -> 0
    map[mcount++] = (NodeMapping){ g_root, 0 };

    Node *cur; //BFS node
    int cid; //BFS id
    while (q_dequeue(&q, &cur, &cid)) {
        //visits yes child, ensures the slot is open, assigns id, enqueues the child, and adds to the count
        if (cur->yes) {
            if (!ensure_map_capacity(&map, &mcap, mcount + 1)) { fclose(fp); q_free(&q); free(map); return 0; }
            map[mcount] = (NodeMapping){ cur->yes, mcount };
            q_enqueue(&q, cur->yes, mcount);
            mcount++;
        }
        if (cur->no) {
            //same thing but with no child
            if (!ensure_map_capacity(&map, &mcap, mcount + 1)) { fclose(fp); q_free(&q); free(map); return 0; }
            map[mcount] = (NodeMapping){ cur->no, mcount };
            q_enqueue(&q, cur->no, mcount);
            mcount++;
        }
    }
    q_free(&q); //frees the queue when done

    // header
    int32_t magic = (int32_t)MAGIC;
    int32_t version = (int32_t)VERSION;
    int32_t count = (int32_t)mcount;

    //write magic, version, count, and bails if input or output error
    if (fwrite(&magic, sizeof(int32_t), 1, fp) != 1 ||
        fwrite(&version, sizeof(int32_t), 1, fp) != 1 ||
        fwrite(&count, sizeof(int32_t), 1, fp) != 1) {
        fclose(fp); free(map); return 0;
    }

    // nodes in mapping order
    for (int i = 0; i < mcount; i++) {
        Node *n = map[i].node; //cur node

        uint8_t isQ = (uint8_t)(n->isQuestion ? 1 : 0); //type checked
        int32_t textLen = (int32_t)strlen(n->text); //text length
        int32_t yesId = n->yes ? (int32_t)find_id(map, mcount, n->yes) : -1; //yes link id
        int32_t noId  = n->no  ? (int32_t)find_id(map, mcount, n->no)  : -1; //no link id

        if (fwrite(&isQ, sizeof(uint8_t), 1, fp) != 1 || //write type
            fwrite(&textLen, sizeof(int32_t), 1, fp) != 1 || //write length
            fwrite(n->text, 1, (size_t)textLen, fp) != (size_t)textLen || //write text
            fwrite(&yesId, sizeof(int32_t), 1, fp) != 1 || //write yes id
            fwrite(&noId, sizeof(int32_t), 1, fp) != 1) { //write no id
            fclose(fp); free(map); return 0; //if input or output fails, bail
        }
    }

    fclose(fp); //close file, free map
    free(map);
    return 1;
}

/* TODO 28: Implement load_tree
 * Load a tree from a binary file and reconstruct the structure
 * 
 * Steps:
 * 1. Open file for reading binary ("rb")
 * 2. Read and validate header (magic, version, count)
 * 3. Allocate arrays for nodes and child IDs:
 *    - Node **nodes = calloc(count, sizeof(Node*))
 *    - int32_t *yesIds = calloc(count, sizeof(int32_t))
 *    - int32_t *noIds = calloc(count, sizeof(int32_t))
 * 4. Read each node:
 *    - Read isQuestion, textLen
 *    - Validate textLen (e.g., < 10000)
 *    - Allocate and read text string (add null terminator!)
 *    - Read yesId, noId
 *    - Validate IDs are in range [-1, count)
 *    - Create Node and store in nodes[i]
 * 5. Link nodes using stored IDs:
 *    - For each node i:
 *      - If yesIds[i] >= 0: nodes[i]->yes = nodes[yesIds[i]]
 *      - If noIds[i] >= 0: nodes[i]->no = nodes[noIds[i]]
 * 6. Free old g_root if not NULL
 * 7. Set g_root = nodes[0]
 * 8. Clean up temporary arrays
 * 9. Return 1 on success
 * 
 * Error handling:
 * - If any read fails or validation fails, goto load_error
 * - In load_error: free all allocated memory and return 0
 */
int load_tree(const char *filename) {
    // TODO: Implement this function
    // This is the most complex function in the lab
    // Take it step by step and test incrementally
    FILE *fp = fopen(filename, "rb"); //open for read
    if (!fp) return 0;

    int32_t magic = 0, version = 0, count = 0; //header fields
    //read magic, version, count
    if (fread(&magic, sizeof(int32_t), 1, fp) != 1 ||
        fread(&version, sizeof(int32_t), 1, fp) != 1 ||
        fread(&count, sizeof(int32_t), 1, fp) != 1) {
        fclose(fp);
        return 0;
    }
    //if invalid header bail
    if (magic != (int32_t)MAGIC || version != (int32_t)VERSION || count <= 0) {
        fclose(fp);
        return 0;
    }

    Node **nodes = (Node **)calloc((size_t)count, sizeof(Node *)); //node array linked
    int32_t *yesIds = (int32_t *)calloc((size_t)count, sizeof(int32_t)); //yes links
    int32_t *noIds  = (int32_t *)calloc((size_t)count, sizeof(int32_t)); //no links
    if (!nodes || !yesIds || !noIds) { //if the above fails it bails
        fclose(fp);
        free(nodes); free(yesIds); free(noIds);
        return 0;
    }

    for (int i = 0; i < count; i++) { //goes through each node record
        uint8_t isQ; //type byte
        int32_t textLen; //text length
        if (fread(&isQ, sizeof(uint8_t), 1, fp) != 1 ||
            fread(&textLen, sizeof(int32_t), 1, fp) != 1) {
            goto load_error; //if header read fails
        }
        if (textLen < 0 || textLen > 10000) goto load_error;

        char *text = (char *)malloc((size_t)textLen + 1); //allocates the text
        if (!text) goto load_error;
        if (fread(text, 1, (size_t)textLen, fp) != (size_t)textLen) {
            free(text);
            goto load_error; //if the body read fails
        }
        text[textLen] = '\0'; //add null terminator

        int32_t yid, nid; //child ids
        if (fread(&yid, sizeof(int32_t), 1, fp) != 1 ||
            fread(&nid, sizeof(int32_t), 1, fp) != 1) {
            free(text);
            goto load_error; //if the link read fails
        }
        if (yid < -1 || yid >= count || nid < -1 || nid >= count) {
            free(text);
            goto load_error; //id is out of range
        }

        Node *n = (Node *)malloc(sizeof(Node)); //allocate the node
        if (!n) { free(text); goto load_error; }
        n->isQuestion = isQ ? 1 : 0; //sets the type
        n->text = text;         /* text already heap-allocated */
        n->yes = NULL; //initializes children
        n->no  = NULL;

        nodes[i] = n; //stores node in array and records child ids
        yesIds[i] = yid;
        noIds[i]  = nid;
    }

    // Link phase
    for (int i = 0; i < count; i++) { //resolves the child links
        if (yesIds[i] >= 0) nodes[i]->yes = nodes[yesIds[i]];
        if (noIds[i]  >= 0) nodes[i]->no  = nodes[noIds[i]];
    }

    // Replace old root
    if (g_root) free_tree(g_root); //frees previous trees
    g_root = nodes[0]; //puts new root

    free(yesIds); //frees the link arrays, node ptr array, closes the file
    free(noIds);
    free(nodes);   /* not freeing the nodes themselves—they're the live tree */
    fclose(fp);
    return 1;

load_error:
    /* free any nodes/text successfully allocated so far */
    for (int i = 0; i < count; i++) { //free text and nodes
        if (nodes && nodes[i]) {
            free(nodes[i]->text);
            free(nodes[i]);
        }
    }
    free(yesIds); //free arrays and close file
    free(noIds);
    free(nodes);
    fclose(fp);
    return 0;
}
