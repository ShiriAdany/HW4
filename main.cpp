//
// Created by aviaa on 29/06/2023.
//

#include "tests/my_stdlib.h"
#include <stdio.h>
#include <cmath>

int main(int argc, char *const argv[])
{
    void* ptr1 = smalloc(128);
    sfree(ptr1);
    printf("start free bytes %zu\n",_num_free_bytes());
    ptr1 = smalloc(1);
    //REQUIRE(ptr1 != nullptr);
    //verify_block_by_order(1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 31, 0, 0, 0);
    printf("freeeee bytes %zu\n",_num_free_bytes());
    // Reallocate to a larger size
    void* ptr2 = srealloc(ptr1, 2*128);
    //REQUIRE(ptr2 != nullptr);
    //verify_block_by_order(0,0,0,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,31,0,0,0);
   // int* newArr = static_cast<int*>(ptr2);
    printf("freeeee bytes after merge %zu\n",_num_free_bytes());
    sfree(ptr2);



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