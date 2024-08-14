#include "lexer.h"
#include "stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_IDENTIFIER_SIZE 16

enum environment {
    default_environment,
    double_quote,
    substitution,
    backtick_substitution,
    backtick_substitution2,
    parameter_expansion,
    arithmetic_expansion,
};

static char *append_string(char *str, size_t *size, size_t *capacity, char ch)
{
    if (!str || !size || !capacity)
        return NULL;
    if (size == capacity) {
        *capacity *= 2;
        str = realloc(str, *capacity);
        if (!str) {
            perror("realloc");
            return NULL;
        }
    }
    str[(*size)++] = ch;

    return str;
}
static inline int is_blank(char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\f' ||
           ch == '\v';
}

static inline char peek_char(struct lexer *lex)
{
    return lex->read_position >= lex->input_len
               ? 0
               : lex->input[lex->read_position];
}

static void read_char(struct lexer *lex)
{
    if (lex->read_position >= lex->input_len) {
        lex->ch = 0;
    } else {
        lex->ch = lex->input[lex->read_position];
    }
    lex->position = lex->read_position;
    ++(lex->read_position);
}

static struct token_pair read_token(struct lexer *lex)
{
    struct token_pair tp = {
        .tok = TOK_ILLEGAL,
        .value = NULL,
    };

    size_t capacity = DEFAULT_IDENTIFIER_SIZE;
    size_t size = 0;
    char *identifier = malloc(capacity);
    if (!identifier) {
        perror("malloc");
        return tp;
    }

    struct stack *nested_environment = stack_create();
    enum environment curr_environment = default_environment;
    int in_backslash = 0;
    int in_single_quote = 0;
    int done = 0;

    while (is_blank(lex->ch) && lex->ch != '\n')
        read_char(lex);
    while (!done) {
        if (lex->ch == 0) {
            done = 1;
        } else if (lex->ch == '\\' && !in_backslash && !in_single_quote) {
            in_backslash = 1;
            append_string(identifier, &size, &capacity, lex->ch);
            read_char(lex);
            continue; // exit backslash mode at the end of the loop
        } else if (lex->ch == '\'' && !in_backslash &&
                   curr_environment != double_quote &&
                   curr_environment != arithmetic_expansion &&
                   !in_single_quote) {
            in_single_quote = 1;
            append_string(identifier, &size, &capacity, lex->ch);
            read_char(lex);
        } else if (lex->ch == '"' && !in_backslash && !in_single_quote &&
                   curr_environment != double_quote) {
            curr_environment = double_quote;
            stack_push(nested_environment, &curr_environment);
            append_string(identifier, &size, &capacity, lex->ch);
            read_char(lex);
        } else if (lex->ch == '$' && !in_backslash && !in_single_quote) {
            append_string(identifier, &size, &capacity, lex->ch);
            read_char(lex);
            if (lex->ch == '(') {
                append_string(identifier, &size, &capacity, lex->ch);
                read_char(lex);
                if (lex->ch == '(') {
                    curr_environment = arithmetic_expansion;
                    append_string(identifier, &size, &capacity, lex->ch);
                    read_char(lex);
                } else {
                    curr_environment = substitution;
                }
            } else if (lex->ch == '{') {
                curr_environment = parameter_expansion;
                append_string(identifier, &size, &capacity, lex->ch);
                read_char(lex);
            }
            stack_push(nested_environment, &curr_environment);
        } else if (lex->ch == '`' && !in_single_quote && !in_backslash &&
                   curr_environment != backtick_substitution) {
            append_string(identifier, &size, &capacity, lex->ch);
            read_char(lex);
            curr_environment = backtick_substitution;
            stack_push(nested_environment, &curr_environment);
        } else if (lex->ch == '`' && !in_single_quote && in_backslash &&
                   curr_environment == backtick_substitution) {
            append_string(identifier, &size, &capacity, lex->ch);
            read_char(lex);
            curr_environment = backtick_substitution2;
            stack_push(nested_environment, &curr_environment);
        } else if (lex->ch == '\'' && !in_backslash &&
                   curr_environment != double_quote &&
                   curr_environment != arithmetic_expansion &&
                   in_single_quote) {
            append_string(identifier, &size, &capacity, lex->ch);
            read_char(lex);
            in_single_quote = 0;
        } else if (lex->ch == '"' && !in_backslash && !in_single_quote &&
                   curr_environment == double_quote) {
            append_string(identifier, &size, &capacity, lex->ch);
            read_char(lex);
            enum environment *tmp = stack_pop(nested_environment);
            if (!tmp)
                curr_environment = default_environment;
            else
                curr_environment = *tmp;
        } else if (lex->ch == ')' && peek_char(lex) == ')' && !in_backslash &&
                   !in_single_quote &&
                   curr_environment == arithmetic_expansion) {
            append_string(identifier, &size, &capacity, lex->ch);
            read_char(lex);
            append_string(identifier, &size, &capacity, lex->ch);
            read_char(lex);
            enum environment *tmp = stack_pop(nested_environment);
            if (!tmp)
                curr_environment = default_environment;
            else
                curr_environment = *tmp;
        } else if ((lex->ch == ')' && !in_backslash && !in_single_quote &&
                    curr_environment == substitution) ||
                   (lex->ch == '}' && !in_backslash && !in_single_quote &&
                    curr_environment == parameter_expansion) ||
                   (lex->ch == '`' && !in_single_quote && in_backslash &&
                    curr_environment == backtick_substitution2) ||
                   (lex->ch == '`' && !in_single_quote && !in_backslash &&
                    curr_environment == backtick_substitution)) {
            append_string(identifier, &size, &capacity, lex->ch);
            read_char(lex);
            enum environment *tmp = stack_pop(nested_environment);
            if (!tmp)
                curr_environment = default_environment;
            else
                curr_environment = *tmp;
        } else if (!in_backslash && !in_single_quote &&
                   curr_environment == default_environment &&
                   (lex->ch == '&' || lex->ch == '|' || lex->ch == ';' ||
                    lex->ch == '<' || lex->ch == '>' || lex->ch == '(' ||
                    lex->ch == ')')) {
            done = 1;
        } else if (!in_backslash && !in_single_quote &&
                   curr_environment == default_environment &&
                   is_blank(lex->ch)) {
            done = 1;
            while (is_blank(lex->ch) && lex->ch != '\n')
                read_char(lex);
        } else if (!in_backslash && !in_single_quote &&
                   curr_environment == default_environment && lex->ch == '#') {
            while (lex->ch != '\n')
                read_char(lex);
        } else {
            append_string(identifier, &size, &capacity, lex->ch);
            read_char(lex);
        }

        in_backslash = 0;
    }
    append_string(identifier, &size, &capacity, 0);

