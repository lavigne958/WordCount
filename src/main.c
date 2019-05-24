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

/**
 * check if the file can be map/reduced
 *
 * return the size of a slice that a worker should read
 *
 * ! this function may exit program !
 */
static u_int32_t init_file(const char *file, const u_int32_t nr_threads, struct stat *fileStats)
{
if (strlen(file) <= 0) {
        printf("file name is empty, please provide a file name\n");
        exit(0);
    }

    if (nr_threads <= 0) {
        printf("number of threads must be a positive integer\n");
        exit(0);
    }

    if (stat(file, fileStats) != 0) {
        perror("Error when getting '%s' stats");
        exit(-1);
    }

    if ((fileStats->st_mode & S_IFMT) != S_IFREG) {
        printf("Can only work on regular files\n");
        exit(1);
    }

    /* if only 1 threads it will treat the entire file */
    if (nr_threads == 1) {
        return fileStats->st_size;
    } else {
        return fileStats->st_size / nr_threads;
    }
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("usage: %s file nr_threads\n", argv[0]);
        return 0;
    }

    const char *file = argv[1];
    const int32_t nr_threads = strtol(argv[2], NULL, 10);
    struct stat fileStats;

    u_int32_t slice = init_file(file, nr_threads, &fileStats);

    int fd = open(file, O_RDONLY);
    if (fd <= 0) {
        perror("Failed to open the file");
        return -1;
    }

    printf("will process '%s' using '%d' threads, each will process '%u' bytes\n", file, nr_threads, slice);
    struct threads_arg *args = NULL;
    pthread_t *threads = (pthread_t *) calloc(nr_threads, sizeof(pthread_t));

    /* if their is some left over after spliting the file, the last thread will process it */
    int i;
    for (i = 0; i < (nr_threads - 1); ++i) {
        args = (struct threads_arg *)calloc(1, sizeof(struct threads_arg));
        args->buff = (char *) calloc(slice, sizeof(char));
        args->size = read(fd, args->buff, slice);
        pthread_create(&threads[i], NULL, worker, args);
    }

    args = (struct threads_arg *)calloc(1, sizeof(struct threads_arg));
    args->buff = (char *) calloc(slice, sizeof(char));
    args->size = read(fd, args->buff, slice + (fileStats.st_size % nr_threads));
    pthread_create(&threads[i], NULL, worker, args);

    for (int i = 0; i < nr_threads; ++i) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}