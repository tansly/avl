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

#include <stdio.h>
#include <stdlib.h>

#define ARR_SIZE 16

struct int_arr {
    int *arr;
    int last;
    int size;
};

int cmp_int(const void *lhs, const void *rhs)
{
    return *((const int *)lhs) - *((const int *)rhs);
}

void free_int(void *p)
{
    free(p);
}

void print_int(void *ptr, void *it_data)
{
    printf("%d\n", *(int *) ptr);
}

void sum_int(void *ptr, void *it_data)
{
    *(int *)it_data += *(int *)ptr;
}

void mk_array(void *ptr, void *it_data)
{
    struct int_arr *elements = it_data;
    if (elements->last >= elements->size - 1) {
        elements->arr = realloc(elements->arr, elements->size * 2 * sizeof(int));
        elements->size = elements->size * 2;
    }
    elements->arr[++elements->last] = *(int *)ptr;
}

int main(void)
{
    struct bstree_node *root = NULL;
    struct bstree_ops ops = { cmp_int, free_int };
    int *arr[ARR_SIZE];
    int i, sum = 0;
    struct int_arr elements = { malloc(sizeof(int)), -1, 1 };
    for (i = 0; i < ARR_SIZE; i++) {
        arr[i] = malloc(sizeof(int));
        *arr[i] = i;
    }
    for (i = 0; i < ARR_SIZE; i++) {
        root = bstree_insert(root, &ops, arr[i]);
        arr[i] = NULL;
    }
    bstree_traverse_inorder(root, NULL, print_int);
    bstree_traverse_inorder(root, &sum, sum_int);
    printf("\nheight = %d\n", root->height);
    printf("\nsum = %d\n", sum);
    bstree_traverse_inorder(root, &elements, mk_array);
    for (i = 0; i <= elements.last; i++) {
        printf("arr[%d] = %d\n", i, elements.arr[i]);
    }
    free(elements.arr);
    bstree_destroy(root, &ops);
    return 0;
}
