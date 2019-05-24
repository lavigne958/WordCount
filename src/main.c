#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "utils.h"

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("usage: %s file nr_threads\n", argv[0]);
        return 0;
    }

    const char *file = argv[1];
    const int32_t nr_threads = strtol(argv[2], NULL, 10);

    if (strlen(file) <= 0) {
        printf("file name is empty, please provide a file name\n");
        return 0;
    }

    if (nr_threads <= 0) {
        printf("number of threads must be a positive integer\n");
        return 0;
    }

    struct stat fileStats;
    if (stat(file, &fileStats) != 0) {
        perror("Error when getting '%s' stats");
        return -1;
    }

    if ((fileStats.st_mode & S_IFMT) != S_IFREG) {
        printf("Can only work on regular files\n");
        return 1;
    }

    int fd = open(file, O_RDONLY);
    if (fd <= 0) {
        perror("Failed to open the file");
        return -1;
    }

    off_t size = fileStats.st_size;
    off_t offset = 0;

    /* all threads compute an equal part of the file
     * the last thread computes the left over
     *
     * if only 1 thread, it compute the left over, which is the entire file
     */
    uint32_t slice;
    uint32_t left_over;
    if (nr_threads > 1) {
        slice = size / (nr_threads - 1);
        left_over = size % (nr_threads - 1);
    } else {
        slice = 0;
        left_over = size;
    }

    printf("will process '%s' using '%d' threads, each will process '%u' bytes, the last processes '%u'\n", file, nr_threads, slice, left_over);
    struct threads_arg *args = NULL;
    pthread_t *threads = (pthread_t *) calloc(nr_threads, sizeof(pthread_t));

    int i;
    for (i = 0; i < (nr_threads - 1); ++i) {
        args = (struct threads_arg *)calloc(1, sizeof(struct threads_arg));
        args->fd = fd;
        args->begin_offset = offset;
        offset += slice;
        args->end_offset = offset;
        pthread_create(&threads[i], NULL, worker, args);
    }

    args = (struct threads_arg *)calloc(1, sizeof(struct threads_arg));
    args->fd = fd;
    args->begin_offset = offset;
    offset += left_over;
    args->end_offset = offset;
    pthread_create(&threads[i], NULL, worker, args);

    for (int i = 0; i < nr_threads; ++i) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}