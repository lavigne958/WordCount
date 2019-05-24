#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

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
        return 1;
    }

    if ((fileStats.st_mode & S_IFMT) != S_IFREG) {
        printf("Can only work on regular files\n");
        return 1;
    }

    off_t size = fileStats.st_size;
    uint32_t slice = size / nr_threads;
    uint32_t left_over = size % nr_threads;

    printf("will process '%s' using '%d' threads, each will process '%u' bytes\n", file, nr_threads, slice);

    return 0;
}