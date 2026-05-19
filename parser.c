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

// or_expr := xor_expr or_expr_tail
// or_expr_tail := OR xor_expr or_expr_tail | NiL

// factor := INT | ID | INCDEC ID | LPAREN assign_expr RPAREN
// factor := INT | ADDSUB INT |
//		   	 ID  | ADDSUB ID  | 
//		   	 ID ASSIGN expr |
//		   	 LPAREN expr RPAREN |
//		   	 ADDSUB LPAREN expr RPAREN
BTNode *factor(void) {
    BTNode *retp = NULL, *left = NULL;
    
    if (match(INT)) { // INT
        retp = makeNode(INT, getLexeme());
        advance();
    } else if (match(ID)) { // ID
        left = makeNode(ID, getLexeme());
        advance();
        retp = left;
        if (!match(ASSIGN)) {
            retp = left;
        } else {
            retp = makeNode(ASSIGN, getLexeme());
            advance();
            retp->left = left;
            retp->right = expr();
        }
    } else if (match(ADDSUB)) {
        retp = makeNode(ADDSUB, getLexeme());
        retp->left = makeNode(INT, "0");
        advance();
        if (match(INT)) {
            retp->right = makeNode(INT, getLexeme());
            advance();
        } else if (match(ID)) {
            retp->right = makeNode(ID, getLexeme());
            advance();
        } else if (match(LPAREN)) {
            advance();
            retp->right = expr();
            if (match(RPAREN))
            advance();
            else
            error(MISPAREN);
        } else {
            error(NOTNUMID);
        }
    } else if (match(LPAREN)) {
        advance();
        retp = expr();
        if (match(RPAREN))
        advance();
        else
        error(MISPAREN);
    } else {
        error(NOTNUMID);
    }
    return retp;
}

// muldiv      := factor muldiv_expr_tail
// muldiv_expr_tail := MULDIV factor muldiv_expr_tail | NiL
BTNode *muldiv_expr(void) {
    BTNode *retp = NULL, *left = NULL;
    
    retp = left = factor();
    while (match(MULDIV)) {
        retp = makeNode(MULDIV, getLexeme());
        advance();
        retp->left = left;
        retp->right = factor();
        left = retp;
    }
    return retp;
}

// addsub_expr      := muldiv_expr addsub_expr_tail
// addsub_expr_tail := ADDSUB muldiv_expr addsub_expr_tail | NiL
BTNode *expr(void) {
    BTNode *retp = NULL, *left = NULL;
    
    retp = left = muldiv_expr();
    while (match(ADDSUB)) {
        retp = makeNode(ADDSUB, getLexeme());
        advance();
        retp->left = left;
        retp->right = muldiv_expr();
        left = retp;
    }
    return retp;
}

// assign_expr := ID ASSIGN assign_expr | ID ADDSUB_ASSIGN assign_expr | or_expr 
BTNode *assign_expr(void) {
    BTNode *retp = NULL;
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
        retp = expr();
        if (match(END)) {
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
