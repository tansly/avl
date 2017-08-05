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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

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

struct options {
    char *initial_word;
    char *delimiter;
    int out_len;
    int print_stats;
    int wrap;
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

void print_usage(char **argv)
{
    fprintf(stderr, "Usage: %s [-l out_len] [-i initial_word] [-t]"
            " [-d delimiter] [-w]\n", argv[0]);
    fprintf(stderr, "-l\t\tLength (in words) of the generated sequence\n");
    fprintf(stderr, "-i\t\tInitial word of the sequence\n");
    fprintf(stderr, "-t\t\tPrint the transition statistics\n");
    fprintf(stderr, "-d delimiter\tWord delimiter string, default is space\n");
    fprintf(stderr, "-w\t\tWrap output if longer than 80 characters\n");
}

/* Parse the command line options and place them in opts.
 * Returns 0 on succes, nonzero on failure.
 */
int parse_opts(int argc, char **argv, struct options *opts)
{
    int opt;
    opts->out_len = OUT_LEN;
    opts->initial_word = NULL;
    opts->print_stats = 0;
    opts->wrap = 0;
    opts->delimiter = " ";
    while ((opt = getopt(argc, argv, "l:i:d:tw")) != -1) {
        switch (opt) {
            case 'l':
                opts->out_len = strtol(optarg, NULL, 10);
                if (errno == EINVAL || errno == ERANGE) {
                    return 1;
                }
                break;
            case 'i':
                if (!optarg) {
                    return 1;
                }
                opts->initial_word = optarg;
                break;
            case 't':
                opts->print_stats = 1;
                break;
            case 'd':
                if (!optarg) {
                    return 1;
                }
                opts->delimiter = optarg;
                break;
            case 'w':
                opts->wrap = 1;
                break;
            default:
                return 1;
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    struct options cli_opts;

    struct bstree_node *root;
    struct bstree_ops ops;

    struct word *initial;
    struct word key_word;

    char *line;
    size_t bufsize;
    ssize_t read_len;
    char *curr, *next;

    int i, line_len;

    if (parse_opts(argc, argv, &cli_opts)) {
        print_usage(argv);
        return 1;
    }

    srand(time(NULL));

    /* Initialize the tree */
    root = NULL;
    ops.compare_object = cmp_word;
    ops.free_object = free_word;

    curr = NULL;
    line = NULL;
    bufsize = 0;
    while ((read_len = getline(&line, &bufsize, stdin)) != EOF) {
        if (line[read_len - 1] == '\n') {
            line[read_len - 1] = '\0';
        }
        next = strtok(line, cli_opts.delimiter);
        while (next) {
            if (curr) {
                root = add_transition(root, &ops, curr, next);
                free(curr);
            }
            curr = strdup(next);
            next = strtok(NULL, cli_opts.delimiter);
        }
    }
    /* Add a transition from the last word to itself */
    root = add_transition(root, &ops, curr, curr);
    free(curr);

    bstree_traverse_inorder(root, &ops, normalize_transitions);
    if (cli_opts.print_stats) {
        bstree_traverse_inorder(root, &ops, print_tree);
    }

    /* Set initial word */
    if (!cli_opts.initial_word) {
        key_word.str = ((struct word *)root->object)->str;
    } else {
        key_word.str = cli_opts.initial_word;
    }

    for (i = 0, line_len = 0; i < cli_opts.out_len; i++) {
        initial = bstree_search(root, &ops, &key_word);
        assert(initial || (argc == 2 && i == 0));
        if (cli_opts.wrap && line_len >= 80) {
            putchar('\n');
            line_len = 0;
        }
        line_len += printf("%s%s", initial->str, cli_opts.delimiter);
        key_word.str = choose_next(initial)->str;
    }
    putchar('\n');

    bstree_destroy(root, &ops);
    free(line);
    return 0;
}
