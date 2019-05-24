#include <pthread.h>
#include <stdio.h>

#include "utils.h"

void *worker(void *arg)
{
    struct threads_arg *args = (struct threads_arg *)arg;

    printf("Starting worker, process '%ld'=>'%ld'\n", args->begin_offset, args->end_offset);

    return NULL;
}