/*
    Generic AVL tree implementation in C
    Copyright (C) 2017 Yagmur Oymak

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "bstree.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define OUT_LEN 30

struct word {
    char *str;
    struct bstree_node *nextwords;
    struct bstree_ops *ops;
    double cnt;
};

struct word_arr {
    struct word **arr;
    int capacity;
    int last;
};

double uniform_rnd(void)
{
    return (double)rand() / (double)RAND_MAX;
}

void fill_words(void *curr, void *words)
{
    struct word_arr *word_arr = words;
    struct word *word = curr;
    if (word_arr->last >= word_arr->capacity - 1) {
        word_arr->capacity *= 2;
        word_arr->arr =
            realloc(word_arr->arr, word_arr->capacity * sizeof *word_arr->arr);
    }
    word_arr->arr[++word_arr->last] = word;
}

struct word *choose_next(struct word *curr)
{
    struct word *chosen;
    struct word_arr choices = { malloc(sizeof *choices.arr), 1, -1 };
    double sum, rnd;
    int i;

    bstree_traverse_inorder(curr->nextwords, &choices, fill_words);
    for (i = 0, sum = 0, rnd = uniform_rnd(); i <= choices.last; i++) {
        if ((sum += choices.arr[i]->cnt) >= rnd) {
            chosen = choices.arr[i];
            free(choices.arr);
            return chosen;
        }
    }
    assert(0);
}

int cmp_word(const void *lhs, const void *rhs)
{
    return strcmp(((struct word *)lhs)->str, ((struct word *)rhs)->str);
}

void free_word(void *p)
{
    struct word *word = p;
    bstree_destroy(word->nextwords, word->ops);
    free(word);
}

struct word *mkword(char *str, struct bstree_ops *ops)
{
    struct word *w = malloc(sizeof *w);
    w->str = str;
    w->ops = ops;
    w->nextwords = NULL;
    w->cnt = 1;
    return w;
}

void print_word(void *p, void *it_data)
{
    struct word *w = p;
    printf("    %s : %.2f\n", w->str, w->cnt);
}

void print_tree(void *p, void *it_data)
{
    struct word *word = p;
    printf("%s\n", word->str);
    bstree_traverse_inorder(word->nextwords, NULL, print_word);
}

void sum_transition(void *p, void *it_data)
{
    double *sum = it_data;
    struct word *w = p;
    *sum += w->cnt;
}

void normalize_counts(void *p, void *it_data)
{
    struct word *word = p;
    word->cnt /= *(double *)it_data;
}

void normalize_transitions(void *p, void *it_data)
{
    struct word *word;
    double sum;
    word = p;
    sum = word->cnt;
    bstree_traverse_inorder(word->nextwords, &sum, normalize_counts);
}

struct bstree_node *add_transition(struct bstree_node *root, struct bstree_ops *ops,
        char *curr, char *next)
{
    struct word key;
    struct word *word, *nextword;
    key.str = curr;
    word = bstree_search(root, ops, &key);
    if (!word) {
        word = mkword(curr, ops);
        root = bstree_insert(root, ops, word);
    } else {
        word->cnt++;
    }
    key.str = next;
    nextword = bstree_search(word->nextwords, word->ops, &key);
    if (!nextword) {
        nextword = mkword(next, ops);
        word->nextwords = bstree_insert(word->nextwords, ops, nextword);
    } else {
        nextword->cnt++;
    }
    return root;
}

int main(int argc, char **argv)
{
    struct bstree_node *root = NULL;
    struct bstree_ops ops = { cmp_word, free_word };

    char *line = NULL;
    size_t len = 0;
    char *curr, *next;

    struct word *initial;
    struct word key_word;

    int i;

    srand(time(NULL));

    getline(&line, &len, stdin);
    *strrchr(line, '\n') = '\0';

    curr = strtok(line, " ");
    while ((next = strtok(NULL, " ")) != NULL) {
        root = add_transition(root, &ops, curr, next);
        curr = next;
    }
    root = add_transition(root, &ops, curr, line);
    bstree_traverse_inorder(root, &ops, normalize_transitions);
//    bstree_traverse_inorder(root, &ops, print_tree);

    if (argc != 2) {
        /* Choose first word of the input */
        key_word.str = line;
    } else {
        key_word.str = argv[1];
    }
    for (i = 0; i < OUT_LEN; i++) {
        initial = bstree_search(root, &ops, &key_word);
        assert(initial || (argc == 2 && i == 0));
        printf("%s ", initial->str);
        key_word.str = choose_next(initial)->str;
    }
    printf("%s\n", initial->str);

    bstree_destroy(root, &ops);
    free(line);
    return 0;
}
