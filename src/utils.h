#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>

/* structure used to pass arguments to workers */
struct threads_arg {
    int fd;
    off_t begin_offset;
    off_t end_offset;
};

/* worker function signature */
void *worker(void *);

#endif