//
// Created by xpsjpr on 3/25/25.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memblock.h"

mem_block* head = NULL;
mem_block* tail = NULL;

int append(size_t alloc_sz) {

    // Malloc and configure new mem_block
    mem_block *new = (mem_block*) malloc(sizeof(mem_block));
    new->size = alloc_sz;
    new->next = NULL;
    new->prev = NULL;

    if (head == NULL) {
        head = new;
        tail = new;
    } else {
        head->prev = new;
        new->next = head;
        head = new;
    }
    return 0;

}

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

void delete(mem_block* block){

    if (head == NULL) return;
    if (head == block) {
        mem_block* temp = head;
        head = head->next;
        free(temp);
        return;
    }
    if (tail == block) {
        mem_block* temp = tail;
        tail = tail->prev;
        free(temp);
        return;
    }

    mem_block* curr = head;
    if (curr->next == NULL) return;
    while (curr->next->next != NULL) {
        if (curr->next == block) {
            mem_block* temp = curr->next;
            curr->next = curr->next->next;
            free(temp);
            return;
        }
        curr = curr->next;
    }
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

int main(void) {

    append(100);
    append(500);
    append(200);
    mem_block* fivehunnit = find(500);
    print_memory();
    delete(fivehunnit);
    append(300);
    print_memory();

    return 0;
}