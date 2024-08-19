#ifndef QUEUE_H_
#define QUEUE_H_

struct queue {
    void **contents;
    int start;
    int end;
    int capacity;
};

struct queue queue_create(void);
void queue_free(struct queue *);

void *queue_dequeue(struct queue *);
void queue_enqueue(struct queue *, void *);

#endif /* queue.h */
