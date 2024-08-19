#include "grammar.h"
#include "lexer.h"
#include <stdlib.h>
#include <wctype.h>

void print_token(struct token tok)
{
    switch (tok.token) {
    case ERROR:
        printf("ERROR ");
        break;
    case WORD:
        printf("WORD(%s) ", tok.value);
        break;
    case NAME:
        printf("NAME(%s) ", tok.value);
        break;
    case ASSIGNMENT_WORD:
        printf("ASSIGNMENT_WORD(%s) ", tok.value);
        break;
    case NEWLINE:
        printf("\n");
        break;
    case IO_NUMBER:
        printf("IO_NUMBER(%s) ", tok.value);
        break;
    case AND_IF:
        printf("AND_IF ");
        break;
    case OR_IF:
        printf("OR_IF ");
        break;
    case DSEMI:
        printf("DSEMI ");
        break;
    case DLESS:
        printf("DLESS ");
        break;
    case DGREAT:
        printf("DGREAT ");
        break;
    case LESSAND:
        printf("LESSAND ");
        break;
    case GREATAND:
        printf("GREATAND ");
        break;
    case LESSGREAT:
        printf("LESSGREAT ");
        break;
    case DLESSDASH:
        printf("DLESSDASH ");
        break;
    case CLOBBER:
        printf("CLOBBER ");
        break;
    case IF:
        printf("IF ");
        break;
    case THEN:
        printf("THEN ");
        break;
    case ELSE:
        printf("ELSE ");
        break;
    case ELIF:
        printf("ELIF ");
        break;
    case FI:
        printf("FI ");
        break;
    case DO:
        printf("DO ");
        break;
    case DONE:
        printf("DONE ");
        break;
    case CASE:
        printf("CASE ");
        break;
    case ESAC:
        printf("ESAC ");
        break;
    case WHILE:
        printf("WHILE ");
        break;
    case UNTIL:
        printf("UNTIL ");
        break;
    case FOR:
        printf("FOR ");
        break;
    case LBRACE:
        printf("LBRACE ");
        break;
    case RBRACE:
        printf("RBRACE ");
        break;
    case BANG:
        printf("BANG ");
        break;
    case IN:
        printf("IN ");
        break;
    case EOF:
        printf("DONE\n");
        break;
    default:
        printf("%c ", tok.token);
    }
}

void grammar_parse(struct lexer *lex)
{
    struct token tok;
    do {
        tok = lexer_next(lex);

        print_token(tok);
        if (tok.value != NULL)
            free(tok.value);
    } while (tok.token != EOF);
}
