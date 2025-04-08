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

// TODO: add static instance variables here. Since you are maintaining a
//       doubly-linked list, you'll want to store the head, tail, etc.
//       You will also want to create functions that modify your linked
//       list so your code is easier to test.

struct mem_block {
    size_t size;
    struct mem_block* next;
    struct mem_block* prev;
    int used;
} typedef mem_block;

mem_block* head = NULL;
mem_block* tail = NULL;
int ALLOC_THRESH = 10;
int list_size = 0;

void append(mem_block* block) {
    block->next = head;
    block->prev = NULL;
    block->used = 0;

    if (head != NULL) head->prev = block;
    head = block;

    if (tail == NULL) tail = block;
    list_size++;
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
    if (tail != NULL) tail->next = NULL;
    else head = NULL;
    list_size--;
    return popped;
}

/**
 * The malloc() function allocates size bytes and returns a pointer to the
 * allocated memory. The memory is not initialized. #Clarify: So used should be 0?
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

    // TODO: scan through your doubly-linked free list, starting at the tail,
    //       and return a viable block if you find one. If no viable blocks are
    //       in the list, you can mmap a new block.
    mem_block* curr = tail;
    while (curr != NULL) { // First we are looking for a big enough block that is free
        if (!curr->used && curr->size >= block_size) { // viable block has been found, mark as used and return block
            curr->used = 1;
            TRACE("Reused block -- [%p]: %zu bytes -- list_size: %d", curr, curr->size, list_size);
            return curr + 1;
        }
        curr = curr->prev;
    }

    // We have gone through our entire list and found nothing
    // mmap: map to memory (do man mmap for info)
    mem_block *block = mmap(
        NULL,                       // We don't care where the memory gets put
        block_size,                          // Size of the block that we want to allocate
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0);

    if (block == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    block->size = block_size;
    append(block);

    block->used = 1; // Clarify: So should I set this to used?

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

    // TODO: find out what the size of the free list should be by checking the
    //       ALLOC_THRESH environment variable (note that getenv returns a
    //       string, not a number). You can store the size so you don't need to
    //       look it up every time.

    // TODO: if there is space in our free list, add the block to the head of
    //       the list instead of unmapping it.

    // TODO: if the list has run out of space, unmap the oldest block in the
    //       list to make space for the block that was just freed. Take note
    //       that the code below is unmapping the block that was just freed, so
    //       you will need to change it.

    if (list_size > ALLOC_THRESH) {
        // List has run out of space, unmap the oldest block
        mem_block* popped = pop();
        if (popped == NULL) return;
        int result = munmap(popped, popped->size);
        if (result == -1) {
            perror("munmap");
        }
        // else {
        //     TRACE("Unmapped block -- [%p]: %zu bytes -- list_size: %d", block, block_size, list_size);
        // }
        // We have enough space, re-append the block to the head of the list as unused so it can be used la
        if (list_size <= ALLOC_THRESH){
            printf("Brace for segfault!!\n");
            block->used = 0;
            append(block); // Re-append the block to the head of the list marking it as unused
            TRACE("Cached free block -- [%p]: %zu bytes -- list_size: %d", block, block_size, list_size);
        }
    }
}

/**
 * The  calloc()  function  allocates  memory  for an array of nmemb elements
 * of size bytes each and returns a pointer to the allocated memory.  The
 * memory is set to zero.
 */
void *calloc(size_t nmemb, size_t size){ // Clarify: I am really confused where this comes into play
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
    // TODO: check if the block can already accommodate the requested size.
    //       if it can, there's no need to do anything
    if (block->size >= size + sizeof(mem_block)) {
        TRACE("Unchanged ---- [%p]: %zu bytes", block, block->size);
        return ptr;
    }

    // TODO: if the block can't accommodate the requested size, then
    //       we should allocate a new block, copy the old data there,
    //       and then free the old block.
    void* new_ptr = malloc(size);
    if (new_ptr == NULL) return NULL;

    memcpy(new_ptr, ptr, block->size - sizeof(mem_block));
    free(ptr);
    return new_ptr;
}


int main(void) {



}
