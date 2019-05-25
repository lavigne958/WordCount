#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"

static void add_inc_word(struct map *root, const char *found)
{
    struct map *it = NULL;
    size_t found_len = strlen(found);
    size_t key_len;

    for_each_word(it, root) {
        key_len = strlen(it->key);
        if (!strncmp(it->key, found, ((found_len>key_len)?found_len:key_len))) {
            it->count++;
            return;
        }
    }

    struct map *new_word = (struct map *)malloc(sizeof(struct map));
    new_word->key = strdup(found);
    new_word->count = 1;
    new_word->next = root->next;
    root->next = new_word;
}

void *worker(void *arg)
{
    struct threads_arg *args = (struct threads_arg *)arg;

    printf("[%lu] Starting worker, process '%lu'\n", args->tid, args->size);

    char *token = NULL;
    char *savePtr = NULL;

    token = strtok_r(args->buff, TOKENS, &savePtr);

    while (token) {
        add_inc_word(args->root, token);
        token = strtok_r(NULL, TOKENS, &savePtr);
    }

    printf("[%lu] goodby world\n", args->tid);

    return NULL;
}