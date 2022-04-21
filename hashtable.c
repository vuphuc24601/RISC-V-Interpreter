#include <stdio.h>
#include <stdlib.h>
#include "linkedlist.h"
#include "hashtable.h"

struct hashtable {
    // TODO: define hashtable struct to use linkedlist as buckets
    int capacity;
    int num_buckets;
    int size;
    linkedlist_t **buckets;
};

/**
 * Hash function to hash a key into the range [0, max_range)
 */
static int hash(int key, int max_range) {
    // TODO: feel free to write your own hash function (NOT REQUIRED)
    key = (key > 0) ? key : -key;
    return key % max_range;
}

hashtable_t *ht_init(int num_buckets) {
    // TODO: create a new hashtable
    hashtable_t *table = malloc(sizeof(hashtable_t));
    table->num_buckets = num_buckets;
    table->size = 0;
    table->buckets = malloc(num_buckets * sizeof(linkedlist_t*));
    for (int i=0; i<num_buckets; i++) {
        table->buckets[i] = ll_init();
    }
    return table;
}

void ht_add(hashtable_t *table, int key, int value) {
    // TODO: create a new mapping from key -> value.
    // If the key already exists, replace the value.
    int index = hash(key, table->num_buckets);
    linkedlist_t *list = table->buckets[index];

    table->size -= ll_size(list);
    ll_add(table->buckets[index], key, value);
    table->size += ll_size(list);
}

int ht_get(hashtable_t *table, int key) {
    // TODO: retrieve the value mapped to the given key.
    // If it does not exist, return 0
    int index = hash(key, table->num_buckets);
    return ll_get(table->buckets[index], key);
}

int ht_size(hashtable_t *table) {
    // TODO: return the number of mappings in this hashtable
    return table->size;
}
