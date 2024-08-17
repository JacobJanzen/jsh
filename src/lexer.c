#include "../build/grammar.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_CAPACITY 16

enum token_delimeter {
    DEL_DEFAULT,
    DEL_DQUOTE,
    DEL_QUOTE,
    DEL_RPAR,
    DEL_DRPAR,
    DEL_RBRACE,
    DEL_BACKQUOTE,
};

struct string {
    char *container;
    size_t size;
    size_t capacity;
};

int saved_char = -1;
int can_be_io_number;
int can_be_name;
int can_be_assignment;
int can_be_grammatical_assignment = 1;
int can_be_grammatical_name = 0;
int first_char;

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
    return c == ' ' || c == '\t' || c == '\f' || c == '\v';
}

static void recognize_token(struct string *str, enum token_delimeter del)
{
    int done = 0;
    while (!done) {
        char c;
        if (saved_char == -1)
            c = getchar();
        else
            c = saved_char;
        saved_char = -1;

        if (c == 0 || c == EOF) {
            done = 1;
        } else if (del == DEL_QUOTE) {
            string_append(str, c);
            if (c == '\'')
                done = 1;
        } else {
            if (c == '\\') {
                c = getchar();

                if (c == 0 || c == EOF) {
                    string_append(str, '\\');
                    done = 1;
                } else if (c != '\n') {
                    /* don't save escaped newlines */
                    string_append(str, '\\');
                    string_append(str, c);
                }
                can_be_name = 0;
                can_be_io_number = 0;
            } else if (c == '$') {
                string_append(str, c);
                c = getchar();

                if (c == 0 || c == EOF) {
                    done = 1;
                } else if (c == '{') {
                    /* identify parameter expansion */
                    string_append(str, c);
                    recognize_token(str, DEL_RBRACE);
                } else if (c == '(') {
                    string_append(str, c);
                    c = getchar();

                    if (c == 0 || c == EOF) {
                        done = 1;
                    } else if (c == '(') {
                        /* identify arithmetic expansion */
                        string_append(str, c);
                        recognize_token(str, DEL_DRPAR);

                    } else {
                        /* identify command substitution */
                        saved_char = c;
                        recognize_token(str, DEL_RPAR);
                    }
                } else {
                    saved_char = c;
                }
                can_be_name = 0;
                can_be_io_number = 0;
            } else if (c == '`') {
                /* identify command substitution */
                string_append(str, c);
                if (del == DEL_BACKQUOTE) {
                    done = 1;
                } else {
                    recognize_token(str, DEL_BACKQUOTE);
                }
                can_be_name = 0;
                can_be_io_number = 0;
            } else if (c == '}' && del == DEL_RBRACE) {
                /* close parameter expansion */
                string_append(str, c);
                done = 1;
            } else if (c == ')' && (del == DEL_RPAR || del == DEL_DRPAR)) {
                /* close command substitution or drop into command substitution
                 * mode if in arithmetic expansion */
                string_append(str, c);
                if (del == DEL_DRPAR) {
                    recognize_token(str, DEL_RPAR);
                }
                done = 1;
            } else if (c == '\'' && del != DEL_DQUOTE && del != DEL_DRPAR) {
                /* identify single quote */
                string_append(str, c);
                recognize_token(str, DEL_QUOTE);
                can_be_name = 0;
                can_be_io_number = 0;
            } else if (c == '\"' && del != DEL_RBRACE) {
                /* identify double quote */
                string_append(str, c);
                if (del == DEL_DQUOTE) {
                    done = 1;
                } else {
                    recognize_token(str, DEL_DQUOTE);
                }
                can_be_name = 0;
                can_be_io_number = 0;
            } else if (del == DEL_DEFAULT &&
                       (c == '&' || c == '|' || c == ';' || c == '<' ||
                        c == '>' || c == '(' || c == ')' || c == '\n' ||
                        is_blank(c))) {
                /* identify end of word by operator or blank */
                string_append(str, 0);
                saved_char = c;
                done = 1;
            } else if (c == '#' && del != DEL_DQUOTE && del != DEL_DRPAR) {
                /* identify comment */
                c = getchar();
                while (c != '\n' && c != 0)
                    c = getchar();
                saved_char = c;
            } else {
                /* add anything else to the word */
                string_append(str, c);

                if ((c <= 'a' || c >= 'z') && (c <= 'A' || c >= 'Z') &&
                    (c <= '0' || c >= '9') && c != '_') {
                    can_be_name = 0;
                    if (c == '=' && !first_char) {
                        can_be_assignment = 1;
                    }
                }
                if (c <= '0' || c >= '9')
                    can_be_io_number = 0;
            }
        }
        first_char = 0;
    }
}

