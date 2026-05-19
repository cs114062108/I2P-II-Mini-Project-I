#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGen.h"

/* Registers */
int registers[RMAX];
static int r_index = 0; // next index of registers to use
static int return_index = -1;

int get_r_index() {
    return r_index;
}

int pos_to_memory(int pos) {
    return sizeof(int) * pos;
}

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
    int lrpos = 0, rrpos = 0; // register position
    int lmpos = 0, rmpos = 0; // memory position
    Symbol sym;

    if (root != NULL) {
        return_index = -1;
        switch (root->data) {
            case ID:
                sym = getsym(root->lexeme);
                retval = sym.val;
                //retval = getval(root->lexeme);
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
            case ASSIGN:
                rv = evaluateTree(root->right);
                lmpos = setval(root->left->lexeme, rv);
                retval = rv;
                printf("MOV [%d] ", pos_to_memory(lmpos));
                printf("r%d", return_index);
                if (TESTMODE) printf("\t(%s)", root->left->lexeme);
                printf("\n");
                r_index = 0;
                //return_index = 0;
                break;
            case ADDSUB:
            case MULDIV:
            case AND: // New
            case OR: // New
            case XOR: // New
                lv = evaluateTree(root->left);
                lrpos = return_index;
                rv = evaluateTree(root->right);
                rrpos = return_index;
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
                    if (rv == 0)
                    error(DIVZERO);
                    retval = lv / rv;
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
                return_index = operRegister(lrpos, rrpos, retval);
                printf("r%d r%d\n", lrpos, rrpos);
                break;
            case END:
                printf("debugcatcappo\n");
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
