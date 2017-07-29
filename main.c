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

#define _GNU_SOURCE
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

struct random_choice {
    struct word *word;
    double rnd;
    double sum;
};

double uniform_rnd(void)
{
    return (double)rand() / (double)RAND_MAX;
}

int choose_word(void *currp, void *choicep)
{
    struct word *curr = currp;
    struct random_choice *choice = choicep;
    if (choice->sum < choice->rnd) {
        choice->word = curr;
        choice->sum += curr->cnt;
        return 0;
    } else {
        return 1;
    }
}

struct word *choose_next(struct word *curr)
{
    struct random_choice choice = { NULL, uniform_rnd(), 0 };
    bstree_traverse_inorder(curr->nextwords, &choice, choose_word);
    return choice.word;
}

int cmp_word(const void *lhs, const void *rhs)
{
    return strcmp(((struct word *)lhs)->str, ((struct word *)rhs)->str);
}

void free_word(void *p)
{
    struct word *word = p;
    bstree_destroy(word->nextwords, word->ops);
    free(word->str);
    free(word);
}

struct word *mkword(char *str, struct bstree_ops *ops)
{
    struct word *w = malloc(sizeof *w);
    w->str = strdup(str);
    w->ops = ops;
    w->nextwords = NULL;
    w->cnt = 1;
    return w;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

int print_word(void *p, void *it_data)
{
    struct word *w = p;
    printf("    %s : %.2f\n", w->str, w->cnt);
    return 0;
}

int print_tree(void *p, void *it_data)
{
    struct word *word = p;
    printf("%s\n", word->str);
    bstree_traverse_inorder(word->nextwords, NULL, print_word);
    return 0;
}

#pragma GCC diagnostic pop

int sum_transition(void *p, void *it_data)
{
    double *sum = it_data;
    struct word *w = p;
    *sum += w->cnt;
    return 0;
}

int normalize_counts(void *p, void *it_data)
{
    struct word *word = p;
    word->cnt /= *(double *)it_data;
    return 0;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

int normalize_transitions(void *p, void *it_data)
{
    struct word *word;
    double sum;
    word = p;
    sum = word->cnt;
    bstree_traverse_inorder(word->nextwords, &sum, normalize_counts);
    return 0;
}

#pragma GCC diagnostic pop

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
    char *curr = NULL, *next;

    struct word *initial;
    struct word key_word;

    int i;

    srand(time(NULL));

    while (getline(&line, &len, stdin) != EOF) {
        *strchrnul(line, '\n') = '\0';
        next = strtok(line, " ");
        while (next) {
            if (curr) {
                root = add_transition(root, &ops, curr, next);
                free(curr);
            }
            curr = strdup(next);
            next = strtok(NULL, " ");
        }
    }
    /* Add a transition from the last word to itself */
    root = add_transition(root, &ops, curr, curr);
    free(curr);

    bstree_traverse_inorder(root, &ops, normalize_transitions);
    //bstree_traverse_inorder(root, &ops, print_tree);

    /* Set initial word */
    if (argc != 2) {
        key_word.str = ((struct word *)root->object)->str;
    } else {
        key_word.str = argv[1];
    }

    for (i = 0; i < OUT_LEN; i++) {
        initial = bstree_search(root, &ops, &key_word);
        assert(initial || (argc == 2 && i == 0));
        printf("%s ", initial->str);
        key_word.str = choose_next(initial)->str;
    }
    putchar('\n');

    bstree_destroy(root, &ops);
    free(line);
    return 0;
}
