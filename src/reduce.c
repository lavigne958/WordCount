#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "utils.h"

static int reduce_word(struct map *result, struct map *word)
{
    struct map *result_it = NULL;

    for_each_word(result_it, result) {
        size_t longuest = LONGUEST_STR(result_it->key_len, word->key_len);

        if (!strncmp(result_it->key, word->key, longuest)) {
            result_it->count += word->count;
            return 0;
        }
    }

    struct map *new_entry = (struct map *)calloc(1, sizeof(struct map));
    memcpy(new_entry, word, sizeof(struct map));
    insert_word(result, new_entry);

    return 1;
}

static void sort_map(struct map *result)
{
    int swapped;
    struct map *pos = NULL;
    struct map *last_pos = NULL;

    if (result->next == result)
        return;

    size_t longuest;
    do {
        swapped = 0;
        pos = result->next;

        while (pos->next != last_pos && pos->next != result) {
            longuest = LONGUEST_STR(pos->key_len, pos->next->key_len);
            if (strncmp(pos->key, pos->next->key, longuest) > 0) {
                // due to swapp, no need to update pos with next pointer
                swap_words(pos, pos->next);
                swapped = 1;
            } else {
                pos = pos->next;
            }
        }
        last_pos = pos;
    } while (swapped);
}

void reduce(struct map *result, struct threads_arg **args, u_int32_t nr_threads)
{
    struct threads_arg *arg;
    u_int64_t nr_words = 0;

    for (int i = 0; i < nr_threads; ++i) {
        arg = args[i];
        struct map *it = NULL;

        for_each_word(it, arg->root) {
            nr_words += reduce_word(result, it);
        }
    }

    printf("there are '%lu' words to sort\n", nr_words);
    sort_map(result);
}