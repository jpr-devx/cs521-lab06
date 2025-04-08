//
// Created by xpsjpr on 3/30/25.
//

#ifndef MEMBLOCK_H
#define MEMBLOCK_H

struct mem_block {
    size_t size;
    struct mem_block* next;
    struct mem_block* prev;
    int used;
} typedef mem_block;

mem_block* head;
mem_block* tail;
int list_size;
/**
 * Appends mem_block with values alloc_sz, name at tail
 * @param block
 */
void append(mem_block* block);

/**
 * Scans through linked list for mem_blocks whose size is less than or equal to size
 * @param size size of allocation to be searched for
 * @return pointer to first matching mem_block, NULL if not found
 */
mem_block* find(size_t size);

/**
 * Deletes first memblock with value name
 * @param name name of memblock
 */
void remove_from_list(mem_block* block);

/**
 * Prints string representation of mem_block_list to stdin
 */
void print_memory();

/**
 * Frees all mem_blocks
 */
void deleteList();

#endif //MEMBLOCK_H
