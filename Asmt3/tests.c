#include "virtual_alloc.h"
#include "virtual_sbrk.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define HEAP_MAX (1 << 20)
unsigned char global_memory[HEAP_MAX];
void* heapstart = global_memory;
void* program_brk = &global_memory[1 << 18]; //2^18

void * virtual_sbrk(int32_t increment) {
    // Your implementation here (for your testing only)
    if ((program_brk + increment) > (void*)(global_memory + HEAP_MAX)) {
        return (void *)(-1);
    }
    program_brk += increment;
    return program_brk;
}

void setup() {
    program_brk = &global_memory[1 << 18];
}

void check_init_value(struct Record* init) {
    assert(init->max_exponent == 15);
    assert(init->min_exponent == 12);
    assert(init->max_size == (1 << 15));
    assert(init->cur_node == 0);
}
void test_init_allocator() {
    printf("+---------------------------+\n");
    printf("Begin testing init_allocator:\n");
    program_brk = &global_memory[1 << 13];
    init_allocator(heapstart, 15, 12);
    struct Record* init = (struct Record*)heapstart;

    check_init_value(init);
    assert(init->node_list[0].size == 0);
    assert(init->node_list[0].exponent == 0);
    assert(init->node_list[0].allocated_status == false);
    assert(init->node_list[0].address == NULL);
    
    printf("Pass init_allocator!\n");
    printf("+---------------------------+\n\n");
}

void test_virtual_malloc() {
    setup();
    printf("+---------------------------+\n");
    printf("Begin testing virtual_malloc:\n");
    init_allocator(heapstart, 15, 12);
    struct Record* init = (struct Record*)heapstart;

    check_init_value(init);

    void* address_0 = virtual_malloc(heapstart, 0);
    assert(address_0 == NULL);
    assert(init->node_list[0].allocated_status == false);

    void* address_1 = virtual_malloc(heapstart, 8000);
    assert(init->node_list[0].allocated_status == true);
    assert(init->node_list[0].size == (1 << 13));
    
    void* address_2 = virtual_malloc(heapstart, 11000);
    assert((address_2 - address_1)  == (1 << 13));
    assert(init->node_list[1].allocated_status == true);
    assert(init->node_list[1].size == (1 << 14));

    void* address_3 = virtual_malloc(heapstart, 300);
    assert((address_1 - address_3) == (1 << 12));
    assert((address_2 - address_3) == ((1 << 14)-(1<<12)));
    assert(init->node_list[2].allocated_status == true);
    assert(init->node_list[2].size == (1 << 12));

    void* address_4 = virtual_malloc(heapstart, 60000);
    assert(address_4 == NULL);
    assert(init->node_list[3].allocated_status == false);

    printf("Pass virtual_malloc!\n");
    printf("+---------------------------+\n\n");
}

void test_virtual_free() {
    setup();
    printf("+---------------------------+\n");
    printf("Begin testing virtual_free:\n");
    init_allocator(heapstart, 15, 12);
    struct Record* init = (struct Record*)heapstart;

    check_init_value(init);

    void* address_0 = virtual_malloc(heapstart, 8000);
    void* address_1 = virtual_malloc(heapstart, 0);
    void* address_2 = virtual_malloc(heapstart, 2000);
    //printf("%p", address_1);
    int free_0 = virtual_free(heapstart, address_0);
    int free_1 = virtual_free(heapstart, address_1);
    int free_2 = virtual_free(heapstart, address_2);
    int free_3 = virtual_free(heapstart, NULL);
    int free_4 = virtual_free(NULL, NULL);
    assert(free_0 == 0);
    assert(free_1 == -1);
    assert(free_2 == 0);
    assert(free_3 == -1);
    assert(free_4 == -1);
    printf("Pass virtual_free!\n");
    printf("+---------------------------+\n\n");
}

void void_virtual_realloc() {
    setup();
    printf("+---------------------------+\n");
    printf("Begin testing virtual_realloc:\n");
    init_allocator(heapstart, 15, 12);
    struct Record* init = (struct Record*)heapstart;

    check_init_value(init);

    void* address = virtual_malloc(heapstart, 8000);
    void* new_address_0 = virtual_realloc(heapstart, address, 10000);

    void* address_4000 = virtual_malloc(heapstart, 4000); 
    void* new_address_1 = virtual_realloc(heapstart, address, 4000); //truncate below
    
    
    void* address_12000 = virtual_malloc(heapstart, 15000);
    void* new_address_2 = virtual_realloc(heapstart, NULL, 15000); 
    
    int malloc_4000 = 0;
    int address_after_truncate = 0;
    for (int i = 0; i < init->cur_node; i++) {
        if (strcmp(new_address_1, init->node_list[i].address) == 0) {
            address_after_truncate = init->node_list[i].size;
        }
        if (strcmp(address_4000, init->node_list[i].address) == 0) {
            malloc_4000 = init->node_list[i].size;
        }
    }
    
    //void* address = virtual_malloc(heapstart, 8000);
    assert(new_address_0 == NULL);
    assert(new_address_1 == address);
    assert(address_after_truncate == malloc_4000);
    assert(address_12000 != new_address_2);

    printf("Pass virtual_realloc!\n");
    printf("+---------------------------+\n\n");
}

int main() {
    // Your own testing code here
    test_init_allocator(); 
    test_virtual_malloc();
    test_virtual_free();
    void_virtual_realloc();

    return 0;
}
