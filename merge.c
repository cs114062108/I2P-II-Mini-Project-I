#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// -- lex.h --
#ifndef __LEX__
#define __LEX__

// Set PRINTERR to 1 to print error message while calling error()
// Make sure you set PRINTERR to 0 before you submit your code
#define PRINTERR 0
#define TESTMODE 0
#define SHOWEXPR 0
#define SHOWREGISTERS 0

#define MAXLEN 256

// Token types
typedef enum {
    UNKNOWN, 
    END, // Token: \n
    ENDFILE, // Token: EOF
    INT, ID,
    ADDSUB, // Token: + -
    MULDIV, // Token: * /
    ASSIGN, // Token: =
    LPAREN, RPAREN, // Token: ( or )
    /* New Tokens */
    INCDEC, // New Token: ++ or -- (unary)
    AND, // New Token: &
    OR, // New Token: |
    XOR, // New Token: ^
    ADDSUB_ASSIGN // New Token: += or -=
} TokenSet;

// Peek the next token without consuming it
extern TokenSet peekNextToken(void);

// Test if a token matches the current token 
extern int match(TokenSet token);

// Get the next token
extern void advance(void);

// Get the lexeme of the current token
extern char *getLexeme(void);

#endif // __LEX__

// -- parser.h --
#ifndef __PARSER__
#define __PARSER__

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
extern BTNode *factor(void);        // factor

// Print error message and exit the program
extern void err(ErrorType errorNum);

#endif // __PARSER__

// -- codeGen.h --
#ifndef __CODEGEN__
#define __CODEGEN__

/* Registers */
#define RMAX 8
extern int registers[RMAX];

// Reset register index to 0
extern void reset_r_index();

// Get register index
extern int get_r_index();

// Evaluate the syntax tree
extern int evaluateTree(BTNode *root);

// Print the syntax tree in prefix
extern void printPrefix(BTNode *root);

#endif // __CODEGEN__

// -- lex.c --
static TokenSet getToken(void);
static TokenSet curToken = UNKNOWN;
static char lexeme[MAXLEN];

TokenSet peekNextToken(void) {
    TokenSet token = UNKNOWN;
    
    char c = '\0';

    while ((c = fgetc(stdin)) == ' ' || c == '\t');

    if (c == '=') { // ASSIGN
        token = ASSIGN;
        ungetc(c, stdin);
    } else if (c == '+' || c == '-') { // ADDSUB
        token = ADDSUB;
        char prev_c = c;
        c = fgetc(stdin);
        if ((c == '+' || c == '-') && c == prev_c) { // New: INCDEC
            token = INCDEC;
        } else if (c == '=') { // New: ADDSUB_ASSIGN
            token = ADDSUB_ASSIGN;
        }
        ungetc(c, stdin);
        ungetc(prev_c, stdin);
    } else {
        ungetc(c, stdin);
    }
    return token;
}

