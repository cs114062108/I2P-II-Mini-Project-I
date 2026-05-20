#ifndef __LEX__
#define __LEX__

// Set PRINTERR to 1 to print error message while calling error()
// Make sure you set PRINTERR to 0 before you submit your code
#define PRINTERR 1
#define TESTMODE 1
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
