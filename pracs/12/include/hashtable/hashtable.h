/*
    Copyright (C) 2023 Nikita Retr0-code Korneev

    c_hash_table is free software: you can redistribute, modify it
    under the terms of the GNU Affero General Public License version 3.

    You should have received a copy of GNU Affero General Public License
    version 3 along with hash_dumper.
    If not, see <https://www.gnu.org/licenses/agpl-3.0.html>.


----

    This header describes an API for hashtable data structure and function that allows full control over it
*/

#ifndef __HASH_TABLE_H
#define __HASH_TABLE_H

#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#define FNV_1A_PRIME_NUM    246817537

typedef size_t (*hash_func_t)(const char*);

typedef struct ht_node_t
{
    void* value;
    const char* key;
    struct ht_node_t* next;
} ht_node_t;

typedef struct hashtable_t
{
    size_t size;
    size_t filled_nodes;
    hash_func_t hash_func;
    struct ht_node_t** data;
} hashtable_t;

// Initializes hash table of specific size with specific hash function
int ht_init(size_t size, hash_func_t ht_func_ptr, hashtable_t* hash_table);

// Resizes hash_table_t::data to new_size
int ht_resize(size_t new_size, hashtable_t* hash_table);

// Adds new element to hashtable
int ht_emplace(const char* key, void* value, hashtable_t* hash_table);

// Returns value of element by key
void* ht_get_elem(const char* key, hashtable_t* hash_table);

// Removes element from hashtable
int ht_remove(const char* key, hashtable_t* hash_table);

// Properly frees memory of hashtable
void ht_destroy(hashtable_t* hash_table);

// Initializes node with NULLs
void ht_node_init(ht_node_t* node);

// Returns zero if added directly to a table and 1 if added to linked list
int ht_insert_node(const ht_node_t* node, ht_node_t** nodes_space, hashtable_t* hash_table);

// Destroys all nodes including base node
void ht_node_destroy_from_base(ht_node_t* base_node);

// Rehashes all nodes in given linked list to new space
void ht_node_rehash_from_base(ht_node_t* base_node, ht_node_t** nodes_space, hashtable_t* hash_table);

// Default hashing function
size_t fnv_1a(const char* key);

#endif
