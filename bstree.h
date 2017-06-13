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

/* Traverse the tree in a similar fashion, but apply the operation 'count' times
 * for each object, 'count' being the count of the object held in the tree.
 */
void bstree_traverse_inorder_cnt(const struct bstree_node *root,
        void *it_data,
        void (*operation)(void *object, void *it_data));

int bstree_count(const struct bstree_node *root, const struct bstree_ops *ops, const void *object);

#endif
