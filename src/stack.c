#include "stack.h"
#include <stdio.h>

#define DEFAULT_CAPACITY 16

struct stack stack_create(void)
{
    struct stack s = {
        .capacity = DEFAULT_CAPACITY,
        .size = 0,
        .values = malloc(sizeof(void *) * DEFAULT_CAPACITY),
    };

    return s;
}

void stack_free(struct stack *s)
{
    if (s && s->values)
        free(s->values);
}

void *stack_pop(struct stack *s)
{
    if (!s || s->size == 0 || !s->values)
        return NULL;

    void *val = s->values[s->size - 1];
    --(s->size);

    return val;
}

void stack_push(struct stack *s, void *val)
{
    if (!s || !s->values)
        return;

    if (s->size == s->capacity) {
        s->capacity *= 2;
        s->values = realloc(s->values, sizeof(void **) * s->capacity * 2);
        if (!s->values)
            return;
    }

    s->values[s->size] = val;
    ++(s->size);
}
