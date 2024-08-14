#include "../src/stack.h"
#include <stdio.h>

int main(void)
{
    int tests[100];
    struct stack *s = stack_create();
    for (int i = 0; i < 100; ++i) {
        tests[i] = i;
        stack_push(s, &tests[i]);
    }

    for (int i = 99; i >= 0; --i) {
        int *val = stack_peek(s);
        if (!val) {
            fprintf(stderr,
                    "expected non-null value to be popped for index %d\n", i);
            return 1;
        }
        if (*val != tests[i]) {
            fprintf(stderr, "expected %d, got %d\n", tests[i], *val);
            return 1;
        }
        val = stack_pop(s);
        if (!val) {
            fprintf(stderr,
                    "expected non-null value to be popped for index %d\n", i);
            return 1;
        }
        if (*val != tests[i]) {
            fprintf(stderr, "expected %d, got %d\n", tests[i], *val);
            return 1;
        }
    }

    stack_free(s);
}
