#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_CAPACITY 16

struct queue queue_create(void)
{
    struct queue q = {.capacity = DEFAULT_CAPACITY,
                      .start = 0,
                      .end = 0,
                      .contents = malloc(sizeof(void *) * DEFAULT_CAPACITY)};

    return q;
}

void queue_free(struct queue *q)
{
    if (q && q->contents)
        free(q->contents);
}

void *queue_dequeue(struct queue *q)
{
    if (!q || q->start == q->end)
        return NULL;
    void *res = q->contents[q->start];
    q->start = (q->start + 1) % q->capacity;

    return res;
}

void queue_enqueue(struct queue *q, void *val)
{
    if (!q)
        return;

    q->contents[q->end] = val;

    int new_end = (q->end + 1) % q->capacity;
    if (new_end == q->start) {
        q->contents = realloc(q->contents, sizeof(void *) * q->capacity * 2);
        new_end = (q->end + 1);
        if (q->end < q->start) {
            for (int i = 0; i <= q->end; ++i) {
                q->contents[i + q->capacity] = q->contents[i];
            }
            new_end = q->capacity + q->end + 1;
        }
        q->capacity *= 2;
    }
    q->end = new_end;
}
