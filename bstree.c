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

#define MAX(a,b) (((a) > (b)) ? (a) : (b))

#define MAX_IMBALANCE 1

static int int_max(int a, int b)
{
    return a > b ? a : b;
}

static int height(struct bstree_node *root)
{
    return root ? root->height : -1;
}

/* Make a node that is a valid tree consisting of one node, only the root.
 */
static struct bstree_node *mknode(void *object)
{
    struct bstree_node *root = malloc(sizeof *root);
    root->object = object;
    root->left = NULL;
    root->right = NULL;
    root->count = 1;
    root->height = 0;
    return root;
}

static struct bstree_node *rotate_with_left(struct bstree_node *root)
{
    struct bstree_node *newroot = root->left;
    root->left = newroot->right;
    newroot->right = root;
    root->height = int_max(height(root->left), height(root->right)) + 1;
    newroot->height = int_max(height(newroot->left), height(newroot->right)) + 1;
    return newroot;
}

static struct bstree_node *rotate_with_right(struct bstree_node *root)
{
    struct bstree_node *newroot = root->right;
    root->right = newroot->left;
    newroot->left = root;
    root->height = int_max(height(root->left), height(root->right)) + 1;
    newroot->height = int_max(height(newroot->left), height(newroot->right)) + 1;
    return newroot;
}

static struct bstree_node *double_with_left(struct bstree_node *root)
{
    root->left = rotate_with_right(root->left);
    root = rotate_with_left(root);
    return root;
}

static struct bstree_node *double_with_right(struct bstree_node *root)
{
    root->right = rotate_with_left(root->right);
    root = rotate_with_right(root);
    return root;
}

/* Assume the given tree is balanced or have an imbalance of 2.
 * We allow a maximum imbalance of 1, and we maintain it as long as the
 * tree exists. Therefore, a valid tree generated by us will either be balanced,
 * or imbalanced by 2 because of a recent insertion (or deletion).
 * If the latter is the case, this function restores the balance.
 */
static struct bstree_node *balance(struct bstree_node *root)
{
    if (!root) {
        return NULL;
    }
    if (height(root->left) - height(root->right) > MAX_IMBALANCE) {
        if (height(root->left->left) > height(root->left->right)) {
            root = rotate_with_left(root);
        } else {
            root = double_with_left(root);
        }
    } else if (height(root->right) - height(root->left) > MAX_IMBALANCE) {
        if (height(root->right->right) > height(root->right->left)) {
            root = rotate_with_right(root);
        } else {
            root = double_with_right(root);
        }
    }
    root->height = int_max(height(root->left), height(root->right)) + 1;
    return root;
}

static struct bstree_node *get_min(struct bstree_node *root)
{
    while (root && root->left) {
        root = root->left;
    }
    return root;
}

struct bstree_node *bstree_insert(struct bstree_node *root,
        const struct bstree_ops *ops, void *object)
{
    if (!root) {
        return mknode(object);
    }
    if (ops->compare_object(object, root->object) < 0) {
        root->left = bstree_insert(root->left, ops, object);
        return balance(root);
    }
    if (ops->compare_object(object, root->object) > 0) {
        root->right = bstree_insert(root->right, ops, object);
        return balance(root);
    }
    /* Inserting equal key. We are not going to hold the given pointer.
     * If it is us who manages the lifetime (the ops->free_object != NULL),
     * we should free it.
     */
    root->count++;
    if (ops->free_object) {
        ops->free_object(object);
    }
    return balance(root);
}

struct bstree_node *bstree_replace(struct bstree_node *root,
        const struct bstree_ops *ops, void *object)
{
    if (!root) {
        return mknode(object);
    }
    if (ops->compare_object(object, root->object) < 0) {
        root->left = bstree_insert(root->left, ops, object);
        return balance(root);
    }
    if (ops->compare_object(object, root->object) > 0) {
        root->right = bstree_insert(root->right, ops, object);
        return balance(root);
    }
    /* Inserting equal key. We are going to replace the existing object with
     * the new one. We shall free the object if we have to (ops->free_object != NULL),
     * then replace the pointer in the node.
     */
    if (ops->free_object) {
        ops->free_object(root->object);
    }
    root->object = object;
    return balance(root);
}

void bstree_destroy(struct bstree_node *root, const struct bstree_ops *ops)
{
    if (!root) {
        return;
    }
    bstree_destroy(root->left, ops);
    bstree_destroy(root->right, ops);
    if (ops->free_object) {
        ops->free_object(root->object);
    }
    free(root);
}

int bstree_traverse_inorder(const struct bstree_node *root,
        void *it_data,
        int (*operation)(void *object, void *it_data))
{
    return
        root &&
        (bstree_traverse_inorder(root->left, it_data, operation) ||
        operation(root->object, it_data) ||
        bstree_traverse_inorder(root->right, it_data, operation));
}

int bstree_traverse_inorder_cnt(const struct bstree_node *root,
        void *it_data,
        int (*operation)(void *object, void *it_data))
{
    int i;
    if (!root) {
        return 0;
    }
    if (bstree_traverse_inorder_cnt(root->left, it_data, operation)) {
        return 1;
    }
    for (i = 0; i < root->count; i++) {
        if (operation(root->object, it_data)) {
            return 1;
        }
    }
    if (bstree_traverse_inorder_cnt(root->right, it_data, operation)) {
        return 1;
    }
    return 0;
}

int bstree_count(const struct bstree_node *root,
        const struct bstree_ops *ops, const void *object)
{
    if (!root) {
        return 0;
    }
    if (ops->compare_object(object, root->object) < 0) {
        return bstree_count(root->left, ops, object);
    }
    if (ops->compare_object(object, root->object) > 0) {
        return bstree_count(root->right, ops, object);
    }
    return root->count;
}

void *bstree_search(const struct bstree_node *root,
        const struct bstree_ops *ops, const void *key)
{
    if (!root) {
        return NULL;
    }
    if (ops->compare_object(key, root->object) < 0) {
        return bstree_search(root->left, ops, key);
    }
    if (ops->compare_object(key, root->object) > 0) {
        return bstree_search(root->right, ops, key);
    }
    return root->object;
}

struct bstree_node *bstree_remove(struct bstree_node *root,
        const struct bstree_ops *ops, const void *key)
{
    if (!root) {
        return NULL;
    }
    if (ops->compare_object(key, root->object) < 0) {
         root->left = bstree_remove(root->left, ops, key);
         return balance(root);
    }
    if (ops->compare_object(key, root->object) > 0) {
        root->right = bstree_remove(root->right, ops, key);
        return balance(root);
    }
    /* Found the node to be deleted */
    if (!root->left || !root->right) {
        struct bstree_node *tmp = root->left ? root->left : root->right;
        if (ops->free_object) {
            ops->free_object(root->object);
        }
        free(root);
        return tmp;
    }
    /* Node to be deleted has two children */
    struct bstree_node *right_min = get_min(root->right);
    /* As we get the pointer held at right_min and put it inside root, we shall
     * remove right_min without free'ing the object it holds.
     * For this reason, we call the remove function with a NULL free_object.
     */
    struct bstree_ops tmp_ops = *ops;
    tmp_ops.free_object = NULL;
    if (ops->free_object) {
        ops->free_object(root->object);
    }
    root->object = right_min->object;
    root->right = bstree_remove(root->right, &tmp_ops, right_min->object);
    return balance(root);
}
