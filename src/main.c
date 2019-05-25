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
#include <sys/mman.h>

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
        perror("Error when getting stats");
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

/**
 * allocate and initialize thread arguments
 */
static void setup_thread_arg(struct threads_arg *args, int32_t nr_threads, u_int32_t slice)
{
    args->buff = (char *) calloc(slice, sizeof(char));
    args->root = (struct map *)calloc(1 ,sizeof(struct map));

    args->root->next = args->root;
    args->root->key = NULL;
    args->root->count = 0;
}

/**
 * read the file up to the next token after the calculated slice
 *
 * if the slice does not ends on a token,
 * read up to the next token
 * this way workers buffer do not overlap on a word
 */
static u_int64_t read_file(char *file_ptr, struct threads_arg *arg, u_int32_t slice)
{
    char *next_tok = NULL;
    /* read an equal slice of the file */
    arg->buff = file_ptr;

    /* read next token to check if the word is not finished yep */
    next_tok = arg->buff+slice+1;
    while (next_tok && strchr(TOKENS, *next_tok) == NULL) {
        /* keep reading until the a token is found */
        slice++;
        next_tok++;
    }

    arg->size = slice;

    return slice;
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("usage: %s file nr_threads\n", argv[0]);
        return 0;
    }

    // init program args
    const char *file = argv[1];
    const int32_t nr_threads = strtol(argv[2], NULL, 10);
    struct stat fileStats;

    u_int32_t slice = init_file(file, nr_threads, &fileStats);

    int fd = open(file, O_RDONLY);
    if (fd <= 0) {
        perror("Failed to open the file");
        return -1;
    }

    char *file_ptr = mmap(NULL, fileStats.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_ptr == MAP_FAILED) {
        perror("Failed to map file to memeory");
        return -1;
    }

    // setup threads arg and read file
    struct threads_arg **args = (struct threads_arg **) calloc(nr_threads, sizeof(struct threads_args *));
    int i;
    u_int32_t started_threads = 0;
    struct threads_arg *arg;
    for (i = 0; i < nr_threads; ++i) {
        arg = (struct threads_arg *)calloc(1, sizeof(struct threads_arg));

        setup_thread_arg(arg, nr_threads, slice);
        arg->size = read_file(file_ptr, arg, slice);
        file_ptr += arg->size;

        if (arg->size > 0) {
            pthread_create(&arg->tid, NULL, map, arg);
            started_threads++;
        } else {
            arg->tid = -1;
        }

        args[i] = arg;
    }

    //wait for each started thread to finish then agregates resulst (reduce phase)
    for (i = 0; i < started_threads; ++i) {
        arg = args[i];
        pthread_join(arg->tid, NULL);
    }

    struct map result = {
        .count = 0,
        .key = NULL,
        .key_len = 0,
        .next = &result,
        .prev = &result
    };

    reduce(&result, args, started_threads);

    struct map *it;
    for_each_word(it, &result) {
        printf("%s=%u\n", it->key, it->count);
    }

    return 0;
}