int yylex(void)
{
    int c;
    if (saved_char == -1)
        c = getchar();
    else
        c = saved_char;
    saved_char = -1;

    while (is_blank(c) && c != 0)
        c = getchar();

    switch (c) {
    case 0:
    case EOF:
        printf("EOF ");
        return YYEOF;
    case '\n':
        printf("\n");
        can_be_grammatical_assignment = 1;
        return NEWLINE;
    case '&':
        c = getchar();
        if (c == '&') {
            printf("AND_IF ");
            return AND_IF;
        }
        saved_char = c;
        printf("& ");
        return '&';
    case '|':
        c = getchar();
        if (c == '|') {
            printf("OR_IF ");
            return OR_IF;
        }
        saved_char = c;
        printf("| ");
        return '|';
    case ';':
        c = getchar();
        if (c == ';') {
            printf("DSEMI ");
            return DSEMI;
        }
        saved_char = c;
        printf("; ");
        return ';';
    case '<':
        c = getchar();
        if (c == '<') {
            c = getchar();
            if (c == '-') {
                printf("DLESSDASH ");
                return DLESSDASH;
            }
            saved_char = c;
            printf("DLESS ");
            return DLESS;
        }
        if (c == '&') {
            printf("LESSAND ");
            return LESSAND;
        }
        if (c == '>') {
            printf("LESSGREAT ");
            return LESSGREAT;
        }
        saved_char = c;
        printf("< ");
        return '<';
    case '>':
        c = getchar();
        if (c == '>') {
            printf("DGREAT ");
            return DGREAT;
        }
        if (c == '&') {
            printf("GREATAND ");
            return GREATAND;
        }
        if (c == '|') {
            printf("CLOBBER ");
            return CLOBBER;
        }
        saved_char = c;
        printf("> ");
        return '>';
    case '(':
    case ')':
        printf("%c ", c);
        return c;
    default: {
        saved_char = c;
        if ((c <= 'a' || c >= 'z') && (c <= 'A' || c >= 'Z') && c != '_')
            can_be_name = 0;
        else
            can_be_name = 1;
        can_be_assignment = 0;
        if (c >= '0' && c <= '9')
            can_be_io_number = 1;
        else
            can_be_io_number = 0;

        struct string *str = malloc(sizeof(struct string));
        str->capacity = DEFAULT_CAPACITY;
        str->size = 0;
        str->container = malloc(DEFAULT_CAPACITY);
        first_char = 1;
        recognize_token(str, DEL_DEFAULT);

        yylval = str->container;
        if (strcmp(str->container, "!") == 0) {
            can_be_grammatical_assignment = 0;
            printf("Bang ");
            return Bang;
        }
        if (strcmp(str->container, "{") == 0) {
            can_be_grammatical_assignment = 0;
            printf("Lbrace ");
            return Lbrace;
        }
        if (strcmp(str->container, "}") == 0) {
            can_be_grammatical_assignment = 0;
            printf("Rbrace ");
            return Rbrace;
        }
        if (strcmp(str->container, "case") == 0) {
            can_be_grammatical_assignment = 0;
            printf("Case ");
            return Case;
        }
        if (strcmp(str->container, "do") == 0) {
            can_be_grammatical_assignment = 0;
            printf("Do ");
            return Do;
        }
        if (strcmp(str->container, "done") == 0) {
            can_be_grammatical_assignment = 0;
            printf("Done ");
            return Done;
        }
        if (strcmp(str->container, "elif") == 0) {
            can_be_grammatical_assignment = 0;
            printf("Elif ");
            return Elif;
        }
        if (strcmp(str->container, "else") == 0) {
            can_be_grammatical_assignment = 0;
            printf("Else ");
            return Else;
        }
        if (strcmp(str->container, "esac") == 0) {
            can_be_grammatical_assignment = 0;
            printf("Esac ");
            return Esac;
        }
        if (strcmp(str->container, "fi") == 0) {
            can_be_grammatical_assignment = 0;
            printf("Fi ");
            return Fi;
        }
        if (strcmp(str->container, "for") == 0) {
            can_be_grammatical_assignment = 0;
            can_be_grammatical_name = 1;
            printf("For ");
            return For;
        }
        if (strcmp(str->container, "if") == 0) {
            can_be_grammatical_assignment = 0;
            printf("If ");
            return If;
        }
        if (strcmp(str->container, "in") == 0) {
            can_be_grammatical_assignment = 0;
            printf("In ");
            return In;
        }
        if (strcmp(str->container, "then") == 0) {
            can_be_grammatical_assignment = 0;
            printf("Then ");
            return Then;
        }
        if (strcmp(str->container, "until") == 0) {
            can_be_grammatical_assignment = 0;
            printf("Until ");
            return Until;
        }
        if (strcmp(str->container, "while") == 0) {
            can_be_grammatical_assignment = 0;
            printf("While ");
            return While;
        }
        if (can_be_name) {
            if (saved_char == -1)
                c = getchar();
            else
                c = saved_char;
            while (is_blank(c) && c != 0)
                c = getchar();
            saved_char = c;
            if (can_be_grammatical_name || c == '(') {
                printf("NAME(%s) ", str->container);
                return NAME;
            }
        }
        if (can_be_assignment && can_be_grammatical_assignment) {
            printf("ASSIGNMENT_WORD(%s) ", str->container);
            return ASSIGNMENT_WORD;
        }
        if (can_be_io_number) {
            can_be_grammatical_assignment = 0;
            if (saved_char == -1)
                c = getchar();
            else
                c = saved_char;
            while (is_blank(c) && c != 0)
                c = getchar();
            saved_char = c;
            if (c == '<' || c == '>') {
                printf("IO_NUMBER(%s) ", str->container);
                return IO_NUMBER;
            }
        }

        can_be_grammatical_assignment = 0;
        printf("WORD(%s) ", str->container);
        return WORD;
    }
    }
}

void yyerror(char const *s) { fprintf(stderr, "%s\n", s); }

int main(void) { return yyparse(); }
