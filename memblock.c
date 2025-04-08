//
// Created by xpsjpr on 3/25/25.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memblock.h"

#include <sys/mman.h>



mem_block* find(size_t size) {
    if (head == NULL) return NULL;
    mem_block* curr = head;
    mem_block* result = NULL;
    while (curr != NULL) {
        if (curr->size <= size) {
            if (result == NULL) result = curr;
            else {
                if (curr->size > result->size) result = curr;
            }
        }
        curr = curr->next;
    }
    return result;
}



void print_memory() {

    mem_block *curr = head;

    while (curr != NULL) {
        printf("[%zu] <-> ", curr->size);
        curr = curr->next;
    }
    printf("NULL\n");

}

void deleteList(){

    mem_block *curr = head;

    while (curr != NULL) {
        mem_block* temp = curr->next;
        free(curr);
        curr = temp;
    }
}