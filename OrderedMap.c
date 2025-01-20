/*******************************************************************************************/
/* OrderedMap.c - Implementation file for the OrderedMap module
 *
 * This file implements an ordered map using an AVL tree for self-balancing.
 * Internal details are hidden from the interface.
 *
 * Author: Adam Rubinstein
 * Date: January 2025
 */

#include "OrderedMap.h"
#include <stdlib.h>
#include <stdio.h>

// AVL Tree node structure
typedef struct AVLNode {
    double key;
    void *value;
    int height;
    struct AVLNode *left;
    struct AVLNode *right;
} *AVLNode;

struct OrderedMap {
    AVLNode root;
    size_t size;
};

struct OrderedMapIterator {
    OrderedMap map;
    AVLNode current;
};

// Helper function prototypes
static AVLNode create_node(double key, void *value);
static void destroy_node(AVLNode node);
static AVLNode insert_node(AVLNode node, double key, void *value, int *inserted);
static AVLNode remove_node(AVLNode node, double key, int *removed);
static AVLNode find_min(AVLNode node);
static int get_height(AVLNode node);
static int get_balance(AVLNode node);
static AVLNode rotate_right(AVLNode y);
static AVLNode rotate_left(AVLNode x);

// Public function implementations
// Takes a function to free values, or NULL
OrderedMap OrderedMap_create() {
    OrderedMap map = malloc(sizeof(struct OrderedMap));
    if (!map) return NULL;
    map->root = NULL;
    map->size = 0;
    return map;
}

void OrderedMap_destroy(OrderedMap *map) {
    if (!map || !*map) return;
    OrderedMap m = *map;

    destroy_node(m->root);
    free(m);

    *map = NULL;
}

int OrderedMap_insert(OrderedMap map, double key, void *value) {
    if (!map) return 0;
    int inserted = 0;
    map->root = insert_node(map->root, key, value, &inserted);
    if (inserted) map->size++;
    return inserted;
}

int OrderedMap_remove(OrderedMap map, double key) {
    if (!map) return 0;
    int removed = 0;
    map->root = remove_node(map->root, key, &removed);
    if (removed) map->size--;
    return removed;
}

int OrderedMap_get(const OrderedMap map, double key, void **value) {
    if (!map) return 0;
    AVLNode current = map->root;
    while (current) {
        if (key == current->key) {
            if (value) *value = current->value;
            return 1;
        }
        current = (key < current->key) ? current->left : current->right;
    }
    return 0;
}

int OrderedMap_get_min(const OrderedMap map, double *key, void **value) {
    if (!map || !map->root) return 0;
    AVLNode min = find_min(map->root);
    if (key) *key = min->key;
    if (value) *value = min->value;
    return 1;
}

int OrderedMap_get_max(const OrderedMap map, double *key, void **value) {
    if (!map || !map->root) return 0;
    AVLNode max = map->root;
    while (max->right) max = max->right;
    if (key) *key = max->key;
    if (value) *value = max->value;
    return 1;
}

size_t OrderedMap_size(const OrderedMap map) {
    return map ? map->size : 0;
}

OrderedMapIterator OrderedMap_front(OrderedMap map) {
    if (!map) return NULL;
    OrderedMapIterator iter = malloc(sizeof(struct OrderedMapIterator));
    if (!iter) return NULL;
    iter->map = map;
    iter->current = map->root;
    while (iter->current && iter->current->left) iter->current = iter->current->left;
    return iter;
}

OrderedMapIterator OrderedMap_back(OrderedMap map) {
    if (!map) return NULL;
    OrderedMapIterator iter = malloc(sizeof(struct OrderedMapIterator));
    if (!iter) return NULL;
    iter->map = map;
    iter->current = map->root;
    while (iter->current && iter->current->right) iter->current = iter->current->right;
    return iter;
}

void OrderedMapIterator_destroy(OrderedMapIterator *iter) {
    if (!iter || !*iter) return;
    free(*iter);
    *iter = NULL;
}

int OrderedMapIterator_get(OrderedMapIterator iter, double *key, void **value) {
    if (value) *value = NULL;
    if (!iter || !iter->current) return 0;
    if (key) *key = iter->current->key;
    if (value) *value = iter->current->value;
    return 1;
}

