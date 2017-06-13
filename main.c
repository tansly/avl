#include "bstree.h"

#include <stdio.h>
#include <stdlib.h>

#define ARR_SIZE 10

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

int main(void)
{
    struct bstree_node *root = NULL;
    struct bstree_ops ops = { cmp_int, free_int };
    int *arr[ARR_SIZE];
    int i, sum = 0;
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
    printf("\n%d\n", root->height);
    printf("\n%d\n", sum);
    bstree_destroy(root, &ops);
    return 0;
}
