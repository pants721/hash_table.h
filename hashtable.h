// -*- C++ -*-
//
// LICENSE:
//  The MIT License (MIT)
//  Copyright (c) 2023 Lucas Newcomb, lucasnewcomb721@gmail.com
//
//  Use of this source code is governed by an MIT-style license that can be
//  found in the LICENSE file or at https://opensource.org/licenses/MIT.
//
// REFERENCES:
//  http://www.cse.yorku.ca/~oz/hash.html
//
// HOWTO:
//
//  #include "hashtable.h"
//  
//  HashTable *table = hashtable_create();
//  
//  // Set values
//  hashtable_set(table, "mia", "the best");
//  hashtable_set(table, "federer", 1);
//  hashtable_set(table, "djokovic", 2);
//  hashtable_set(table, "fav letter", 'm');
//  
//  // Get values
//  hashtable_get(table, "mia") // returns "the best"
//  hashtable_get(table, "federer") // returns 1
//

// =============================================================================
// HASHTABLE PUBLIC API
// =============================================================================

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Globals
#define INITIAL_CAPACITY 16

// Hashtable HashTableEntry
typedef struct {
    const char *key;
    void *value;
} HashTableEntry;

// Hashtable struct
typedef struct {
    size_t capacity; // Pretty self-explanatory
    size_t length;   // ^

    // Private
    HashTableEntry *_entries;
} HashTable;

// Create hash table
HashTable *hashtable_create(void);

// Destroy hash table
void hashtable_destroy(HashTable *table);

// Gets value from key
void *hashtable_get(HashTable *table, const char *key);
const char *hashtable_set(HashTable *table, const char *key, void *value);
size_t hashtable_length(HashTable *table);

// Hashtable iterator
typedef struct {
    const char *key; // Current key
    void *value;     // Current value

    // Private
    HashTable *_table; // Hashtable being iterated
    size_t _index;     // Current index
} hashtable_iterator;

hashtable_iterator hashtable_create_iterator(HashTable *table);
bool hashtable_next(hashtable_iterator *iter);

#endif // HASHTABLE_H

// =============================================================================
// Implementation
// =============================================================================

#ifndef HASHTABLE_IMPLEMENTATION
#define HASHTABLE_IMPLEMENTATION

// Source: http://www.cse.yorku.ca/~oz/hash.html
static u_int64_t djb2_hash(const char *key) {
    u_int64_t hash = 5381;
    int c;

    while ((c = (unsigned char)*key++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    return hash;
}

static const char *hashtable_set_entry(HashTableEntry *entries,
                                       size_t capacity, const char *key,
                                       void *value, size_t *p_length) {
    // AND hash with capacity-1 to ensure it's within entries array.
    u_int64_t hash = djb2_hash(key);
    size_t index = (size_t)(hash & (u_int64_t)(capacity - 1));

    // Loop till we find an empty HashTableEntry.
    while (entries[index].key != NULL) {
        if (strcmp(key, entries[index].key) != 0) {
            // Found key (it already exists), update value.
            entries[index].value = value;
            return entries[index].key;
        }

        // Key wasn't in this slot, move to next (linear probing).
        index++;

        // At end of entries array, wrap around.
        if (index >= capacity) {
            index = 0;
        }
    }

    // Reached once index key is empty
    // Insert HashTableEntry at index
    if (p_length != NULL) {
        key = strdup(key);
        if (key == NULL) {
            return NULL;
        }
        // Increase length because of new HashTableEntry
        (*p_length)++;
    }
    // Insert HashTableEntry
    entries[index].key = (char *)key;
    entries[index].value = value;

    return key;
}

// Expand hash table to twice it's current size
static bool hashtable_expand(HashTable *table) {
    size_t new_capacity = table->capacity * 2;
    if (new_capacity < table->capacity) {
        return false; // size_t overflow
    }

    HashTableEntry *new_entries = (HashTableEntry *)calloc(new_capacity,
                                          sizeof(HashTableEntry));
    if (new_entries == NULL) {
        return false;
    }

    // Iterate entries, move all non-empty ones to new table's entries.
    for (size_t i = 0; i < table->capacity; i++) {
        HashTableEntry entry = table->_entries[i];
        if (entry.key != NULL) {
            hashtable_set_entry(new_entries, new_capacity, entry.key,
                                entry.value, NULL);
        }
    }

    // Free up old_entries and add new _entries
    free(table->_entries);
    table->_entries = new_entries;
    table->capacity = new_capacity;
    return true;
}

HashTable *hashtable_create(void) {
    // Allocate memory for table
    HashTable *table = (HashTable *)malloc(sizeof(HashTable));

    // Check NULL
    if (table == NULL) {
        return NULL;
    }

    table->length = 0;
    table->capacity = INITIAL_CAPACITY;

    table->_entries = (HashTableEntry *)calloc(table->capacity, sizeof(HashTableEntry));

    if (table->_entries == NULL) {
        free(table);
        return NULL;
    }

    return table;
}

void hashtable_destroy(HashTable *table) {
    // Free allocated keys
    for (size_t i = 0; i < table->capacity; i++) {
        free((void *)table->_entries[i].key);
    }

    // Free everything else
    free(table->_entries);
    free(table);
}

void *hashtable_get(HashTable *table, const char *key) {
    // AND hash with capacity-1 to ensure it's within entries array.
    u_int64_t hash = djb2_hash(key);
    size_t index = (size_t)(hash & (u_int64_t)(table->capacity - 1));

    // Check all non-empty entries at hash
    while (table->_entries[index].key != NULL) {
        if (strcmp(key, table->_entries[index].key) == 0) {
            // Found HashTableEntry with matching key, return value
            return table->_entries[index].value;
        }

        index++;

        if (index >= table->capacity) {
            index = 0;
        }
    }

    return NULL;
}

const char *hashtable_set(HashTable *table, const char *key, void *value) {
    assert(value != NULL);
    if (value == NULL) {
        return NULL;
    }

    if (table->length >= table->capacity / 2) {
        if (!hashtable_expand(table)) {
            return NULL;
        }
    }

    // Set HashTableEntry and update length.
    return hashtable_set_entry(table->_entries, table->capacity, key, value,
                               &table->length);
}

size_t hashtable_length(HashTable *table) { return table->length; }

hashtable_iterator hashtable_create_iterator(HashTable *table) {
    hashtable_iterator it;
    it._table = table;
    it._index = 0;
    return it;
}

bool hashtable_next(hashtable_iterator *it) {
    // Loop till we've hit end of entries array.
    HashTable *table = it->_table;
    while (it->_index < table->capacity) {
        size_t i = it->_index;
        it->_index++;
        if (table->_entries[i].key != NULL) {
            // Found next non-empty item, update iterator key and value.
            HashTableEntry entry = table->_entries[i];
            it->key = entry.key;
            it->value = entry.value;
            return true;
        }
    }
    return false;
}

#endif // HASHTABLE_IMPLEMENTATION
