#ifndef __PARSER__
#define __PARSER__

#include "lex.h"
#define TBLSIZE 64

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

/* EXPRESSION PARSING */
/*
statement           := ENDFILE | END | assign_expr END
assign_expr         := ID ASSIGN assign_expr | ID ADDSUB_ASSIGN assign_expr | or_expr
or_expr             := xor_expr or_expr_tail
or_expr_tail        := OR xor_expr or_expr_tail | NiL
xor_expr            := and_expr xor_expr_tail
xor_expr_tail       := XOR and_expr xor_expr_tail | NiL
and_expr            := addsub_expr and_expr_tail
and_expr_tail       := AND addsub_expr and_expr_tail | NiL
addsub_expr         := muldiv_expr addsub_expr_tail
addsub_expr_tail    := ADDSUB muldiv_expr addsub_expr_tail | NiL
muldiv_expr         := unary_expr muldiv_expr_tail
muldiv_expr_tail    := MULDIV unary_expr muldiv_expr_tail | NiL
unary_expr          := ADDSUB unary_expr | factor
factor              := INT | ID | INCDEC ID | LPAREN assign_expr RPAREN
*/
extern void statement(void);
extern BTNode *assign_expr(void);   // assign
extern BTNode *or_expr(void);       // or
extern BTNode *xor_expr(void);      // xor
extern BTNode *and_expr(void);      // and
extern BTNode *addsub_expr(void);   // addsub
extern BTNode *muldiv_expr(void);   // muldiv
extern BTNode *unary_expr(void);    // unary
extern BTNode *factor(void);        // factor

// Print error message and exit the program
extern void err(ErrorType errorNum);

#endif // __PARSER__
