#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

#define TOKENS " ,.?!\n"

void *worker(void *arg)
{
    struct threads_arg *args = (struct threads_arg *)arg;

    printf("Starting worker, process '%lu'\n", args->size);

    char *token = NULL;
    char *savePtr = NULL;

    token = strtok_r(args->buff, TOKENS, &savePtr);

    while (token) {
        printf("Found '%s'\n", token);
        token = strtok_r(NULL, TOKENS, &savePtr);
    }

    return NULL;
}