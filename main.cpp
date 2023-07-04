//
// Created by aviaa on 29/06/2023.
//

#include "tests/my_stdlib.h"
#include <stdio.h>
#include <cmath>
#include <cassert>
#include <iostream>
#include <vector>

#define REQUIRE(thing) assert(thing)

#define MAX_ELEMENT_SIZE (128*1024)

#define verify_blocks(allocated_blocks, allocated_bytes, free_blocks, free_bytes)                                      \
    do                                                                                                                 \
    {                                                                                                                  \
        assert(_num_allocated_blocks() == allocated_blocks);                                                          \
        assert(_num_allocated_bytes() == (allocated_bytes));                                              \
        assert(_num_free_blocks() == free_blocks);                                                                    \
        assert(_num_free_bytes() == (free_bytes));                                                                     \
        assert(_num_meta_data_bytes() == (_size_meta_data() * allocated_blocks));                         \
    } while (0)

void verify_block_by_order(int order0free, int order0used, int order1free, int order1used, \
                                int order2free, int order2used,\
                                int order3free, int order3used, \
                                int order4free, int order4used, \
                                int order5free, int order5used, \
                                int order6free, int order6used, \
                                int order7free, int order7used, \
                                int order8free,int  order8used, \
                                int order9free,int  order9used, \
                                int order10free,int  order10used,
                           int big_blocks_count, long big_blocks_size  )\
                                                                                                                     \
    {                                                                                                                  \
        unsigned int __total_blocks = order0free + order0used+ order1free + order1used+ order2free + order2used+ order3free + order3used+ order4free + order4used+ order5free + order5used+ order6free + order6used+ order7free + order7used+ order8free + order8used+ order9free + order9used+ order10free + order10used + big_blocks_count       ;        \
        unsigned int __total_free_blocks = order0free+ order1free+ order2free+ order3free+ order4free+ order5free+ order6free+ order7free+ order8free+ order9free+ order10free ;                     \
        unsigned int __total_free_bytes_with_meta  = order0free*128*pow(2,0) +  order1free*128*pow(2,1) +  order2free*128*pow(2,2) +  order3free*128*pow(2,3) +  order4free*128*pow(2,4) +  order5free*128*pow(2,5) +  order6free*128*pow(2,6) +  order7free*128*pow(2,7) +  order8free*128*pow(2,8) +  order9free*128*pow(2,9)+  order10free*128*pow(2,10) ;                                                                     \
        unsigned int testing_allocated_bytes;


    if (__total_blocks==0) testing_allocated_bytes = 0;
    else testing_allocated_bytes = big_blocks_size+32 * MAX_ELEMENT_SIZE - (__total_blocks-big_blocks_count)*(_size_meta_data());
    printf("\n\n%zu\n",_num_allocated_blocks());                                                                                                             \
    printf("%lu\n",__total_blocks);                                                                                                               \
    verify_blocks(__total_blocks, testing_allocated_bytes, __total_free_blocks,__total_free_bytes_with_meta - __total_free_blocks*(_size_meta_data()));\
    }

int main(int argc, char *const argv[])
{
    //size of M is 40 bytes
    //void* ptr1 = smalloc(0);
    size_t n = _num_free_bytes();
    n += 40 * 32; //is exactly 2 ^ 22
    printf("start\n\tfree bytes %zu\n",_num_free_bytes());
    printf("\tfree blocks %zu\n", _num_free_blocks());
    printf("\taloc blocks %zu\n",_num_allocated_blocks());
    printf("\taloc bytes %zu\n",_num_allocated_bytes());
//    return 0;
    assert(_num_meta_data_bytes() == _num_allocated_blocks() * _size_meta_data()); //this should always hold
    void* ptr1 = smalloc(40);
    REQUIRE(ptr1 != nullptr);
    verify_block_by_order(1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 31, 0, 0, 0);

    // Reallocate to a larger size
    void* ptr2 = srealloc(ptr1, 128*pow(2,2) -64);
    REQUIRE(ptr2 != nullptr);
    verify_block_by_order(0,0,0,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,31,0,0,0);
    int* newArr = static_cast<int*>(ptr2);

    // Verify  elements are copied
    for (int i = 0; i < 10; i++) {
        newArr[i] = i + 1;
    }

    // Reallocate to a larger size
    void* ptr3 = srealloc(ptr2, 100);
    REQUIRE(ptr3 != nullptr);
    REQUIRE(ptr2 == ptr3);
    verify_block_by_order(0,0,0,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,31,0,0,0);


    void* ptr4 = srealloc(ptr3, 128*pow(2,8) -64);
    verify_block_by_order(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,31,0,0,0);
    int* newArr2 = static_cast<int*>(ptr4);
    for (int i = 0; i < 10; i++) {
        REQUIRE(newArr2[i] == i + 1);
    }
    sfree(ptr4);
    verify_block_by_order(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0);
    return 0;

    printf("after allocation\n\tfree bytes %zu\n",_num_free_bytes());
    printf("\tfree blocks %zu\n", _num_free_blocks());
    printf("\taloc blocks %zu\n",_num_allocated_blocks());
    printf("\taloc bytes %zu\n",_num_allocated_bytes());
    printf("----------\n");

    ptr1 = srealloc(ptr1,200);

    printf("after realloc\n\tfree bytes %zu\n",_num_free_bytes());
    printf("\tfree blocks %zu\n", _num_free_blocks());
    printf("\taloc blocks %zu\n",_num_allocated_blocks());
    printf("\taloc bytes %zu\n",_num_allocated_bytes());
    printf("----------\n");

//    return 1;
    sfree(ptr1);
    printf("after free\n\tfree bytes %zu\n",_num_free_bytes());
    printf("\tfree blocks %zu\n", _num_free_blocks());
    printf("\taloc blocks %zu\n",_num_allocated_blocks());
    printf("\taloc bytes %zu\n",_num_allocated_bytes());
    // Reallocate to a larger size

    ptr2 = srealloc(ptr1, 20);
    printf("freeeee bytes after merge %zu\n",_num_free_bytes());

    sfree(ptr2);
    printf("freeeee bytes after merge %zu\n",_num_free_bytes());




    // Allocate another small block
//    void *ptr3 = smalloc(50);
//    REQUIRE(ptr3 != nullptr);
//    verify_block_by_order(0,2,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,31,0,1,MAX_ELEMENT_SIZE+100);
//
//    // Free the first small block
//    sfree(ptr1);
//    verify_block_by_order(1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,31,0,1,MAX_ELEMENT_SIZE+100);
//
//
//    // Allocate another small block
//    void *ptr4 = smalloc(40);
//    REQUIRE(ptr4 != nullptr);
//    verify_block_by_order(0,2,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,31,0,1,MAX_ELEMENT_SIZE+100);
//
//    // Free all blocks
//    sfree(ptr3);
//    sfree(ptr4);
//    verify_block_by_order(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,1,MAX_ELEMENT_SIZE+100);
//    sfree(ptr1); //free again
//    sfree(ptr2);
//    verify_block_by_order(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0);

    return 0;
}