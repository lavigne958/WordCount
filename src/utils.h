#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <pthread.h>

/**
 * node that holds a word and it occurences
 */
struct node {
    char *key;
    size_t key_len;
    u_int32_t count;
    struct node *left;
    struct node *right;
};

struct tree {
    u_int32_t nr_nodes;
    struct node *root;
};

/**
 * structure used to pass arguments to workers
 *
 * buff: buffer containing the bytes to read
 * size: the size of the buffer
 * tree: pointer to the tree that holds the words
 * tid: the thread id
 */
struct threads_arg {
    char *buff;
    u_int64_t size;
    struct tree *tree;
    pthread_t tid;
};

/**
 * macro to iterate over the map
#define for_each_word(it, root) for (it = (root)->next; it != (root); it = it->next)
#define for_each_word_safe(it, tmp, root) for (it = (root)->next, tmp = it->next; it != (root); it = tmp, tmp = tmp->next)
 */

#define LONGUEST_STR(str1, str2) ((str1 > str2)? str1 : str2)
#define IS_LETTER(pos) ((65 <= (pos) && (pos) <= 90) || (97 <= (pos) && (pos) <= 122) || (pos) == 45)

#ifdef __GNUC__
#define unlikely(x) __builtin_expect((x), 0)
#else
#define unlikely(x) (x)
#endif

/*
static inline void insert_word(struct node *root, struct node *word)
{
    word->next = root->next;
    word->next->prev = word;
    root->next = word;
    word->prev = root;
}

static inline void swap_words(struct node *w1, struct node *w2)
{
    struct node *tmp = w1->next;
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
*/

/**
 * tokens that mark the limit of a word
 */
#define TOKENS " ,.:;!?\n\t"

/* worker function signature */
void *map(void *);

/* reduce function signature */
void reduce(struct tree *result, struct threads_arg **args, u_int32_t nr_threads);

struct node *allocate_new_node(const char *word, const size_t word_len, u_int32_t count);

/* sort linked list of words */
//void sort_map(struct node *result);

#endif