#ifndef BSTREE_H
#define BSTREE_H

struct bstree_node {
    void *object;
    struct bstree_node *left;
    struct bstree_node *right;
    int count;
    int height;
};

struct bstree_ops {
    int (*compare_object)(const void *lhs, const void *rhs);
    /* If the user supplies a function to free the objects, then we know that
     * we take the ownership of the resources, and we should eventually free every pointer
     * we are given if we are still holding it in the end. If the freeing function is NULL, we just keep
     * the pointers to the objects and never free them, that means the user manages the lifetime.
     */
    void (*free_object)(void *object);
};

struct bstree_node *bstree_insert(struct bstree_node *root, const struct bstree_ops *ops, void *object);

void bstree_destroy(struct bstree_node *root, const struct bstree_ops *ops);

/* Traverse the tree with a given operation, optionally accumulating
 * data in it_data. 'operation' must point to a valid function.
 * Usage of it_data is up to the operation function given by the user.
 * It is not used anywhere by us.
 */
void bstree_traverse_inorder(const struct bstree_node *root,
        void *it_data,
        void (*operation)(void *object, void *it_data));

int bstree_count(const struct bstree_node *root, const struct bstree_ops *ops, const void *object);

#endif
