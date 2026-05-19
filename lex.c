#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lex.h"

static TokenSet getToken(void);
static TokenSet curToken = UNKNOWN;
static char lexeme[MAXLEN];

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
        return INT;
    } else if (isalpha(c)) { // ID: read variable name with more than one letter or has numbers or underscores
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
        return ID;
    } else if (c == '+' || c == '-') { // ADDSUB
        lexeme[0] = c;
        c = fgetc(stdin);
        if ((c == '+' || c == '-') && c == lexeme[0]) { // New: INCDEC
            lexeme[1] = c;
            lexeme[2] = '\0';
            return INCDEC;
        } else if (c == '=') { // New: ADDSUB_ASSIGN
            lexeme[1] = c;
            lexeme[2] = '\0';
            return ADDSUB_ASSIGN;
        } else {
            ungetc(c, stdin);
        }
        lexeme[1] = '\0';
        return ADDSUB;
    } else if (c == '*' || c == '/') { // MULDIV
        lexeme[0] = c;
        lexeme[1] = '\0';
        return MULDIV;
    } else if (c == '\n') { // END
        lexeme[0] = '\0';
        return END;
    } else if (c == '=') { // ASSIGN
        strcpy(lexeme, "=");
        return ASSIGN;
    } else if (c == '(') { // LPAREN
        strcpy(lexeme, "(");
        return LPAREN;
    } else if (c == ')') { // RPAREN
        strcpy(lexeme, ")");
        return RPAREN;
    } else if (c == '&') { // New: AND
        lexeme[0] = c;
        lexeme[1] = '\0';
        return AND;
    } else if (c == '|') { // New: OR
        lexeme[0] = c;
        lexeme[1] = '\0';
        return OR;
    } else if (c == '^') { // New: XOR
        lexeme[0] = c;
        lexeme[1] = '\0';
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
