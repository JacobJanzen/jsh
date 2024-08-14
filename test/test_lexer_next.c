#include "../src/lexer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main()
{
    char *input = "\
five=5# hello                                        \n\
ten=10                                               \n\
#                                                    \n\
add () {                                             \n\
    echo $(($1+$2))                                  \n\
}                                                    \n\
                                                     \n\
echo $(add $five $ten)                               \n\
(echo hi)                                            \n\
                                                     \n\
if ! [ hi = hi ]; then                               \n\
    echo \"this doesn't work\"                       \n\
elif [ hi = 'hi' ] && [ hi = bye ] || [ hi = hello ] \n\
then echo nope                                       \n\
else                                                 \n\
    echo 'this does work'                            \n\
fi                                                   \n\
                                                     \n\
for $file in *; do                                   \n\
    echo $file                                       \n\
done                                                 \n\
                                                     \n\
until [ a = a ]                                      \n\
do                                                   \n\
    echo hi | grep hello                             \n\
done                                                 \n\
                                                     \n\
while [ hi = bye ]; do                               \n\
    echo hi                                          \n\
done                                                 \n\
                                                     \n\
case $five in                                        \n\
    1)                                               \n\
         echo no &                                   \n\
         ;;                                          \n\
    5)echo yes;;                                     \n\
    *);;                                             \n\
esac                                                 \n\
                                                     \n\
echo hi > test                                       \n\
echo hi >| test                                      \n\
echo hi >> test                                      \n\
cat <<EOF                                            \n\
word                                                 \n\
EOF                                                  \n\
cat<<-EOF                                            \n\
word                                                 \n\
EOF                                                  \n\
cat < test                                           \n\
1<>test                                              \n\
1>&test                                              \n\
while read line <&3; do                              \n\
    echo $line 3<&-                                  \n\
done 3< test                                         \n";
    struct token_pair tests[] = {
        {TOK_ASSIGNMENT_WORD, "five=5"},
        {TOK_NEWLINE, "\n"},
        {TOK_ASSIGNMENT_WORD, "ten=10"},
        {TOK_NEWLINE, "\n"},
        {TOK_NEWLINE, "\n"},
    };

    struct lexer *lex = lexer_create(input);
    for (int i = 0; i < 4; ++i) {
        struct token_pair tp = lexer_next(lex);

        if (tp.tok != tests[i].tok) {
            fprintf(stderr, "Expected %d, received %d\n", tests[i].tok, tp.tok);
            return 1;
        }
        if (strcmp(tp.value, tests[i].value)) {
            fprintf(stderr, "Expected %s, received %s\n", tests[i].value,
                    tp.value);
            return 1;
        }

        lexer_token_pair_free(&tp);
    }
    lexer_free(lex);
    return 0;
}