TokenSet getToken(void) {
    int i = 0;
    char c = '\0';

    while ((c = fgetc(stdin)) == ' ' || c == '\t');

    if (isdigit(c)) { // INT
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isdigit(c) && i < MAXLEN) {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        if (TESTMODE) printf("INT ");
        return INT;
    } else if (isalpha(c) || c == '_') { // ID: read variable name with more than one letter or has numbers or underscores
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while ((isalpha(c) || isdigit(c) || c == '_') && i < MAXLEN) {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        if (TESTMODE) printf("ID ");
        return ID;
    } else if (c == '+' || c == '-') { // ADDSUB
        lexeme[0] = c;
        char prev_c = c;
        c = fgetc(stdin);
        if ((c == '+' || c == '-') && c == prev_c) { // New: INCDEC
            lexeme[1] = c;
            lexeme[2] = '\0';
            if (TESTMODE) printf("INCDEC ");
            return INCDEC;
        } else if (c == '=') { // New: ADDSUB_ASSIGN
            lexeme[1] = c;
            lexeme[2] = '\0';
            if (TESTMODE) printf("ADDSUB_ASSIGN ");
            return ADDSUB_ASSIGN;
        } else {
            ungetc(c, stdin);
        }
        lexeme[1] = '\0';
        if (TESTMODE) printf("ADDSUB ");
        return ADDSUB;
    } else if (c == '*' || c == '/') { // MULDIV
        lexeme[0] = c;
        lexeme[1] = '\0';
        if (TESTMODE) printf("MULDIV ");
        return MULDIV;
    } else if (c == '\n') { // END
        lexeme[0] = '\0';
        if (TESTMODE) printf("END ");
        return END;
    } else if (c == '=') { // ASSIGN
        strcpy(lexeme, "=");
        if (TESTMODE) printf("ASSIGN ");
        return ASSIGN;
    } else if (c == '(') { // LPAREN
        strcpy(lexeme, "(");
        if (TESTMODE) printf("LPAREN ");
        return LPAREN;
    } else if (c == ')') { // RPAREN
        strcpy(lexeme, ")");
        if (TESTMODE) printf("RPAREN ");
        return RPAREN;
    } else if (c == '&') { // New: AND
        lexeme[0] = c;
        lexeme[1] = '\0';
        if (TESTMODE) printf("AND ");
        return AND;
    } else if (c == '|') { // New: OR
        lexeme[0] = c;
        lexeme[1] = '\0';
        if (TESTMODE) printf("OR ");
        return OR;
    } else if (c == '^') { // New: XOR
        lexeme[0] = c;
        lexeme[1] = '\0';
        if (TESTMODE) printf("XOR ");
        return XOR;
    } else if (c == EOF) { // ENDFILE
        return ENDFILE;
    } else {
        return UNKNOWN;
    }
}

void advance(void) {
    curToken = getToken();
}

int match(TokenSet token) {
    if (curToken == UNKNOWN)
        advance();
    return token == curToken;
}

char *getLexeme(void) {
    return lexeme;
}

// -- parser.c --
int sbcount = 0;
Symbol table[TBLSIZE];

void initTable(void) {
    strcpy(table[0].name, "x");
    table[0].val = 0;
    table[0].pos = 0;
    strcpy(table[1].name, "y");
    table[1].val = 0;
    table[1].pos = 1;
    strcpy(table[2].name, "z");
    table[2].val = 0;
    table[2].pos = 2;
    sbcount = 3;
}

Symbol getsym(char *str) {
    int i = 0;
    
    for (i = 0; i < sbcount; i++)
        if (strcmp(str, table[i].name) == 0)
            return table[i];

    if (sbcount >= TBLSIZE)
        error(RUNOUT);
    
    // shoud not be able to access undefined variable, call error() instead
    error(NOTFOUND);
    strcpy(table[sbcount].name, str);
    table[sbcount].val = 0;
    table[sbcount].pos = sbcount;
    //sbcount++;
    return table[sbcount++];
}

int getval(char *str) {
    int i = 0;
    
    for (i = 0; i < sbcount; i++)
        if (strcmp(str, table[i].name) == 0)
            return table[i].val;

    if (sbcount >= TBLSIZE)
        error(RUNOUT);
    
    // shoud not be able to access undefined variable, call error() instead
    error(NOTFOUND);
    strcpy(table[sbcount].name, str);
    table[sbcount].val = 0;
    table[sbcount].pos = sbcount;
    sbcount++;
    return 0;
}

int setval(char *str, int val) {
    int i = 0;
    
    for (i = 0; i < sbcount; i++) {
        if (strcmp(str, table[i].name) == 0) {
            table[i].val = val;
            //return val;
            return i;
        }
    }
    
    if (sbcount >= TBLSIZE)
        error(RUNOUT);
    
    // set new value for new variable
    strcpy(table[sbcount].name, str);
    table[sbcount].val = val;
    table[sbcount].pos = sbcount;
    sbcount++;
    //return val;
    return i;
}

BTNode *makeNode(TokenSet tok, const char *lexe) {
    BTNode *node = (BTNode*)malloc(sizeof(BTNode));
    strcpy(node->lexeme, lexe);
    node->data = tok;
    node->val = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void freeTree(BTNode *root) {
    if (root != NULL) {
        freeTree(root->left);
        freeTree(root->right);
        free(root);
    }
}

// factor := INT | ID | INCDEC ID | LPAREN assign_expr RPAREN
// factor := INT | ADDSUB INT |
//		   	 ID  | ADDSUB ID  | 
//		   	 ID ASSIGN expr |
//		   	 LPAREN expr RPAREN |
//		   	 ADDSUB LPAREN expr RPAREN
BTNode *factor(void) {
    BTNode *retp = NULL, *left = NULL;
    if (TESTMODE) printf("factor( ");
    if (match(INT)) { // INT
        //if (TESTMODE) printf("INT ");
        retp = makeNode(INT, getLexeme());
        advance();
    } else if (match(ID)) { // ID
        //if (TESTMODE) printf("ID ");
        left = makeNode(ID, getLexeme());
        advance();
        retp = left;
    } else if (match(INCDEC)) { // INCDEC ID
        //if (TESTMODE) printf("INCDEC ");
        retp = makeNode(INCDEC, getLexeme());
        advance();
        left = makeNode(ID, getLexeme());
        advance();
        retp->left = left;
    } else if (match(LPAREN)) { // LPAREN assign_expr RPAREN
        if (TESTMODE) printf("factorLP ");
        advance();
        //retp = addsub_expr();
        retp = assign_expr();
        if (match(RPAREN)) {
            if (TESTMODE) printf("factorRP ");
            advance();
        } else {
            if (TESTMODE) printf("factorOHNO ");
            error(MISPAREN);
        }
    } else {
        error(NOTNUMID);
    }
    if (TESTMODE) printf(") ");
    return retp;
}

// unary_expr   := ADDSUB unary_expr | factor
BTNode *unary_expr(void) {
    BTNode *retp = NULL;
    if (TESTMODE && SHOWEXPR) printf("unary( ");
    if (match(ADDSUB)) {
        //if (TESTMODE) printf("ADDSUB ");
        retp = makeNode(ADDSUB, getLexeme());
        advance();
        retp->left = makeNode(INT, "0"); // treat unary + or - as binary with left operand 0
        retp->right = unary_expr();
    } else {
        retp = factor();
    }
    if (TESTMODE && SHOWEXPR) printf(") ");
    return retp;
}

//muldiv_expr         := unary_expr muldiv_expr_tail
//muldiv_expr_tail    := MULDIV unary_expr muldiv_expr_tail | NiL
BTNode *muldiv_expr(void) {
    BTNode *retp = NULL, *left = NULL;
    if (TESTMODE && SHOWEXPR) printf("muldiv( ");
    retp = left = unary_expr();
    while (match(MULDIV)) {
        //if (TESTMODE) printf("MULDIV ");
        retp = makeNode(MULDIV, getLexeme());
        advance();
        retp->left = left;
        retp->right = unary_expr();
        left = retp;
    }
    if (TESTMODE && SHOWEXPR) printf(") ");
    return retp;
}

// addsub_expr      := muldiv_expr addsub_expr_tail
// addsub_expr_tail := ADDSUB muldiv_expr addsub_expr_tail | NiL
BTNode *addsub_expr(void) {
    BTNode *retp = NULL, *left = NULL;
    if (TESTMODE && SHOWEXPR) printf("addsub( ");
    retp = left = muldiv_expr();
    while (match(ADDSUB)) {
        //if (TESTMODE) printf("ADDSUB ");
        retp = makeNode(ADDSUB, getLexeme());
        advance();
        retp->left = left;
        retp->right = muldiv_expr();
        left = retp;
    }
    if (TESTMODE && SHOWEXPR) printf(") ");
    return retp;
}

// and_expr            := addsub_expr and_expr_tail
// and_expr_tail       := AND addsub_expr and_expr_tail | NiL
BTNode *and_expr(void) {
    BTNode *retp = NULL, *left = NULL;
    if (TESTMODE && SHOWEXPR) printf("and( ");
    retp = left = addsub_expr();
    while (match(AND)) {
        //if (TESTMODE) printf("AND ");
        retp = makeNode(AND, getLexeme());
        advance();
        retp->left = left;
        retp->right = addsub_expr();
        left = retp;
    }
    if (TESTMODE && SHOWEXPR) printf(") ");
    return retp;
}

// xor_expr            := and_expr xor_expr_tail
// xor_expr_tail       := XOR and_expr xor_expr_tail | NiL
BTNode *xor_expr(void) {
    BTNode *retp = NULL, *left = NULL;
    if (TESTMODE && SHOWEXPR) printf("xor( ");
    retp = left = and_expr();
    while (match(XOR)) {
        //if (TESTMODE) printf("XOR ");
        retp = makeNode(XOR, getLexeme());
        advance();
        retp->left = left;
        retp->right = and_expr();
        left = retp;
    }
    if (TESTMODE && SHOWEXPR) printf(") ");
    return retp;
}

// or_expr             := xor_expr or_expr_tail
// or_expr_tail        := OR xor_expr or_expr_tail | NiL
BTNode *or_expr(void) {
    BTNode *retp = NULL, *left = NULL;
    if (TESTMODE && SHOWEXPR) printf("or( ");
    retp = left = xor_expr();
    while (match(OR)) {
        //if (TESTMODE) printf("OR ");
        retp = makeNode(OR, getLexeme());
        advance();
        retp->left = left;
        retp->right = xor_expr();
        left = retp;
    }
    if (TESTMODE && SHOWEXPR) printf(") ");
    return retp;
}

// assign_expr := ID ASSIGN assign_expr | ID ADDSUB_ASSIGN assign_expr | or_expr 
BTNode *assign_expr(void) {
    BTNode *retp = NULL, *left = NULL;

    TokenSet nextToken = peekNextToken();
    if (TESTMODE) printf("assign( ");
    if (match(ID) && (nextToken == ASSIGN || nextToken == ADDSUB_ASSIGN)) {
        //if (TESTMODE) printf("ID ");
        left = makeNode(ID, getLexeme());
        advance();
        retp = left;
        if (match(ASSIGN)) {
            //if (TESTMODE) printf("ASSIGN ");
            retp = makeNode(ASSIGN, getLexeme());
            advance();
            retp->left = left;
            retp->right = assign_expr();
            //retp->right = or_expr();
        } else if (match(ADDSUB_ASSIGN)) {
            //if (TESTMODE) printf("ADDSUB_ASSIGN ");
            retp = makeNode(ADDSUB_ASSIGN, getLexeme());
            advance();
            retp->left = left;
            retp->right = assign_expr();
            //retp->right = or_expr();
        }
    } else {
        retp = left = or_expr();
    }
    if (TESTMODE) printf(") ");
    return retp;
}

// statement := ENDFILE | END | assign_expr END
// statement := ENDFILE | END | expr END
void statement(void) {
    BTNode *retp = NULL;
    
    if (match(ENDFILE)) {
        for (int i = 0; i < 3; i++) {
            printf("MOV r%d [%d]\n", i, 4*i);
        }
        printf("EXIT 0\n");
        exit(0);
    } else if (match(END)) {
        if (TESTMODE) printf(">> ");
        advance();
    } else {
        //retp = addsub_expr();
        retp = assign_expr();
        if (match(END)) {
            reset_r_index(); // reset register index for each statement
            if (TESTMODE) printf("\n");
            if (TESTMODE) {
                printf("Result: %d\n", evaluateTree(retp));
            } else {
                evaluateTree(retp);
            }
            if (TESTMODE) printf("Prefix traversal: ");
            if (TESTMODE) printPrefix(retp);
            if (TESTMODE) printf("\n");
            freeTree(retp);
            if (TESTMODE && SHOWREGISTERS) {
                for (int i = 0; i < RMAX; i++) {
                    printf("r[%d] = %d\n", i, registers[i]);
                }
            }
            if (TESTMODE) printf(">> ");
            advance();
        } else {
            error(SYNTAXERR);
        }
    }
}

void err(ErrorType errorNum) {
    if (PRINTERR) {
        fprintf(stderr, "error: ");
        switch (errorNum) {
            case MISPAREN:
                fprintf(stderr, "mismatched parenthesis\n");
                break;
            case NOTNUMID:
                fprintf(stderr, "number or identifier expected\n");
                break;
            case NOTFOUND:
                fprintf(stderr, "variable not defined\n");
                break;
            case RUNOUT:
                fprintf(stderr, "out of memory\n");
                break;
            case NOTLVAL:
                fprintf(stderr, "lvalue required as an operand\n");
                break;
            case DIVZERO:
                fprintf(stderr, "divide by constant zero\n");
                break;
            case SYNTAXERR:
                fprintf(stderr, "syntax error\n");
                break;
            default:
                fprintf(stderr, "undefined error\n");
                break;
        }
    }
    printf("EXIT 1\n");
    exit(0);
}

// -- codeGen.c --
/* Registers */
int registers[RMAX];
static int r_index = 0; // next index of registers to use
static int return_index = -1;

// all int no ID as value in the tree
bool all_int(BTNode *root) {
    if (root == NULL) return true;
    if (root->data == ID) return false;
    return all_int(root->left) && all_int(root->right);
}

void reset_r_index() {
    r_index = 0;
}

int get_r_index() {
    return r_index;
}

int pos_to_memory(int pos) {
    return sizeof(int) * pos;
}

// Set rigister with value and return the register index, print MOV instruction
int setRegister(int value) {
    if (r_index >= RMAX) err(RUNOUT);
    registers[r_index] = value;
    printf("MOV r%d ", r_index);
    return r_index++;
}

int operRegister(int lr, int rr, int value) {
    registers[lr] = value;
    if (rr+1 == r_index) r_index = rr;
    return lr;
}

int evaluateTree(BTNode *root) {
    int retval = 0, lv = 0, rv = 0;
    int lridx = 0, rridx = 0; // register index for left and right value
    int lmpos = 0, rmpos = 0; // memory position
    Symbol sym;

    if (root != NULL) {
        return_index = -1;
        switch (root->data) {
            case ID:
                sym = getsym(root->lexeme);
                retval = sym.val;
                return_index = setRegister(retval);
                printf("[%d]", pos_to_memory(sym.pos));
                if (TESTMODE) printf("\t(%s)", root->lexeme);
                printf("\n");
                break;
            case INT:
                retval = atoi(root->lexeme);
                return_index = setRegister(retval);
                printf("%d\n", retval);
                break;
            case INCDEC: // New: ++ or -- (unary)
                // id part
                sym = getsym(root->left->lexeme);
                lv = sym.val;
                lmpos = sym.pos;
                lridx = setRegister(lv);
                printf("[%d]", pos_to_memory(lmpos));
                if (TESTMODE) printf("\t(%s) ", root->left->lexeme);
                printf("\n");
                // addsub part
                rridx = setRegister(1); printf("1\n");
                if (strcmp(root->lexeme, "++") == 0) {
                    lv += 1;
                    printf("ADD ");
                } else if (strcmp(root->lexeme, "--") == 0) {
                    lv -= 1;
                    printf("SUB ");
                }
                return_index = operRegister(lridx, rridx, lv);
                printf("r%d r%d\n", lridx, rridx);
                // assign part
                rridx = return_index;
                lmpos = setval(root->left->lexeme, lv);
                retval = lv;
                printf("MOV [%d] ", pos_to_memory(lmpos));
                printf("r%d", rridx);
                if (TESTMODE) printf("\t(%s)", root->left->lexeme);
                printf("\n");
                break;
            case ASSIGN:
                rv = evaluateTree(root->right);
                rridx = return_index;
                lv = rv;
                lmpos = setval(root->left->lexeme, lv);
                retval = lv;
                printf("MOV [%d] ", pos_to_memory(lmpos));
                printf("r%d", rridx);
                if (TESTMODE) printf("\t(%s)", root->left->lexeme);
                printf("\n");
                break;
            case ADDSUB_ASSIGN: // NEW: += or -=
                if (root->left->data != ID) {
                    error(NOTLVAL);
                }
                // addsub part
                lv = evaluateTree(root->left);
                lridx = return_index;
                rv = evaluateTree(root->right);
                rridx = return_index;
                if (strcmp(root->lexeme, "+=") == 0) {
                    lv += rv;
                    printf("ADD ");
                } else if (strcmp(root->lexeme, "-=") == 0) {
                    lv -= rv;
                    printf("SUB ");
                }
                return_index = operRegister(lridx, rridx, lv);
                printf("r%d r%d\n", lridx, rridx);
                // assign part
                lmpos = setval(root->left->lexeme, lv);
                retval = lv;
                printf("MOV [%d] ", pos_to_memory(lmpos));
                printf("r%d", return_index);
                if (TESTMODE) printf("\t(%s)", root->left->lexeme);
                printf("\n");
                break;
            case ADDSUB:
            case MULDIV:
            case AND: // New
            case OR: // New
            case XOR: // New
                lv = evaluateTree(root->left);
                lridx = return_index;
                rv = evaluateTree(root->right);
                rridx = return_index;
                if (strcmp(root->lexeme, "+") == 0) {
                    retval = lv + rv;
                    printf("ADD ");
                } else if (strcmp(root->lexeme, "-") == 0) {
                    retval = lv - rv;
                    printf("SUB ");
                } else if (strcmp(root->lexeme, "*") == 0) {
                    retval = lv * rv;
                    printf("MUL ");
                } else if (strcmp(root->lexeme, "/") == 0) {
                    if (rv == 0 && all_int(root->right)) { // exit with error if divide by zero only when the right subtree is all integer
                        error(DIVZERO);
                    }
                    if (rv == 0) { // if divide by zero but the right node is not an integer, just set retval to 0 without exit
                        retval = 0;
                    } else {
                        retval = lv / rv;
                    }
                    printf("DIV ");
                } else if (strcmp(root->lexeme, "&") == 0) { // New: AND
                    retval = lv & rv;
                    printf("AND ");
                } else if (strcmp(root->lexeme, "|") == 0) { // New: OR
                    retval = lv | rv;
                    printf("OR ");
                } else if (strcmp(root->lexeme, "^") == 0) { // New: XOR
                    retval = lv ^ rv;
                    printf("XOR ");
                }
                return_index = operRegister(lridx, rridx, retval);
                printf("r%d r%d\n", lridx, rridx);
                break;
            case END:
                if (TESTMODE) printf("debugcatcapoo\n");
            default:
                retval = 0;
        }
    }
    return retval;
}

void printPrefix(BTNode *root) {
    if (root != NULL) {
        printf("%s ", root->lexeme);
        printPrefix(root->left);
        printPrefix(root->right);
    }
}


// -- main.c --
int main() {
    initTable();
    if (TESTMODE) printf(">> ");
    while (true) {
        statement();
    }
    return 0;
}