    if (!identifier)
        return tp;

    tp.value = identifier;
    if (strcmp(identifier, "!") == 0)
        tp.tok = TOK_BANG;
    else if (strcmp(identifier, "{") == 0)
        tp.tok = TOK_LBRACE;
    else if (strcmp(identifier, "}") == 0)
        tp.tok = TOK_RBRACE;
    else if (strcmp(identifier, "case") == 0)
        tp.tok = TOK_CASE;
    else if (strcmp(identifier, "do") == 0)
        tp.tok = TOK_DO;
    else if (strcmp(identifier, "done") == 0)
        tp.tok = TOK_DONE;
    else if (strcmp(identifier, "elif") == 0)
        tp.tok = TOK_ELIF;
    else if (strcmp(identifier, "else") == 0)
        tp.tok = TOK_ELSE;
    else if (strcmp(identifier, "esac") == 0)
        tp.tok = TOK_ESAC;
    else if (strcmp(identifier, "fi") == 0)
        tp.tok = TOK_FI;
    else if (strcmp(identifier, "for") == 0)
        tp.tok = TOK_FOR;
    else if (strcmp(identifier, "if") == 0)
        tp.tok = TOK_IF;
    else if (strcmp(identifier, "then") == 0)
        tp.tok = TOK_THEN;
    else if (strcmp(identifier, "until") == 0)
        tp.tok = TOK_UNTIL;
    else if (strcmp(identifier, "while") == 0)
        tp.tok = TOK_WHILE;

    // TODO REDIRECTION
    // TODO HERE_DOCUMENT
    // TODO NAME in for
    // TODO Third word of for and case
    // TODO assignment preceding command name
    // TODO NAME in function
    // TODO Body of function
    else
        tp.tok = TOK_WORD;

    return tp;
}

struct lexer *lexer_create(char *input)
{
    struct lexer *lex = malloc(sizeof(struct lexer));
    if (!lex) {
        perror("malloc");
        return NULL;
    }
    lex->input = input;
    lex->input_len = strlen(input);
    lex->position = 0;
    lex->read_position = 0;
    read_char(lex);
    return lex;
}

void lexer_free(struct lexer *lex)
{
    if (lex)
        free(lex);
}

struct token_pair lexer_next(struct lexer *lex)
{
    struct token_pair tp = {
        .tok = TOK_ILLEGAL,
        .value = NULL,
    };

