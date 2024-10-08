#include "lexer.h"
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

static void delimit_token(struct lexer *lex, struct string *str,
                          enum delimiter del)
{
    while (1) {
        int c = getc(lex->stream);

        switch (c) {
        case EOF: /* exit on EOF */
            return;
        case '\\': /* handle escape character */
            c = getc(lex->stream);
            switch (c) {
            case EOF:
                return;
            case '\n':
                break;
            default:
                string_append(str, c);
            }
            break;
        case '\'': /* enter/exit single quote */
            string_append(str, c);
            if (del == DEL_QUOTE)
                return;
            if (del != DEL_DQUOTE && del != DEL_BRACE)
                delimit_token(lex, str, DEL_QUOTE);
            break;
        case '\"': /* enter/exit double quote */
            string_append(str, c);
            if (del == DEL_DQUOTE)
                return;
            if (del != DEL_QUOTE)
                delimit_token(lex, str, DEL_DQUOTE);
            break;
        case '`': /* enter/exit backquote */
            string_append(str, c);
            if (del == DEL_BQUOTE)
                return;
            if (del != DEL_QUOTE)
                delimit_token(lex, str, DEL_BQUOTE);
            break;
        case '$':
            string_append(str, c);
            if (del != DEL_QUOTE) {
                c = getc(lex->stream);
                switch (c) {
                case '{': /* enter paramter expansion */
                    string_append(str, c);
                    delimit_token(lex, str, DEL_BRACE);
                    break;
                case '(':
                    string_append(str, c);
                    c = getc(lex->stream);
                    if (c == '(') { /* enter arithmetic expansion */
                        string_append(str, c);
                        delimit_token(lex, str, DEL_DPAR);
                    } else { /* enter command substitution */
                        ungetc(c, lex->stream);
                        delimit_token(lex, str, DEL_PAR);
                    }
                    break;
                default:
                    ungetc(c, lex->stream);
                }
            }
            break;
        case '(':
            if (del == DEL_DEFAULT) /* special character; exit by default */
                return;
            string_append(str, c);
            break;
        case ')':
            if (del == DEL_DEFAULT) /* special character; exit by default */
                return;
            string_append(str, c);
            if (del == DEL_DPAR) { /* exit arithmetic expansion */
                delimit_token(lex, str, DEL_PAR);
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
                    c = getc(lex->stream);
                ungetc(c, lex->stream);
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
                ungetc(c, lex->stream);
                return;
            }
            string_append(str, c);
            break;
        default: /* don't do anything special with other characters */
            string_append(str, c);
        }
    }
}

static int recognize_token(struct lexer *lex, struct string *str)
{
    if (str->size == 0)
        return ERROR;
    if (str->container[0] == '=')
        return WORD;

    int eligible_name = 1;
    int eligible_io_number = 1;
    int eligible_assignment_word = 0;
    if ((str->container[0] < 'a' || str->container[0] > 'z') &&
        (str->container[0] < 'A' || str->container[0] > 'Z') &&
        str->container[0] != '_')
        eligible_name = 0;
    for (size_t i = 0; i < str->size - 1; ++i) {
        if (i > 0 && str->container[i] == '=')
            eligible_assignment_word = 1;
        if ((str->container[i] < 'a' || str->container[i] > 'z') &&
            (str->container[i] < 'A' || str->container[i] > 'Z') &&
            str->container[i] != '_') {
            eligible_name = 0;
        }
        if (str->container[i] < '0' || str->container[i] > '9') {
            eligible_io_number = 0;
        }
    }

    if (lex->prev == 0 || lex->prev == ';' || lex->prev == '&' ||
        lex->prev == '|' || lex->prev == NEWLINE || lex->prev == BANG ||
        lex->prev == AND_IF || lex->prev == OR_IF) {
        if (strcmp(str->container, "if") == 0)
            return IF;
        if (strcmp(str->container, "then") == 0)
            return THEN;
        if (strcmp(str->container, "else") == 0)
            return ELSE;
        if (strcmp(str->container, "elif") == 0)
            return ELIF;
        if (strcmp(str->container, "fi") == 0)
            return FI;
        if (strcmp(str->container, "do") == 0)
            return DO;
        if (strcmp(str->container, "done") == 0)
            return DONE;
        if (strcmp(str->container, "case") == 0)
            return CASE;
        if (strcmp(str->container, "esac") == 0)
            return ESAC;
        if (strcmp(str->container, "while") == 0)
            return WHILE;
        if (strcmp(str->container, "until") == 0)
            return UNTIL;
        if (strcmp(str->container, "for") == 0)
            return FOR;
        if (strcmp(str->container, "{") == 0)
            return LBRACE;
        if (strcmp(str->container, "}") == 0)
            return RBRACE;
        if (strcmp(str->container, "!") == 0)
            return BANG;
        if (strcmp(str->container, "in") == 0)
            return IN;

        if (eligible_name) {
            /* check for beginning of function */
            int c = getc(lex->stream);
            while (is_blank(c))
                c = getc(lex->stream);
            ungetc(c, lex->stream);
            if (c == '(')
                return NAME;
        }
    }
    if (lex->prev == LBRACE || lex->prev == RBRACE || lex->prev == DO ||
        lex->prev == DONE || lex->prev == ELIF || lex->prev == ELSE ||
        lex->prev == ESAC || lex->prev == FI || lex->prev == IF ||
        lex->prev == THEN || lex->prev == UNTIL || lex->prev == WHILE) {
        if (strcmp(str->container, "if") == 0)
            return IF;
        if (strcmp(str->container, "then") == 0)
            return THEN;
        if (strcmp(str->container, "else") == 0)
            return ELSE;
        if (strcmp(str->container, "elif") == 0)
            return ELIF;
        if (strcmp(str->container, "fi") == 0)
            return FI;
        if (strcmp(str->container, "do") == 0)
            return DO;
        if (strcmp(str->container, "done") == 0)
            return DONE;
        if (strcmp(str->container, "case") == 0)
            return CASE;
        if (strcmp(str->container, "esac") == 0)
            return ESAC;
        if (strcmp(str->container, "while") == 0)
            return WHILE;
        if (strcmp(str->container, "until") == 0)
            return UNTIL;
        if (strcmp(str->container, "for") == 0)
            return FOR;
        if (strcmp(str->container, "{") == 0)
            return LBRACE;
        if (strcmp(str->container, "}") == 0)
            return RBRACE;
        if (strcmp(str->container, "!") == 0)
            return BANG;
        if (strcmp(str->container, "in") == 0)
            return IN;
    }
    if (lex->prevprev == CASE && strcmp(str->container, "in") == 0)
        return IN;
    if (lex->prevprev == FOR) {
        if (strcmp(str->container, "in") == 0)
            return IN;
        if (strcmp(str->container, "do") == 0)
            return DO;
    }
    if (eligible_name && lex->prev == FOR) {
        if (lex->prev == FOR)
            return NAME;
    }

    /* TODO handle correctly */
    if (eligible_assignment_word)
        return ASSIGNMENT_WORD;
    if (eligible_io_number)
        return IO_NUMBER;

    return WORD;
}

