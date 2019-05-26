#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <pthread.h>

/**
 * map that holds a word and it occurences
 */
struct map {
    char *key;
    size_t key_len;
    u_int32_t count;
    struct map *next;
    struct map *prev; //easier for sorting
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
#define for_each_word_safe(it, tmp, root) for (it = (root)->next, tmp = it->next; it != (root); it = tmp, tmp = tmp->next)

#define LONGUEST_STR(str1, str2) ((str1 > str2)? str1 : str2)
#define IS_LETTER(pos) ((65 <= (pos) && (pos) <= 90) || (97 <= (pos) && (pos) <= 122) || (pos) == 45)

static inline void insert_word(struct map *root, struct map *word)
{
    word->next = root->next;
    word->next->prev = word;
    root->next = word;
    word->prev = root;
}

static inline void swap_words(struct map *w1, struct map *w2)
{
    struct map *tmp = w1->next;
    w1->next = w2->next;
    w2->next->prev = w1;
    w2->next = tmp;
    tmp->prev = w2;
    tmp = w1->prev;
    w1->prev = w2->prev;
    w1->prev->next = w1;
    w2->prev = tmp;
    tmp->next = w2;
}

/**
 * tokens that mark the limit of a word
 */
#define TOKENS " ,.:;!?\n\t"

/* worker function signature */
void *map(void *);

/* reduce function signature */
void reduce(struct map * result, struct threads_arg **args, u_int32_t nr_threads);

#endif