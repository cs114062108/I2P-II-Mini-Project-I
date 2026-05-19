#ifndef __CODEGEN__
#define __CODEGEN__

#include "parser.h"

/* Registers */
#define RMAX 8
extern int registers[RMAX];

// Get register index
extern int get_r_index();

// Evaluate the syntax tree
extern int evaluateTree(BTNode *root);

// Print the syntax tree in prefix
extern void printPrefix(BTNode *root);

#endif // __CODEGEN__
