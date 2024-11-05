/*
    Copyright (C) 2023 Nikita Retr0-code Korneev

    c_hash_table is free software: you can redistribute, modify it
    under the terms of the GNU Affero General Public License version 3.

    You should have received a copy of GNU Affero General Public License
    version 3 along with hash_dumper.
    If not, see <https://www.gnu.org/licenses/agpl-3.0.html>.


----

    This files defines an API for hashtable functions that allows full control over it
*/

#include "hashtable.h"

int ht_init(size_t size, hash_func_t ht_func_ptr, hashtable_t* hash_table)
{
    // Validating parameters
    if (hash_table == NULL || size == 0)
    {
        errno = EPERM;
        return -1;
    }

    // Setting default hash function
    if (ht_func_ptr == NULL)
        ht_func_ptr = &fnv_1a;

    hash_table->size = size;
    hash_table->filled_nodes = 0;
    hash_table->hash_func = ht_func_ptr;

    // Allocation of storage for hashtable values
    hash_table->data = malloc(hash_table->size * sizeof(ht_node_t*));
    if (hash_table->data == NULL)
        return -4;

    // Setting everything with NULL
    memset(hash_table->data, NULL, hash_table->size * sizeof(ht_node_t*));

    return 0;
}

int ht_resize(size_t new_size, hashtable_t* hash_table)
{
    // Validating parameters
    if (new_size == 0 || hash_table == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    // Saving old size to iterate further throug all elements
    size_t old_size = hash_table->size;
    hash_table->size = new_size;

    // Allocating new space for data
    ht_node_t** new_data = malloc(hash_table->size * sizeof(ht_node_t*));
    if (new_data == NULL)
        return -2;

    // Setting everything to NULL
    memset(new_data, NULL, hash_table->size * sizeof(ht_node_t*));

    // Iterating through original hash table
    for (size_t i = 0; i < old_size; i++)
    {
        if (hash_table->data[i] == NULL)
            continue;

        // Rehashing branches
        ht_node_rehash_from_base(hash_table->data[i], new_data, hash_table);
    }

    // Delete original hashtable memory
    free(hash_table->data);
    // Setting rehashed data
    hash_table->data = new_data;
    return 0;
}

int ht_emplace(const char* key, void* value, hashtable_t* hash_table)
{
    // Validating parameters
    if (key == NULL || hash_table == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    // Checking if resizing required
    if (hash_table->filled_nodes == hash_table->size - 1)
        ht_resize(hash_table->size * 2, hash_table);

    // Initialization of new cell in table
    ht_node_t* new_node = malloc(sizeof(ht_node_t));
    if (new_node == NULL)
        return -3;

    ht_node_init(new_node);
    new_node->value = value;
    new_node->key = key;
    
    ht_insert_node(new_node, hash_table->data, hash_table);

    ++hash_table->filled_nodes;

    return 0;
}

void* ht_get_elem(const char* key, hashtable_t* hash_table)
{
    // Validating parameters
    if (key == NULL || hash_table == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    void* value = NULL;
    ht_node_t* last_node = hash_table->data[(*hash_table->hash_func)(key) % hash_table->size];

    // Compare keys to get to required node
    int result = strcmp(last_node->key, key);
    while (result != 0)
    {
        if (last_node->next == NULL)
            break;

        last_node = last_node->next;
            result = strcmp(last_node->key, key);
    }

    // If key was found then set value
    if (result == 0)
        value = last_node->value;

    // If not return NULL
    return value;
}

int ht_remove(const char* key, hashtable_t* hash_table)
{
    // Validating parameters
    if (key == NULL || hash_table == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    size_t hash = (*hash_table->hash_func)(key) % hash_table->size;
    ht_node_t* prev_node = NULL;
    ht_node_t* last_node = hash_table->data[hash];

    // Searching for specified node
    while (strcmp(last_node->key, key) != 0 || last_node->next != NULL)
    {
        prev_node = last_node;
        last_node = last_node->next;
    }

    // If node was first in a list
    if (prev_node == NULL)
    {
        hash_table->data[hash] = last_node->next;
    }
    else
    {
        prev_node->next = last_node->next;
    }

    --hash_table->filled_nodes;

    free(last_node);
    return 0;
}

void ht_destroy(hashtable_t* hash_table)
{
    // Validating parameters
    if (hash_table == NULL)
        return;

    // Iterating through all branches/lists
    for (size_t i = 0; i < hash_table->size; i++)
        if (hash_table->data[i] != NULL)
            ht_node_destroy_from_base(hash_table->data[i]);
    
    // Deleting table
    free(hash_table);
}

void ht_node_init(ht_node_t* node)
{
    node->value = NULL;
    node->key = NULL;
    node->next = NULL;
}

int ht_insert_node(const ht_node_t* node, ht_node_t** nodes_space, hashtable_t* hash_table)
{
    // Getting a hash for specified node
    size_t hash = (*hash_table->hash_func)(node->key) % hash_table->size;

    // If cell is clear then write direcly into a table
    if (nodes_space[hash] == NULL)
    {
        nodes_space[hash] = node;
        return 0;
    }

    // Getting first node in a list
    ht_node_t* last_node = nodes_space[hash];

    // Going to last element in the list
    while (last_node->next != NULL)
        last_node = last_node->next;

    // Adding a new one
    last_node->next = node;
    return 1;
}

void ht_node_destroy_from_base(ht_node_t* base_node)
{
    // Going to last node in list
    if (base_node->next != NULL)
        ht_node_destroy_from_base(base_node->next);

    // Deleting node
    free(base_node);
}

void ht_node_rehash_from_base(ht_node_t* base_node, ht_node_t** nodes_space,hashtable_t* hash_table)
{
    // Going to last node
    if (base_node->next != NULL)
        ht_node_rehash_from_base(base_node->next, nodes_space, hash_table);

    // Inserting node to new node space
    ht_insert_node(base_node, nodes_space, hash_table);
    base_node->next = NULL;
}

size_t fnv_1a(const char* key)
{
    size_t key_length = strlen(key);
    size_t hash = 0;

    for (size_t i = 0; i < key_length; i++)
    {
        hash *= FNV_1A_PRIME_NUM;
        hash ^= key[i];
    }

    return hash;
}
