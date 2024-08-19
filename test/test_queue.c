#include "../src/queue.h"
#include <stdio.h>

int main(void)
{
    int tests[100];
    struct queue q = queue_create();

    /* insert 0-99 */
    for (int i = 0; i < 100; ++i) {
        tests[i] = i;
        queue_enqueue(&q, &tests[i]);
    }

    /* remove 0-49 */
    for (int i = 0; i < 50; ++i) {
        int *val = queue_dequeue(&q);
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

    /* insert 0-49 */
    for (int i = 0; i < 50; ++i) {
        tests[i] = i;
        queue_enqueue(&q, &tests[i]);
    }

    /* remove 50-100 */
    for (int i = 50; i < 100; ++i) {
        int *val = queue_dequeue(&q);
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

    /* remove 0-49 */
    for (int i = 0; i < 50; ++i) {
        int *val = queue_dequeue(&q);
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

    queue_free(&q);
}
