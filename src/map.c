#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"

#define IS_LETTER(pos) ((65 <= *(pos) && *(pos) <= 90) || (97 <= *(pos) && *(pos) <= 122))

static void add_inc_word(struct map *root, const char *found, const size_t found_len)
{
    struct map *it = NULL;

    for_each_word(it, root) {
        /* compare up to the longest word,
         * otherwise a word that is a sub part of the other one migh return false positive
         */
        size_t longuest = (found_len > it->key_len)? found_len : it->key_len;
        if (!strncmp(it->key, found, longuest)) {
            it->count++;
            return;
        }
    }

    struct map tmp = {
        .key = strdup(found),
        .key_len = found_len,
        .count = 1
    };

    /* necessary to keep the values in the struct as 'const' */
    struct map *new_word = (struct map *)calloc(1, sizeof(struct map));
    memcpy(new_word, &tmp, sizeof(struct map));

    new_word->next = root->next;
    root->next = new_word;
}

static size_t next_word(char *buff, char **result)
{
    char *pos = buff;
    char *begin;
    size_t len = 0;

    if (!buff || !*buff) {
        *result = NULL;
        goto exit;
    }

    while (pos && *pos && !IS_LETTER(pos)) {
        pos++;
        len++;
    }

    if (!pos || !*pos) {
        *result = NULL;
        goto exit;
    }

    begin = pos;

    while (IS_LETTER(pos)) {
        len++;
        pos++;
    }

    if (len == 0) {
        *result = NULL;
        goto exit;
    }

    size_t result_len = pos - begin;
    *result = (char *)calloc(result_len+1, sizeof(char));
    strncpy(*result, begin, result_len);
    (*result)[result_len] = '\0';

exit:
    return len;
}

/**
 * main thread function (map phase)
 *
 * tokenize the given buffer
 * count occurences of each word found
 */
void *map(void *arg)
{
    struct threads_arg *args = (struct threads_arg *)arg;

    char *token = NULL;
    char *pos = args->buff;
    size_t offset;

    //printf("[%lu] |%lu| '%c|%c|%c'=>'%c|%c|%c'\n", args->tid, args->size, pos[0], pos[1], pos[2], pos[args->size-3], pos[args->size-2], pos[args->size-1]);

    offset = next_word(pos, &token);

    while (token && (pos + offset) < (args->buff + args->size) ) {
        add_inc_word(args->root, token, strlen(token));
        pos += offset;
        offset = next_word(pos, &token);
    }

    if (token) {
        add_inc_word(args->root, token, strlen(token));
        pos += offset;
    }

    return NULL;
}