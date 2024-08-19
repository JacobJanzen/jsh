#ifndef STACK_H_
#define STACK_H_

#include <stdlib.h>

struct stack {
    void **values;
    size_t size;
    size_t capacity;
};

struct stack stack_create(void);
void stack_free(struct stack *);

void *stack_pop(struct stack *);
void stack_push(struct stack *, void *);

#endif /* STACK_H_ */
