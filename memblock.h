//
// Created by xpsjpr on 3/30/25.
//

#ifndef MEMBLOCK_H
#define MEMBLOCK_H

struct mem_block {
    size_t size;
    struct mem_block* next;
    struct mem_block* prev;
} typedef mem_block;

mem_block* head;
mem_block* tail;

/**
 * Appends mem_block with values alloc_sz, name at tail
 * @param alloc_sz size of allocation for memblock
 * @param name name of memblock
 * @return 0 on success
 */
int append(size_t alloc_sz);

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
void delete(mem_block* block);

/**
 * Prints string representation of mem_block_list to stdin
 */
void print_memory();

/**
 * Frees all mem_blocks
 */
void deleteList();

#endif //MEMBLOCK_H