struct token lexer_next(struct lexer *lex)
{
    int c = getc(lex->stream);

    /* skip whitespace */
    while (is_blank(c))
        c = getc(lex->stream);

    /* determine token */
    struct token result = {.token = ERROR, .value = NULL};
    switch (c) {
    case '(': /* fallthrough */
    case ')':
        result.token = c;
        break;
    case '\n':
        result.token = NEWLINE;
        break;
    case '&':
        c = getc(lex->stream);
        if (c == '&') {
            result.token = AND_IF;
        } else {
            ungetc(c, lex->stream);
            result.token = '&';
        }
        break;
    case '|':
        c = getc(lex->stream);
        if (c == '|') {
            result.token = OR_IF;
        } else {
            ungetc(c, lex->stream);
            result.token = '|';
        }
        break;
    case ';':
        c = getc(lex->stream);
        if (c == ';') {
            result.token = DSEMI;
        } else {
            ungetc(c, lex->stream);
            result.token = ';';
        }
        break;
    case '<':
        c = getc(lex->stream);
        if (c == '<') {
            c = getc(lex->stream);
            if (c == '-') {
                result.token = DLESSDASH;
            } else {
                ungetc(c, lex->stream);
                result.token = DLESS;
            }
        } else if (c == '&') {
            result.token = LESSAND;
        } else if (c == '>') {
            result.token = LESSGREAT;
        } else {
            result.token = '<';
        }
        break;
    case '>':
        c = getc(lex->stream);
        if (c == '>') {
            result.token = DGREAT;
        } else if (c == '&') {
            result.token = GREATAND;
        } else if (c == '|') {
            result.token = CLOBBER;
        } else {
            ungetc(c, lex->stream);
            result.token = '>';
        }
        break;
    case '#':
        while (c != '\n' && c != EOF)
            c = getc(lex->stream);
        if (c == '\n') {
            result.token = NEWLINE;
        } else {
            result.token = EOF;
        }
        break;
    case EOF:
        result.token = EOF;
        break;
    default: /* recognize a WORD, NAME, IO_NUMBER, or ASSIGNMENT_WORD */
    {
        /* create a string to store the token in */
        struct string str;
        str.capacity = DEFAULT_CAPACITY;
        str.size = 0;
        str.container = malloc(DEFAULT_CAPACITY);
        if (!str.container) {
            perror("malloc");
            result.token = ERROR;
        }

        ungetc(c, lex->stream);
        delimit_token(lex, &str, DEL_DEFAULT);
        string_append(&str, 0);
        result.value = str.container;
        result.token = recognize_token(lex, &str);
    }
    }

    lex->prevprev = lex->prev;
    lex->prev = result.token;
    return result;
}
