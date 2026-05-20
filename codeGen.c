#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGen.h"

/* Registers */
int registers[RMAX];
static int r_index = 0; // next index of registers to use
static int return_index = -1;

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
