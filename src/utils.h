#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>

/* structure used to pass arguments to workers */
struct threads_arg {
    char *buff;
    u_int64_t size;
};

/* worker function signature */
void *worker(void *);

#endif