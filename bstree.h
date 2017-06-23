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

/* Some notes:
 ** We directly expose the node structure to the user, hiding nothing.
 ** No duplicate keys will be present in the tree, inserting an already existing
 * value will increase the count held at the node. Functions that have
 * "cnt" as a suffix in their names operate considering the count at the found node,
 * with details explained for each of them below.
 ** The functions that modify the tree structure return the new root node
 * of the tree. So calls should be made like: root = bstree_insert(root, ...
 ** NULL represents an empty tree.
 */

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
     * we take the ownership of the resources, and we should eventually
     * free every pointer we are given if we are still holding it in the end.
     * If the freeing function is NULL, we just keep the pointers to the
     * objects and never free them, that means the user manages the lifetime.
     */
    void (*free_object)(void *object);
};

/* Inserts the given object to the tree. If the object already exists,
 * increment the count.
 */
struct bstree_node *bstree_insert(struct bstree_node *root,
        const struct bstree_ops *ops, void *object);

/* Destroy everything, ggwp.
 */
void bstree_destroy(struct bstree_node *root, const struct bstree_ops *ops);

/* Traverse the tree with a given operation, optionally accumulating
 * data in it_data. 'operation' must point to a valid function.
 * Usage of it_data is up to the operation function given by the user.
 * It is not used anywhere by us.
 * Traversal is stopped when the operation returns true.
 * Returns true if traversal is stopped by the operation function,
 * else false (traversal is finished by visiting all nodes).
 */
int bstree_traverse_inorder(const struct bstree_node *root,
        void *it_data,
        int (*operation)(void *object, void *it_data));

/* Traverse the tree in a similar fashion, but apply the operation 'count' times
 * for each object, 'count' being the count of the object held in the tree.
 */
int bstree_traverse_inorder_cnt(const struct bstree_node *root,
        void *it_data,
        int (*operation)(void *object, void *it_data));

/* Return the count of the given key.
 */
int bstree_count(const struct bstree_node *root, const struct bstree_ops *ops,
        const void *key);

/* Return the pointer to the object matching the given key.
 */
void *bstree_search(const struct bstree_node *root, const struct bstree_ops *ops,
        const void *key);

/* Finds and removes the node matching the given key.
 * If the user wants the node removed but does not want the object to be free'd,
 * he/she shall supply ops with ops->free_object set to NULL.
 * Does nothing if the given key is not found in the tree.
 */
struct bstree_node *bstree_remove(struct bstree_node *root,
        const struct bstree_ops *ops, const void *key);

#endif
