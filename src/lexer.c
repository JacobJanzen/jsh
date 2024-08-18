#include "lexer.h"
#include "grammar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_CAPACITY 16

enum delimiter {
    DEL_DEFAULT,
    DEL_PAR,
    DEL_DPAR,
    DEL_BRACE,
    DEL_QUOTE,
    DEL_DQUOTE,
    DEL_BQUOTE,
};

struct string {
    char *container;
    size_t size;
    size_t capacity;
};

FILE *file;

static void string_append(struct string *s, char c)
{
    if (s->size == s->capacity) {
        s->capacity *= 2;
        s->container = realloc(s->container, s->capacity);
    }
    s->container[s->size++] = c;
}

static int is_blank(char c)
{
    return c == ' ' || c == '\t' || c == '\f' || c == '\v' || c == '\r';
}

static void delimit_token(struct string *str, enum delimiter del)
{
    while (1) {
        int c = getc(file);

        switch (c) {
        case EOF: /* exit on EOF */
            return;
        case '\\': /* handle escape character */
            c = getc(file);
            switch (c) {
            case EOF:
                return;
            case '\n':
                break;
            default:
                string_append(str, c);
            }
        case '\'': /* enter/exit single quote */
            string_append(str, c);
            if (del == DEL_QUOTE)
                return;
            if (del != DEL_DQUOTE)
                delimit_token(str, DEL_QUOTE);
            break;
        case '\"': /* enter/exit double quote */
            string_append(str, c);
            if (del == DEL_DQUOTE)
                return;
            if (del != DEL_QUOTE)
                delimit_token(str, DEL_DQUOTE);
            break;
        case '`': /* enter/exit backquote */
            string_append(str, c);
            if (del == DEL_BQUOTE)
                return;
            if (del != DEL_QUOTE)
                delimit_token(str, DEL_BQUOTE);
            break;
        case '$':
            string_append(str, c);
            c = getc(file);
            switch (c) {
            case '{': /* enter paramter expansion */
                string_append(str, c);
                delimit_token(str, DEL_BRACE);
                break;
            case '(':
                string_append(str, c);
                c = getc(file);
                if (c == '(') { /* enter arithmetic expansion */
                    string_append(str, c);
                    delimit_token(str, DEL_DPAR);
                } else { /* enter command substitution */
                    ungetc(c, file);
                    delimit_token(str, DEL_PAR);
                }
                break;
            default:
                ungetc(c, file);
            }
            break;
        case ')':
            if (del == DEL_DEFAULT) /* special character; exit by default */
                return;
            string_append(str, c);
            if (del == DEL_DPAR) { /* exit arithmetic expansion */
                delimit_token(str, DEL_PAR);
                return;
            }
            if (del == DEL_PAR) /* exit command substitution */
                return;
            break;
        case '}': /* exit parameter expansion */
            string_append(str, c);
            if (del == DEL_BRACE)
                return;
            break;
        case '#': /* enter comment */
            if (del == DEL_DEFAULT || del == DEL_PAR) {
                while (c != '\n' && c != EOF)
                    c = getc(file);
                ungetc(c, file);
            } else
                string_append(str, c);
            break;
        case '&':  /* fallthrough */
        case '|':  /* fallthrough */
        case ';':  /* fallthrough */
        case '<':  /* fallthrough */
        case '>':  /* fallthrough */
        case ' ':  /* fallthrough */
        case '\t': /* fallthrough */
        case '\f': /* fallthrough */
        case '\v': /* fallthrough */
        case '\r': /* fallthrough */
        case '\n':
            if (del == DEL_DEFAULT) { /* exit default environment */
                ungetc(c, file);
                return;
            }
            string_append(str, c);
            break;
        default: /* don't do anything special with other characters */
            string_append(str, c);
        }
    }
}

static int recognize_token(struct string *str) { return WORD; }

int yylex(void)
{
    int c = getc(file);

    /* skip whitespace */
    while (is_blank(c))
        c = getc(file);

    /* determine token */
    switch (c) {
    case '(': /* fallthrough */
    case ')':
        printf("%c", c);
        return c;
    case '\n':
        printf("\n");
        return NEWLINE;
    case '&':
        c = getc(file);
        if (c == '&') {
            printf("AND_IF ");
            return AND_IF; /* && */
        }
        ungetc(c, file);
        printf("& ");
        return '&';
    case '|':
        c = getc(file);
        if (c == '|') {
            printf("OR_IF ");
            return OR_IF; /* || */
        }
        ungetc(c, file);
        printf("| ");
        return '|';
    case ';':
        c = getc(file);
        if (c == ';') {
            printf("DSEMI ");
            return DSEMI; /* ;; */
        }
        ungetc(c, file);
        printf("; ");
        return ';';
    case '<':
        c = getc(file);
        if (c == '<') {
            c = getc(file);
            if (c == '-') {
                printf("DLESSDASH ");
                return DLESSDASH; /* <<- */
            }
            ungetc(c, file);
            printf("DLESS ");
            return DLESS; /* << */
        }
        if (c == '&') {
            printf("LESSAND ");
            return LESSAND; /* <& */
        }
        if (c == '>') {
            printf("LESSGREAT ");
            return LESSGREAT; /* <> */
        }
        printf("< ");
        return '<';
    case '>':
        c = getc(file);
        if (c == '>') {
            printf("DGREAT ");
            return DGREAT; /* >> */
        }
        if (c == '&') {
            printf("GREATAND ");
            return GREATAND; /* >& */
        }
        if (c == '|') {
            printf("CLOBBER ");
            return CLOBBER; /* >| */
        }
        ungetc(c, file);
        printf("> ");
        return '>';
    case '#':
        while (c != '\n' && c != EOF)
            c = getc(file);
        if (c == '\n')
            return NEWLINE;
        if (c == EOF)
            return YYEOF;
    case EOF:
        printf("DONE ");
        return YYEOF;
    default: /* recognize a WORD, NAME, IO_NUMBER, or ASSIGNMENT_WORD */
    {
        /* create a string to store the token in */
        struct string str;
        str.capacity = DEFAULT_CAPACITY;
        str.size = 0;
        str.container = malloc(DEFAULT_CAPACITY);
        if (!str.container) {
            perror("malloc");
            return ERROR;
        }

        ungetc(c, file);
        delimit_token(&str, DEL_DEFAULT);
        string_append(&str, 0);
        printf("WORD(%s) ", str.container);
        free(str.container);
        return recognize_token(&str);
    }
    }

    return 0;
}

void yyerror(char const *s) { fprintf(stderr, "%s\n", s); }

int yyparse_wrapper(FILE *f)
{
    file = f;
    return yyparse();
}
