// NOTE: Due date will be Mon 7 Apr

/**
 * @file allocator.c
 * @author John Riordan
 *
 * Implements a C memory allocator with a FIFO free list cache. Users can set
 * ALLOC_THRESH to determine how many free blocks are cached in the free list.
 */

#include <stddef.h>
#include <string.h>
#include <sys/mman.h>
#include <stdlib.h>

#include "logger.h"
#include "trace.h"

struct mem_block {
    size_t size;
    struct mem_block* next;
    struct mem_block* prev;
    int used;
} typedef mem_block;

int ALLOC_THRESH = 10;
mem_block* head = NULL;
mem_block* tail = NULL;
int list_size = 0;

void append(mem_block* block) {
    block->next = head;
    block->prev = NULL;

    if (head != NULL) head->prev = block;
    head = block;

    if (tail == NULL) tail = block;
    list_size++;
}

void remove_from_list(mem_block* block){
    if (head == NULL || tail == NULL) return; // There's nothing in the list
    if (block == head && block == tail) { // There's only one block left
        head = NULL;
        tail = NULL;
    } else if (block == head) { // head != tail, block being removed is the head
        head = head->next;
        head->prev = NULL;
    } else if (block == tail) { // head != tail, block being removed is the tail
        tail = tail->prev;
        tail->next = NULL;
    } else { // block being removed is neither the head nor tail, somewhere in between
        block->prev->next = block->next;
        block->next->prev = block->prev;
    }
}

/**
 * Removes the tail mem_block, decrements the list_size and returns mem_block for munmap or reappending
 * (Depends on the list_size relative to the ALLOC_THRESH)
 * @return old mem_block
 */
mem_block* pop() {
    if (tail == NULL) return NULL;
    mem_block* popped = tail;
    tail = popped->prev;
    popped->prev = NULL;
    popped->next = NULL;
    if (tail != NULL) tail->next = NULL;
    else head = NULL;
    list_size--;
    return popped;
}

/**
 * The malloc() function allocates size bytes and returns a pointer to the
 * allocated memory. The memory is not initialized.
 * @param size size of memory block to be allocated
 * @return pointer to allocated memory
 */
void *malloc(size_t size)
{
    if (head == NULL) {
        char* thresh = getenv("ALLOC_THRESH");
        if (thresh != NULL) {
            ALLOC_THRESH = atoi(thresh);
        }
    }

    size_t block_size = size + sizeof(mem_block);
    LOG("malloc request. size: %zu, block size: %zu\n", size, block_size);

    mem_block* curr = tail;
    while (curr != NULL) { // First we are looking for a big enough block that is free
        if (curr->used && curr->size >= block_size) { // per email, remove block if its reused in malloc()
            remove_from_list(curr);
            TRACE("Reused block -- [%p]: %zu bytes -- list_size: %d", curr, curr->size, list_size);
            list_size--;
            return curr + 1;
        }
        curr = curr->prev;
    }

    // We have gone through our entire list and found nothing, so we allocate (w/ mmap) a new mem_block to memory
    mem_block *block = mmap(NULL,block_size,PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS,-1,0);

    if (block == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    // Configure the mem_block
    block->size = block_size;
    block->used = 1;
    TRACE("Allocated block [%p]: %zu bytes -- list_size: %d", block, block_size, list_size);
    return block + 1;
}

/**
 * The free() function frees the memory space pointed to by ptr, which must
 * have been returned by a previous call to malloc() or related functions.
 * Otherwise, or if ptr has already been freed, undefined behavior occurs.
 * If ptr is NULL, no operation is performed.
 */
void free(void *ptr) {
    if (ptr == NULL) return; /* Freeing a NULL pointer does nothing. */

    mem_block *block = (mem_block *) ptr - 1;
    LOG("free request. ptr: %p, size: %zu\n", block, block->size);
    size_t block_size = block->size;
    block->next = NULL;
    block->prev = NULL;
    append(block); // Add the block to the head of our list, if out of space, we pop and unmap, else it stays cached

    if (list_size > ALLOC_THRESH) {
        // List has run out of space, unmap the oldest block (the tail of our mem_block list)
        mem_block* popped = pop();
        if (popped == NULL) return;
        int result = munmap(popped, popped->size);
        if (result == -1) {
            perror("munmap");
        } else {
            TRACE("Unmapped block -- [%p]: %zu bytes -- list_size: %d", block, block_size, list_size);
            return;
        }
    }
    // If it is within the alloc_threshold it's cached into our list
    TRACE("Cached free block -- [%p]: %zu bytes -- list_size: %d", block, block_size, list_size);

}

/**
 * The  calloc()  function  allocates  memory  for an array of nmemb elements
 * of size bytes each and returns a pointer to the allocated memory.  The
 * memory is set to zero.
 */
void *calloc(size_t nmemb, size_t size){
    LOG("calloc request. size: %zu memb, %zu bytes each\n", nmemb, size);
    void *ptr = malloc(nmemb * size);
    memset(ptr, 0, nmemb * size);
    return ptr;
}

/**
 * The realloc() function changes the size of the memory block pointed to by
 * ptr to size bytes.  The contents of the memory will be unchanged in the
 * range from the start of the region up to the minimum of the old and new
 * sizes.  If the new size is larger than the old size, the added memory will
 * not be initialized.
 */
void *realloc(void *ptr, size_t size){
    if (ptr == NULL) return malloc(size);

    mem_block* block = ((mem_block*)ptr) - 1;
    LOG("realloc request. ptr: %p, new size: %zu\n", ptr, size);
    if (block->size >= size + sizeof(mem_block)) { // block can fit the requested size, return the same ptr
        TRACE("Unchanged ---- [%p]: %zu bytes", block, block->size);
        return ptr;
    }

    // block can't accomodate the requested size
    void* new_ptr = malloc(size); // allocate new block with the new size
    if (new_ptr == NULL) return NULL;
    memcpy(new_ptr, ptr, block->size - sizeof(mem_block)); // copy the stuff over from the previous block
    free(ptr); // call our free to free the old mem_block at the ptr
    TRACE("Resized block [%p]: %zu bytes -> %zu bytes", block, block->size, size);
    return new_ptr; // return the new address
}