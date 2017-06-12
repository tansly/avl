#include "bstree.h"

#include <stdio.h>
#include <stdlib.h>

int cmp_int(const void *lhs, const void *rhs)
{
    return *((const int *)lhs) - *((const int *)rhs);
}

void free_int(void *p)
{
    free(p);
}

void print_int(void *ptr)
{
    printf("%d\n", *(int *) ptr);
}

int main(void)
{
    struct bstree_node *root = NULL;
    struct bstree_ops ops = { cmp_int, free_int };
    int *arr[100];
    int i;
    for (i = 0; i < 100; i++) {
        arr[i] = malloc(sizeof(int));
        *arr[i] = rand() % 1000;
    }
    for (i = 0; i < 100; i++) {
        root = bstree_insert(root, &ops, arr[i]);
        arr[i] = NULL;
    }
    bstree_traverse_inorder(root, print_int);
    bstree_destroy(root, &ops);
    return 0;
}
