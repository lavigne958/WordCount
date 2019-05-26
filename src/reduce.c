#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "utils.h"

static struct node *reduce_word(struct node *result, struct node *word)
{
    if (unlikely(!word)) {
        printf("Can not reduce empty node");
    }

    if (!result) {
        return allocate_new_node(word->key, word->key_len, word->count);
    }

    size_t longuest = LONGUEST_STR(word->key_len, result->key_len);
    int cmp = strncmp(word->key, result->key, longuest);

    if (cmp > 0){
        result->right = reduce_word(result->right, word);
    } else if (cmp < 0) {
        result->left = reduce_word(result->left, word);
    } else {
        result->count += word->count;
    }

    return result;
}

static struct node *reduce_worker(struct node *result, struct node *worker)
{
    if (unlikely(!worker)) {
        printf("Can not reduce empty result\n");
        return NULL;
    }

    result = reduce_word(result, worker);

    if (worker->left)
        result = reduce_worker(result, worker->left);

    if (worker->right)
        result = reduce_worker(result, worker->right);

    return result;
}

void reduce(struct tree *result, struct threads_arg **args, u_int32_t nr_threads)
{
    struct threads_arg *arg;

    for (int i = 0; i < nr_threads; ++i) {
        arg = args[i];
        result->root = reduce_worker(result->root, arg->tree->root);
    }
}