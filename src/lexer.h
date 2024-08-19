#ifndef LEXER_H_
#define LEXER_H_

#include <stdio.h>

enum tokenval {
    ERROR = 256,
    WORD = 257,            /* any token */
    NAME = 258,            /* [a-zA-Z_][0-9a-zA-Z_]* */
    ASSIGNMENT_WORD = 259, /* {NAME}={WORD} */
    NEWLINE = 260,         /* \n */
    IO_NUMBER = 261,       /* [0-9][0-9]* */

    /* operators */
    AND_IF = 262,    /* && */
    OR_IF = 263,     /* || */
    DSEMI = 264,     /* ;; */
    DLESS = 265,     /* << */
    DGREAT = 266,    /* >> */
    LESSAND = 267,   /* <& */
    GREATAND = 268,  /* >& */
    LESSGREAT = 269, /* <> */
    DLESSDASH = 270, /* <<- */
    CLOBBER = 271,   /* >| */

    /* reserved words */
    IF = 272,     /* if */
    THEN = 273,   /* then */
    ELSE = 274,   /* else */
    ELIF = 275,   /* elif */
    FI = 276,     /* fi */
    DO = 277,     /* do */
    DONE = 278,   /* done */
    CASE = 279,   /* case */
    ESAC = 280,   /* esac */
    WHILE = 281,  /* while */
    UNTIL = 282,  /* until */
    FOR = 283,    /* for */
    LBRACE = 284, /* { */
    RBRACE = 285, /* } */
    BANG = 286,   /* ! */
    IN = 287,     /* in */
};

struct token {
    int token;
    char *value;
};

struct lexer {
    FILE *stream;
    int prev;
    int prevprev;
};

struct token lexer_next(struct lexer *);

#endif /* lexer.h */
