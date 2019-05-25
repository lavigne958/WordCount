#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>

struct map {
    const char *key;
    u_int32_t count;
    struct map *next;
};

/* structure used to pass arguments to workers */
struct threads_arg {
    char *buff;
    u_int64_t size;
    struct map *root;
    pthread_cond_t *worker_ready;
    pthread_cond_t *worker_finished;
    pthread_mutex_t *lock;
};

#define for_each_word(it, root) for (it = (root)->next; it != (root); it = it->next)

/* worker function signature */
void *worker(void *);

#endif