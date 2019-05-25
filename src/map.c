#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"

#define TOKENS " ,.?!\n"

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

    //printf("Starting worker, process '%lu'\n", args->size);

    char *token = NULL;
    char *savePtr = NULL;

    token = strtok_r(args->buff, TOKENS, &savePtr);

    while (token) {
        add_inc_word(args->root, token);
        token = strtok_r(NULL, TOKENS, &savePtr);
    }

    //printf("finish, pass root (0x%p) to args for main thread\n", &args->root);

    pthread_mutex_lock(args->lock);
    pthread_cond_signal(args->worker_ready);
    pthread_mutex_unlock(args->lock);

    pthread_mutex_lock(args->lock);
    pthread_cond_wait(args->worker_finished, args->lock);
    pthread_mutex_unlock(args->lock);

    return NULL;
}