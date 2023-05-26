# hash_table.h

An extremely simple hash table implementation.

## Usage
```c
#include "hashtable.h"

HashTable *table = hashtable_create();

// Set values
hashtable_set(table, "mia", "the best");
hashtable_set(table, "federer", 1);
hashtable_set(table, "djokovic", 2);
hashtable_set(table, "fav letter", 'm');

// Get values
hashtable_get(table, "mia") // returns "the best"
hashtable_get(table, "federer") // returns 1
... // you get it
```
