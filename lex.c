#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lex.h"

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