    switch (lex->ch) {
    case '&':
        if (peek_char(lex) == '&') {
            read_char(lex);
            tp.tok = TOK_AND_IF;
            tp.value = malloc(3);
            tp.value[0] = '&';
            tp.value[1] = '&';
            tp.value[2] = 0;
        } else {
            tp.tok = TOK_AMPERSAND;
            tp.value = malloc(2);
            tp.value[0] = '&';
            tp.value[1] = 0;
        }
        read_char(lex);
        break;
    case '|':
        if (peek_char(lex) == '|') {
            read_char(lex);
            tp.tok = TOK_OR_IF;
            tp.value = malloc(3);
            tp.value[0] = '|';
            tp.value[1] = '|';
            tp.value[2] = 0;
        } else {
            tp.tok = TOK_PIPE;
            tp.value = malloc(2);
            tp.value[0] = '|';
            tp.value[1] = 0;
        }
        read_char(lex);
        break;
    case ';':
        if (peek_char(lex) == ';') {
            read_char(lex);
            tp.tok = TOK_DSEMI;
            tp.value = malloc(3);
            tp.value[0] = ';';
            tp.value[1] = ';';
            tp.value[2] = 0;
        } else {
            tp.tok = TOK_SEMICOLON;
            tp.value = malloc(2);
            tp.value[0] = ';';
            tp.value[1] = 0;
        }
        read_char(lex);
        break;
    case '<':
        if (peek_char(lex) == '<') {
            read_char(lex);
            if (peek_char(lex) == '-') {
                read_char(lex);
                tp.tok = TOK_DLESSDASH;
                tp.value = malloc(4);
                tp.value[0] = '<';
                tp.value[1] = '<';
                tp.value[2] = '-';
                tp.value[3] = 0;
            } else {
                tp.tok = TOK_DLESS;
                tp.value = malloc(3);
                tp.value[0] = '<';
                tp.value[1] = '<';
                tp.value[2] = 0;
            }
        } else if (peek_char(lex) == '&') {
            read_char(lex);
            tp.tok = TOK_LESSAND;
            tp.value = malloc(3);
            tp.value[0] = '<';
            tp.value[1] = '&';
            tp.value[2] = 0;
        } else if (peek_char(lex) == '>') {
            read_char(lex);
            tp.tok = TOK_LESSGREAT;
            tp.value = malloc(3);
            tp.value[0] = '<';
            tp.value[1] = '>';
            tp.value[2] = 0;
        } else {
            tp.tok = TOK_LESS;
            tp.value = malloc(2);
            tp.value[0] = '<';
            tp.value[1] = 0;
        }
        read_char(lex);
        break;
    case '>':
        if (peek_char(lex) == '>') {
            read_char(lex);
            tp.tok = TOK_DGREAT;
            tp.value = malloc(3);
            tp.value[0] = '>';
            tp.value[1] = '>';
            tp.value[2] = 0;
        } else if (peek_char(lex) == '&') {
            read_char(lex);
            tp.tok = TOK_GREATAND;
            tp.value = malloc(3);
            tp.value[0] = '>';
            tp.value[1] = '&';
            tp.value[2] = 0;
        } else if (peek_char(lex) == '|') {
            read_char(lex);
            tp.tok = TOK_CLOBBER;
            tp.value = malloc(3);
            tp.value[0] = '>';
            tp.value[1] = '|';
            tp.value[2] = 0;
        } else {
            tp.tok = TOK_GREAT;
            tp.value = malloc(2);
            tp.value[0] = '>';
            tp.value[1] = 0;
        }
        read_char(lex);
        break;
    case '(':
        tp.tok = TOK_LPAREN;
        tp.value = malloc(2);
        tp.value[0] = '(';
        tp.value[1] = 0;
        read_char(lex);
        break;
    case ')':
        tp.tok = TOK_RPAREN;
        tp.value = malloc(2);
        tp.value[0] = ')';
        tp.value[1] = 0;
        read_char(lex);
        break;
    case '\n':
        tp.tok = TOK_NEWLINE;
        tp.value = malloc(2);
        tp.value[0] = '\n';
        tp.value[1] = 0;
        read_char(lex);
        break;
    case 0:
        tp.tok = TOK_EOF;
        tp.value = malloc(1);
        tp.value[0] = 0;
        break;
    default:
        tp = read_token(lex);
    }

    return tp;
}

void lexer_token_pair_free(struct token_pair *tp)
{
    if (tp->value)
        free(tp->value);
}
