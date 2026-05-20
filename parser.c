#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "codeGen.h"

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
    
    if (match(INT)) { // INT
        //if (TESTMODE) printf("INT ");
        retp = makeNode(INT, getLexeme());
        advance();
    } else if (match(ID)) { // ID
        //if (TESTMODE) printf("ID ");
        left = makeNode(ID, getLexeme());
        advance();
        retp = left;
        if (!match(ASSIGN)) {
            retp = left;
        } else {
            retp = makeNode(ASSIGN, getLexeme());
            advance();
            retp->left = left;
            retp->right = addsub_expr();
        }
    } else if (match(INCDEC)) { // INCDEC ID
        //if (TESTMODE) printf("INCDEC ");
        left = makeNode(ID, getLexeme());
        advance();
        retp = makeNode(INCDEC, getLexeme());
        retp->left = left;
    } else if (match(LPAREN)) { // LPAREN assign_expr RPAREN
        advance();
        //retp = addsub_expr();
        retp = assign_expr();
        if (match(RPAREN))
            advance();
        else
            error(MISPAREN);
    } else {
        error(NOTNUMID);
    }
    return retp;
}

// muldiv           := factor muldiv_expr_tail
// muldiv_expr_tail := MULDIV factor muldiv_expr_tail | NiL
BTNode *muldiv_expr(void) {
    BTNode *retp = NULL, *left = NULL;
    if (TESTMODE && SHOWEXPR) printf("muldiv( ");
    retp = left = factor();
    while (match(MULDIV)) {
        //if (TESTMODE) printf("MULDIV ");
        retp = makeNode(MULDIV, getLexeme());
        advance();
        retp->left = left;
        retp->right = factor();
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
    return retp;
}

// statement := ENDFILE | END | assign_expr END
// statement := ENDFILE | END | expr END
void statement(void) {
    BTNode *retp = NULL;
    
    if (match(ENDFILE)) {
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
                printf("%d\n", evaluateTree(retp));
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
