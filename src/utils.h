#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <pthread.h>

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
    pthread_t tid;
};

#define for_each_word(it, root) for (it = (root)->next; it != (root); it = it->next)
#define TOKENS " ,.?!\n"

/* worker function signature */
void *worker(void *);

#endif