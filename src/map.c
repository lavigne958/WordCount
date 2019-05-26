#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "utils.h"

struct node *allocate_new_node(const char *word, const size_t word_len, u_int32_t count)
{
    struct node *n = (struct node *)calloc(1, sizeof(struct node));
    if (unlikely(!n)) {
        printf("Could not allocaet tree node for '%s'\n", word);
        return NULL;
    }

    n->count = count;
    n->key = strdup(word);
    n->key_len = word_len;
    n->left = NULL;
    n->right = NULL;

    return n;
}

static struct node *add_inc_word(struct node *node, const char *found, const size_t found_len)
{
    if (!node) {
        return allocate_new_node(found, found_len, 1);
    }

    size_t longuest = LONGUEST_STR(node->key_len, found_len);
    int cmp = strncmp(found, node->key, longuest);

    // insert in right side of the tree
    if (cmp > 0) {
        node->right = add_inc_word(node->right, found, found_len);

    } else if (cmp < 0) {
        node->left = add_inc_word(node->left, found, found_len);

    } else {
        node->count++;
    }

    return node;
}

/**
 * copy the (potential) implementation
 * of strcpy from man pages
 * to introduce => tolower() call to lower string
 */
static inline void strncopy_and_lower(char *dest, char *src, size_t len)
{
    size_t i;

    for (i = 0; i < len && src[i] != '\0'; i++)
        dest[i] = tolower(src[i]);
    for ( ; i < len; i++)
        dest[i] = '\0';
}

static size_t next_word(char *buff, char **result)
{
    char *pos = buff;
    char *begin;
    size_t len = 0;

    //clean up previous token found
    if (*result) {
        free(*result);
    }

    if (!buff || !*buff) {
        *result = NULL;
        goto exit;
    }

    while (pos && *pos && !IS_LETTER(*pos)) {
        pos++;
        len++;
    }

    if (!pos || !*pos) {
        *result = NULL;
        goto exit;
    }

    begin = pos;

    while (IS_LETTER(*pos)) {
        len++;
        pos++;
    }

    if (len == 0) {
        *result = NULL;
        goto exit;
    }

    size_t result_len = pos - begin;

    *result = (char *)calloc(result_len+1, sizeof(char));
    if (unlikely(!*result)) {
        printf("Could not alloca memory for token");
        *result = NULL;
        goto exit;
    }

    strncopy_and_lower(*result, begin, result_len);
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

    offset = next_word(pos, &token);

    while (token && (pos + offset) < (args->buff + args->size) ) {
        //printf("[%lu] found '%s'\n", args->tid, token);
        args->tree->root = add_inc_word(args->tree->root, token, strlen(token));
        args->tree->nr_nodes++;
        pos += offset;
        offset = next_word(pos, &token);
    }

    if (token) {
        add_inc_word(args->tree->root, token, strlen(token));
        args->tree->nr_nodes++;
        pos += offset;
        free(token);
    }

    return NULL;
}