#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <pthread.h>

/**
 * map that holds a word and it occurences
 */
struct map {
    const char *key;
    const size_t key_len;
    u_int32_t count;
    struct map *next;
};

/**
 * structure used to pass arguments to workers
 *
 * buff: buffer containing the bytes to read
 * size: the size of the buffer
 * root: root node of the map for a single worker
 * tid: the thread id
 */
struct threads_arg {
    char *buff;
    u_int64_t size;
    struct map *root;
    pthread_t tid;
};

/**
 * macro to iterate over the map
 */
#define for_each_word(it, root) for (it = (root)->next; it != (root); it = it->next)

/**
 * tokens that mark the limit of a word
 */
#define TOKENS " ,.?!\n"

/* worker function signature */
void *map(void *);

#endif