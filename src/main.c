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
static u_int64_t read_file(int fd, struct threads_arg *arg, u_int32_t slice)
{
    char next_tok;
    char *tmp_buff;

    /* read an equal slice of the file */
    slice = read(fd, arg->buff, slice);

    /* read next token to check if the word is not finished yep */
    while (read(fd, &next_tok, 1) > 0 && strchr(TOKENS, next_tok) == NULL) {
        /* keep reading until the a token is found */
        tmp_buff = (char *)calloc(slice + 2, sizeof(char));
        snprintf(tmp_buff, slice+2, "%s%c", arg->buff, next_tok);
        free(arg->buff);
        arg->buff = tmp_buff;
        slice++;
    }

    return slice;
}

static void reduce_word(struct map *result, struct map *word, unsigned long i)
{
    struct map *result_it = NULL;

    for_each_word(result_it, result) {
        size_t longuest = (result_it->key_len > word->key_len)? result_it->key_len : word->key_len;

        if (!strncmp(result_it->key, word->key, longuest)) {
            result_it->count += word->count;
            return;
        }
    }

    struct map *new_entry = (struct map *)calloc(1, sizeof(struct map));
    memcpy(new_entry, word, sizeof(struct map));
    new_entry->next = result->next;
    result->next = new_entry;
}

static void reduce(struct map *result, struct map *map_result, unsigned long i)
{
    struct map *it = NULL;

    for_each_word(it, map_result) {
        reduce_word(result, it, i);
    }
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

    // setup threads arg and read file
    struct threads_arg **args = (struct threads_arg **) calloc(nr_threads, sizeof(struct threads_args *));
    int i;
    u_int32_t started_threads = 0;
    struct threads_arg *arg;
    for (i = 0; i < nr_threads; ++i) {
        arg = (struct threads_arg *)calloc(1, sizeof(struct threads_arg));

        setup_thread_arg(arg, nr_threads, slice);
        arg->size = read_file(fd, arg, slice);

        if (arg->size > 0) {
            pthread_create(&arg->tid, NULL, map, arg);
            started_threads++;
        } else {
            arg->tid = -1;
        }

        args[i] = arg;
    }

    //wait for each started thread to finish then agregates resulst (reduce phase)
    struct map result;
    result.count = 0;
    result.key = NULL;
    result.next = &result;

    for (i = 0; i < started_threads; ++i) {
        arg = args[i];
        pthread_join(arg->tid, NULL);
    }

    for (i = 0; i < started_threads; ++i) {
        arg = args[i];
        reduce(&result, arg->root, arg->tid);
    }
    struct map *it;
    for_each_word(it, &result) {
        printf("%s=%u\n", it->key, it->count);
    }

    return 0;
}