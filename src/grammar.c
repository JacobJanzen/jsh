#include "grammar.h"
#include "lexer.h"
#include <stdlib.h>

void grammar_parse(struct lexer *lex)
{
    struct token tok;
    do {
        tok = lexer_next(lex);
        if (tok.value != NULL)
            free(tok.value);
    } while (tok.token != EOF);
}
