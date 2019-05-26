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
    if (unlikely(strlen(file) <= 0)) {
        printf("file name is empty, please provide a file name\n");
        exit(0);
    }

    if (unlikely(nr_threads <= 0)) {
        printf("number of threads must be a positive integer\n");
        exit(0);
    }

    if (unlikely(stat(file, fileStats) != 0)) {
        perror("Error when getting stats");
        exit(-1);
    }

    if (unlikely((fileStats->st_mode & S_IFMT) != S_IFREG)) {
        printf("Can only work on regular files\n");
        exit(1);
    }

    return fileStats->st_size / nr_threads;
}

/**
 * allocate and initialize thread arguments
 */
static int setup_thread_arg(struct threads_arg *args, u_int32_t slice)
{
    args->tree = (struct tree *)calloc(1 ,sizeof(struct tree));
    if (unlikely(!args->tree)) {
        printf("Could not allocate worker linked list\n");
        return -1;
    }

    args->tree->root = NULL;
    args->tree->nr_nodes = 0;

    return 0;
}

/**
 * set the thread arg to memory addr where file reside
 *
 * if the worker slice next char is a word keep reading until it is
 * not a word anymore
 *
 * each worker should start and end its slice with a FULL word,
 * as equally as possible
 */
static u_int32_t read_file(char *file_ptr, struct threads_arg *arg, u_int32_t slice)
{
    if (unlikely(slice == 0)) {
        return slice;
    }

    char *next_tok = NULL;
    /* read an equal slice of the file */
    arg->buff = file_ptr;

    if (IS_LETTER(arg->buff[slice-1])) {
        /* read next token to check if the word is not finished yep */
        next_tok = arg->buff+slice;
        while (next_tok && strchr(TOKENS, *next_tok) == NULL) {
            /* keep reading until the a token is found */
            slice++;
            next_tok++;
        }
    }

    arg->size = slice;

    return slice;
}

static void rec_print_result(struct node *n)
{
    if (n->left)
        rec_print_result(n->left);

    printf("%s=%u\n", n->key, n->count);

    if (n->right)
        rec_print_result(n->right);

    free(n->key);
    free(n);
}

static inline void print_result(struct tree *result)
{
    rec_print_result(result->root);
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
    int ret = 0;

    u_int32_t slice = init_file(file, nr_threads, &fileStats);

    int fd = open(file, O_RDONLY);
    if (unlikely(fd <= 0)) {
        perror("Failed to open the file");
        ret = -1;
        goto exit;
    }

    void *orig_file_prt = mmap(NULL, fileStats.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (unlikely(orig_file_prt == MAP_FAILED)) {
        perror("Failed to map file to memeory");
        ret = -1;
        goto exit_file;
    }

    char *file_ptr = (char *)orig_file_prt;

    // setup threads arg and read file
    struct threads_arg **args = (struct threads_arg **) calloc(nr_threads, sizeof(struct threads_args *));
    if (unlikely(!args)) {
        printf("Could not allocate threads argument array\n");
        goto exit_mmap;
    }

    int i;
    u_int32_t started_threads = 0;
    struct threads_arg *arg;
    size_t length = 0;

    for (i = 0; i < nr_threads; ++i) {
        arg = (struct threads_arg *)calloc(1, sizeof(struct threads_arg));
        if (unlikely(!arg)) {
            printf("Could not allocate thread '%d' argument structure", i);
            ret = -1;
            goto exit_args;
        }

        if (unlikely(setup_thread_arg(arg, slice) < 0)) {
            printf("Failed to setup thread '%d' arguments\n", i);
            free(arg);
            goto exit_args;
        }

        // the last thread takes the left over
        if ((length + slice) > fileStats.st_size) {
            slice = fileStats.st_size - length;
        }

        arg->size = read_file(file_ptr, arg, slice);
        file_ptr += arg->size;
        length += arg->size;

        if (length <= fileStats.st_size) {
            pthread_create(&arg->tid, NULL, map, arg);
            started_threads++;
        } else {
            free(arg->tree);
            arg->tree = NULL;
            free(arg);
            arg = NULL;
            arg->tid = -1;
        }

        args[i] = arg;
    }

    //wait for each started thread to finish then agregates resulst (reduce phase)
    for (i = 0; i < started_threads; ++i) {
        arg = args[i];
        pthread_join(arg->tid, NULL);
    }

    struct tree result = {
        .nr_nodes = 0,
        .root = NULL,
    };

    reduce(&result, args, started_threads);
    print_result(&result);

exit_args:
    for (i = 0; i < started_threads; ++i) {
        free(args[i]->tree);
        free(args[i]);
    }

    free(args);

exit_mmap:
    munmap(orig_file_prt, fileStats.st_size);

exit_file:
    close(fd);

exit:
    return ret;
}