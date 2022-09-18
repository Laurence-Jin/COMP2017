#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

struct Node {
    void* address; //the block start address
    bool allocated_status; //true -> allocated, false -> free
    int size; //the actual size of it
    int exponent;
};

struct Record {
    int max_size;
    int max_exponent;
    int min_exponent;
    int cur_node; //the current node stored in list
    struct Node node_list[100];
};

void init_allocator(void * heapstart, uint8_t initial_size, uint8_t min_size);

void * virtual_malloc(void * heapstart, uint32_t size);

int virtual_free(void * heapstart, void * ptr);

void * virtual_realloc(void * heapstart, void * ptr, uint32_t size);

void virtual_info(void * heapstart);
