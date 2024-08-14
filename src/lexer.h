#ifndef LEXER_H_
#define LEXER_H_

#include <stdlib.h>

enum token {
    TOK_ILLEGAL,
    TOK_EOF,

    TOK_WORD,
    TOK_ASSIGNMENT_WORD,
    TOK_NAME,
    TOK_NEWLINE,
    TOK_IO_NUMBER,

    /* operators */
    TOK_AND_IF,
    TOK_OR_IF,
    TOK_DSEMI,
    TOK_DLESS,
    TOK_DGREAT,
    TOK_LESSAND,
    TOK_GREATAND,
    TOK_LESSGREAT,
    TOK_DLESSDASH,
    TOK_CLOBBER,
    TOK_PIPE,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LESS,
    TOK_GREAT,
    TOK_AMPERSAND,
    TOK_SEMICOLON,

    /* reserved words */
    TOK_BANG,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_CASE,
    TOK_DO,
    TOK_DONE,
    TOK_ELIF,
    TOK_ELSE,
    TOK_ESAC,
    TOK_FI,
    TOK_FOR,
    TOK_IF,
    TOK_IN,
    TOK_THEN,
    TOK_UNTIL,
    TOK_WHILE,
};

struct lexer {
    char *input;
    size_t input_len;
    size_t position;
    size_t read_position;
    char ch;
};

struct token_pair {
    enum token tok;
    char *value;
};

struct lexer *lexer_create(char *);
void lexer_free(struct lexer *);

struct token_pair lexer_next(struct lexer *);
void lexer_token_pair_free(struct token_pair *);

#endif /* LEXER_H_ */
