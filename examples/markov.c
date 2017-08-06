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

struct cli_opts {
    char *initial_word;
    char *delimiter;
    int out_len;
    int print_stats;
    int wrap;
};

struct word {
    char *str;
    struct bstree *nextwords;
    double cnt;
};

struct random_choice {
    struct word *word;
    double rnd;
    double sum;
};

static double uniform_rnd(void)
{
    return (double)rand() / (double)RAND_MAX;
}

static int choose_word(void *currp, void *choicep)
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

static struct word *choose_next(struct word *curr)
{
    struct random_choice choice = { NULL, uniform_rnd(), 0 };
    bstree_traverse_inorder(curr->nextwords, &choice, choose_word);
    return choice.word;
}

static int cmp_word(const void *lhs, const void *rhs)
{
    return strcmp(((struct word *)lhs)->str, ((struct word *)rhs)->str);
}

static void free_word(void *p)
{
    struct word *word = p;
    bstree_destroy(word->nextwords);
    free(word->str);
    free(word);
}

static struct word *mkword(char *str)
{
    struct word *w = malloc(sizeof *w);
    w->str = strdup(str);
    w->nextwords = bstree_new(cmp_word, free_word);
    w->cnt = 1;
    return w;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

static int print_word(void *p, void *it_data)
{
    struct word *w = p;
    printf("    %s : %.2f\n", w->str, w->cnt);
    return 0;
}

static int print_tree(void *p, void *it_data)
{
    struct word *word = p;
    printf("%s\n", word->str);
    bstree_traverse_inorder(word->nextwords, NULL, print_word);
    return 0;
}

#pragma GCC diagnostic pop

static int normalize_counts(void *p, void *it_data)
{
    struct word *word = p;
    word->cnt /= *(double *)it_data;
    return 0;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

static int normalize_transitions(void *p, void *it_data)
{
    struct word *word;
    double sum;
    word = p;
    sum = word->cnt;
    bstree_traverse_inorder(word->nextwords, &sum, normalize_counts);
    return 0;
}

#pragma GCC diagnostic pop

static void add_transition(struct bstree *table, char *curr, char *next)
{
    struct word key;
    struct word *word, *nextword;
    key.str = curr;
    word = bstree_search(table, &key);
    if (!word) {
        word = mkword(curr);
        bstree_insert(table, word);
    } else {
        word->cnt++;
    }
    key.str = next;
    nextword = bstree_search(word->nextwords, &key);
    if (!nextword) {
        nextword = mkword(next);
        bstree_insert(word->nextwords, nextword);
    } else {
        nextword->cnt++;
    }
}

static void print_usage(char **argv)
{
    fprintf(stderr, "Usage: %s -i initial_word [-l out_len] [-t]"
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
static int parse_opts(int argc, char **argv, struct cli_opts *opts)
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
    /* If there is no initial word set, we fail */
    return !opts->initial_word;
}

static struct bstree *generate_transition_table(struct cli_opts *opts)
{
    char *line;
    size_t bufsize;
    ssize_t read_len;
    char *curr, *next;
    struct bstree *table;
    table = bstree_new(cmp_word, free_word);
    for (curr = NULL, line = NULL, bufsize = 0;
            (read_len = getline(&line, &bufsize, stdin)) != EOF; ) {
        if (line[read_len - 1] == '\n') {
            line[read_len - 1] = '\0';
        }
        next = strtok(line, opts->delimiter);
        while (next) {
            if (curr) {
                add_transition(table, curr, next);
                free(curr);
            }
            curr = strdup(next);
            next = strtok(NULL, opts->delimiter);
        }
    }
    if (!curr) {
        /* Empty input */
        free(line);
        return table;
    }
    /* Add a transition from the last word to itself */
    add_transition(table, curr, curr);
    bstree_traverse_inorder(table, NULL, normalize_transitions);
    free(curr);
    free(line);
    return table;
}

static void print_transition_table(struct bstree *table)
{
    bstree_traverse_inorder(table, NULL, print_tree);
}

static void generate_chain(struct bstree *table, struct cli_opts *opts)
{
    struct word *initial;
    struct word key_word;
    int i, line_len;
    if (bstree_size(table) == 0) {
        /* Empty input */
        return;
    }
    key_word.str = opts->initial_word;
    for (i = 0, line_len = 0; i < opts->out_len; i++) {
        initial = bstree_search(table, &key_word);
        if (!initial) {
            if (i == 0 && opts->initial_word) {
                fprintf(stderr, "Initial word not found in dictionary."
                        " Make sure you have supplied a word that really exists"
                        " in the text.\n");
            } else {
                /* Oh, well... */
                fprintf(stderr, "This error does not exist.\n");
            }
            break;
        }
        if (opts->wrap && line_len >= 80) {
            putchar('\n');
            line_len = 0;
        }
        line_len += printf("%s%s", initial->str, opts->delimiter);
        key_word.str = choose_next(initial)->str;
    }
    putchar('\n');
}

int main(int argc, char **argv)
{
    struct bstree *table;
    struct cli_opts opts;
    if (parse_opts(argc, argv, &opts)) {
        print_usage(argv);
        return 1;
    }
    srand(time(NULL));
    table = generate_transition_table(&opts);
    if (opts.print_stats) {
        print_transition_table(table);
    }
    generate_chain(table, &opts);
    bstree_destroy(table);
    return 0;
}