// Returns 0 if the iterator is at the end of the map
int OrderedMapIterator_next(OrderedMapIterator iter) {
    if (!iter || !iter->current) return 0;
    if (iter->current->right) {
        iter->current = find_min(iter->current->right);
    } else {
        AVLNode parent = NULL;
        AVLNode temp = iter->map->root;
        // First ancestor of current node whose left child is also ancestor of current node
        while (temp) {
            if (iter->current->key < temp->key) {
                parent = temp;
                temp = temp->left;
            } else if (iter->current->key > temp->key) {
                temp = temp->right;
            } else {
                break;
            }
        }
        iter->current = parent;
    }

    return iter->current ? 1 : 0;
}

int OrderedMapIterator_prev(OrderedMapIterator iter) {
    if (!iter || !iter->current) return 0;
    if (iter->current->left) { //Max of left subtree
        iter->current = iter->current->left;
        while (iter->current->right) iter->current = iter->current->right;
    } else {
        AVLNode parent = NULL;
        AVLNode temp = iter->map->root;
        while (temp) {
            if (iter->current->key > temp->key) {
                parent = temp;
                temp = temp->right;
            } else if (iter->current->key < temp->key) {
                temp = temp->left;
            } else {
                break;
            }
        }
        iter->current = parent;
    }

    return iter->current ? 1 : 0;
}

// Helper function implementations
static AVLNode create_node(double key, void *value) {
    AVLNode node = malloc(sizeof(struct AVLNode));
    if (!node) return NULL;
    node->key = key;
    node->value = value;
    node->height = 1;
    node->left = node->right = NULL;
    return node;
}

static void destroy_node(AVLNode node) {
    if (!node) return;
    destroy_node(node->left);
    destroy_node(node->right);
    free(node);
}

static AVLNode insert_node(AVLNode node, double key, void *value, int *inserted) {
    if (!node) {
        *inserted = 1;
        return create_node(key, value);
    }
    if (key < node->key) {
        node->left = insert_node(node->left, key, value, inserted);
    } else if (key > node->key) {
        node->right = insert_node(node->right, key, value, inserted);
    } else {
        node->value = value; // Update value if key exists
        *inserted = 0;
        return node;
    }

    node->height = 1 + (get_height(node->left) > get_height(node->right) ? get_height(node->left) : get_height(node->right));
    int balance = get_balance(node);

    // Balance the node
    if (balance > 1 && key < node->left->key) return rotate_right(node);
    if (balance < -1 && key > node->right->key) return rotate_left(node);
    if (balance > 1 && key > node->left->key) {
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }
    if (balance < -1 && key < node->right->key) {
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }

    return node;
}

static AVLNode remove_node(AVLNode node, double key, int *removed) {
    if (!node) return NULL;
    if (key < node->key) {
        node->left = remove_node(node->left, key, removed);
    } else if (key > node->key) {
        node->right = remove_node(node->right, key, removed);
    } else {
        *removed = 1;
        if (!node->left || !node->right) {
            AVLNode temp = node->left ? node->left : node->right;
            free(node);
            return temp;
        }
        AVLNode temp = find_min(node->right);
        node->key = temp->key;
        node->value = temp->value;
        node->right = remove_node(node->right, temp->key, removed);
    }

    if (!node) return NULL;
    node->height = 1 + (get_height(node->left) > get_height(node->right) ? get_height(node->left) : get_height(node->right));
    int balance = get_balance(node);

    if (balance > 1 && get_balance(node->left) >= 0) return rotate_right(node);
    if (balance > 1 && get_balance(node->left) < 0) {
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }
    if (balance < -1 && get_balance(node->right) <= 0) return rotate_left(node);
    if (balance < -1 && get_balance(node->right) > 0) {
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }

    return node;
}

static AVLNode find_min(AVLNode node) {
    while (node && node->left) node = node->left;
    return node;
}

static int get_height(AVLNode node) {
    return node ? node->height : 0;
}

static int get_balance(AVLNode node) {
    return node ? get_height(node->left) - get_height(node->right) : 0;
}

static AVLNode rotate_right(AVLNode y) {
    AVLNode x = y->left;
    AVLNode T = x->right;

    x->right = y;
    y->left = T;

    y->height = 1 + (get_height(y->left) > get_height(y->right) ? get_height(y->left) : get_height(y->right));
    x->height = 1 + (get_height(x->left) > get_height(x->right) ? get_height(x->left) : get_height(x->right));

    return x;
}

static AVLNode rotate_left(AVLNode x) {
    AVLNode y = x->right;
    AVLNode T = y->left;

    y->left = x;
    x->right = T;

    x->height = 1 + (get_height(x->left) > get_height(x->right) ? get_height(x->left) : get_height(x->right));
    y->height = 1 + (get_height(y->left) > get_height(y->right) ? get_height(y->left) : get_height(y->right));

    return y;
}