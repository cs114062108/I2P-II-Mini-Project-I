#ifndef __PARSER__
#define __PARSER__

#include "lex.h"
#define TBLSIZE 64

// Set PRINTERR to 1 to print error message while calling error()
// Make sure you set PRINTERR to 0 before you submit your code
#define PRINTERR 1
#define TESTMODE 1
#define SHOWREGISTERS 0

// Call this macro to print error message and exit the program
// This will also print where you called it in your program
#define error(errorNum) { \
    if (PRINTERR) \
        fprintf(stderr, "error() called at %s:%d: ", __FILE__, __LINE__); \
    err(errorNum); \
}

// Error types
typedef enum {
    UNDEFINED,
    MISPAREN,   // mismatched parentheses
    NOTNUMID,   // not a number or identifier
    NOTFOUND,   // variable not defined
    RUNOUT,     // out of memory or registers
    NOTLVAL,    // lvalue required as an operand
    DIVZERO,    // divide by constant zero
    SYNTAXERR
} ErrorType;

// Structure of the symbol table
typedef struct _Symbol {
    int val;
    char name[MAXLEN];
    int pos;
} Symbol;

// Structure of a tree node
typedef struct _Node {
    TokenSet data;
    int val;
    char lexeme[MAXLEN];
    struct _Node *left; 
    struct _Node *right;
} BTNode;

// The symbol table
extern Symbol table[TBLSIZE];

// Initialize the symbol table with builtin variables
extern void initTable(void);

// Get the symbol object of a variable
extern Symbol getsym(char *str);

// Get the value of a variable
extern int getval(char *str);

// Set the value of a variable, and return its position
extern int setval(char *str, int val);

// Make a new node according to token type and lexeme
extern BTNode *makeNode(TokenSet tok, const char *lexe);

// Free the syntax tree
extern void freeTree(BTNode *root);

extern BTNode *factor(void);
extern BTNode *muldiv_expr(void); // muldiv
extern BTNode *expr(void); // addsub
extern void statement(void);

// Print error message and exit the program
extern void err(ErrorType errorNum);

#endif // __PARSER__
