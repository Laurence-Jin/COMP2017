#include "virtual_alloc.h"
#include "virtual_sbrk.h"

void init_allocator(void * heapstart, uint8_t initial_size, uint8_t min_size) {
    int total_size = (1 << (int)initial_size);  
    struct Node init = {NULL, .allocated_status = false, .size = 0, .exponent = 0};
    struct Record record = {.max_size = total_size, .max_exponent = (int)initial_size, .min_exponent = (int)min_size, .cur_node = 0, .node_list[0] = init};
    memcpy(heapstart, &record, sizeof(struct Record));  
    virtual_sbrk(total_size);
}
// find the pow with 2
int find_the_power_num (int max_exponent, int min_exponent, int size) {
    int ret_exponent = 0;
    for (int i = max_exponent; i > 0; i--) {
        if (i == min_exponent) {
            ret_exponent = i;
            break;
        }
        int num = (1 << i);
        int num_next = (1 << (i-1));
        if (num == size) {
            ret_exponent = i;
            break;
        }
        if (size < num && size > num_next) {
            ret_exponent = i;
            break; 
        }
    }
    return ret_exponent;
}

void * virtual_malloc(void * heapstart, uint32_t size) {
    struct Record* init = (struct Record*)heapstart;
    int min_exponent = init->min_exponent;
    int max_exponent = init->max_exponent;
    int max_size = init->max_size;
    int max_exp_in_list = 0;
    int count = 0;
    if (size > max_size || size == 0) {
        return NULL;
    }
    int sum = 0;
    for (int i = 0; i < init->cur_node; i++) {
        sum += init->node_list[i].size;
        if ((sum + size) > init->max_size) {
            return NULL;
        }
    }

    int this_exponent = find_the_power_num(max_exponent, min_exponent, size);
    int this_size = (1 << this_exponent); //the current address
    int ret_size = this_size; //return the num of address
    //find if this block has been occpuied
    for (int i = 0; i < init->cur_node; i++) {
        if (this_size == init->node_list[i].size) {
            count++;
        }
        if (this_size < init->node_list[i].size) {
            max_exp_in_list = init->node_list[i].exponent;
        }
    }
    if (count == 1 && max_exp_in_list > this_exponent) {
        ret_size = (1 << (max_exp_in_list+1)); //next one
    }
    struct Node node = {.address = heapstart+ret_size, .allocated_status = true, .size = this_size, .exponent = this_exponent};
    init->node_list[init->cur_node] = node;
    init->cur_node++;
    virtual_sbrk(max_size);
    return heapstart+ret_size; //return the head
}

int virtual_free(void * heapstart, void * ptr) {
    if (heapstart == NULL || ptr == NULL) {
        return -1;
    }
    struct Record* init = (struct Record*)heapstart;
    for (int i = 0; i < init->cur_node; i++) {
        if (init->node_list[i].address == NULL && init->node_list[i+1].address != NULL) {
            i++;
        }
        if (init->node_list[i].address == NULL && init->node_list[i+1].address == NULL) {
            break;
        }
        if (strcmp(init->node_list[i].address, ptr) == 0) {
            struct Node node = {.address = NULL, .allocated_status = false, .size = 0, .exponent = 0};
            init->node_list[i] = node;
            init->cur_node--;
            return 0;
        }
    }
    return -1;
}

void * virtual_realloc(void * heapstart, void * ptr, uint32_t size) { 
    struct Record* heap_info = (struct Record*)heapstart;
    if (ptr == NULL) {
        if (size == 0) {
            return NULL;
        }
        virtual_malloc(heapstart, size);
        return ptr;
    }
    for (int i = 0; i < heap_info->cur_node; i++) {
        struct Node node = heap_info->node_list[i];
        if (node.address == NULL) {
            break;
        }
        if (strcmp (node.address, ptr) == 0) {
            if (size > node.size) {
                return NULL; 
            }
            if (size < node.size) { 
                //truncate   
                int this_exponent = find_the_power_num(heap_info->max_exponent, heap_info->min_exponent, size);
                heap_info->node_list[i].size = (1 << this_exponent);
                heap_info->node_list[i].exponent = (int)(log(size) / log(2));
                return ptr;
            }
        }
    }
    //not found the same address in node list
    return ptr;
}

void virtual_info(void * heapstart) {
    struct Record* init = (struct Record*)heapstart;
    int total_node = init->cur_node;
    if (total_node == 0) {
        printf("free %d\n", init->max_size);
        return;
    }
    //descending store the val in node_list
    for (int i = 0; i < total_node; i++) {
        for (int j = i+1; j < total_node; j++) {
            if (init->node_list[i].size < init->node_list[j].size) {
                struct Node temp = init->node_list[i];
                init->node_list[i] = init->node_list[j];
                init->node_list[j] = temp;
            }
        }
    }
    int this_exponent = init->node_list[0].exponent; //the highest num in a list 
    if (this_exponent == 0) {
        return;
    }
    int count_size = 0;
    for (int i = total_node - 1; i >= 0; i--) {
        if (init->node_list[i].allocated_status) {
            printf("allocated %d\n", init->node_list[i].size);
            count_size+=init->node_list[i].size;
        }
    }
    int i = 0;
    int max_exponent = init->max_exponent;
    int diff = max_exponent - this_exponent; 
    //print the rest of free block
    while (diff > 0) {
        int free_size = (1 << (this_exponent+i));
        if (count_size <= free_size) {
            printf("free %d\n", free_size);
        }
        diff--;
        i++;
    }
}