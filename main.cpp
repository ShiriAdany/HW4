//
// Created by aviaa on 29/06/2023.
//

#include "tests/my_stdlib.h"
#include <stdio.h>
#include <cmath>
#include <cassert>
#include <iostream>
#include <vector>

#include <fstream>
#include <unistd.h>

#define REQUIRE(thing) assert(thing)

#define MAX_ALLOCATION_SIZE (1e8)
#define MMAP_THRESHOLD (128 * 1024)
#define DEFAULT_MMAP_THRESHOLD_MAX (4 * 1024 * 1024 * sizeof(long))
#define SMALLOC_HUGE_PAGE_THRESHOLD (1000 * 1000 * 4)
#define SCALLOC_HUGE_PAGE_THRESHOLD (1000 * 1000 * 2)

static inline size_t aligned_size(size_t size)
{
    return (size % 8) ? (size & (size_t)(-8)) + 8 : size;
}

#define verify_blocks(allocated_blocks, allocated_bytes, free_blocks, free_bytes)                                      \
    do                                                                                                                 \
    {                                                                                                                  \
        printf("%d\n",_num_allocated_blocks());                                                                                                               \
        REQUIRE(_num_allocated_blocks() == allocated_blocks);                                                          \
        REQUIRE(_num_allocated_bytes() == aligned_size(allocated_bytes));                                              \
        REQUIRE(_num_free_blocks() == free_blocks);                                                                    \
        REQUIRE(_num_free_bytes() == aligned_size(free_bytes));                                                        \
        REQUIRE(_num_meta_data_bytes() == aligned_size(_size_meta_data() * allocated_blocks));                         \
    } while (0)

#define verify_size(base)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        void *after = sbrk(0);                                                                                         \
        REQUIRE(_num_allocated_bytes() + aligned_size(_size_meta_data() * _num_allocated_blocks()) ==                  \
                (size_t)after - (size_t)base);                                                                         \
    } while (0)

#define verify_size_with_large_blocks(base, diff)                                                                      \
    do                                                                                                                 \
    {                                                                                                                  \
        void *after = sbrk(0);                                                                                         \
        REQUIRE(diff == (size_t)after - (size_t)base);                                                                 \
    } while (0)

long long get_huge_pages_amount()
{
    std::ifstream meminfo("/proc/meminfo");
    REQUIRE(meminfo.is_open());
    std::string line;
    long long total = -1;
    long long free = -1;
    while (getline(meminfo, line))
    {
        if (line.find("HugePages_Total") != std::string::npos)
        {
            std::string sub = line.substr(line.find(":") + 1);
            total = std::atoll(sub.c_str());
        }
        if (line.find("HugePages_Free") != std::string::npos)
        {
            std::string sub = line.substr(line.find(":") + 1);
            free = std::atoll(sub.c_str());
        }
    }
    REQUIRE(total != -1);
    REQUIRE(free != -1);
    return total - free;
}

#define validate_huge_pages_amount(base, amount)                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        REQUIRE(base + amount == get_huge_pages_amount());                                                             \
    } while (0)



#define MAX_ELEMENT_SIZE (128*1024)

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
    verify_blocks(0, 0, 0, 0);
    void *base = sbrk(0);
    char *a = (char *)smalloc(MMAP_THRESHOLD + 8);
    REQUIRE(a != nullptr);
    //verify_blocks(1, MMAP_THRESHOLD + 8, 0, 0);
    //verify_size_with_large_blocks(base, 0);

    sfree(a);
    //verify_blocks(0, 0, 0, 0);
    //verify_size(base);

    char *b = (char *)smalloc(MMAP_THRESHOLD);
    REQUIRE(b != nullptr);
    //verify_blocks(1, MMAP_THRESHOLD, 0, 0);
    //verify_size(base);

    sfree(b);
    //verify_blocks(1, MMAP_THRESHOLD, 1, MMAP_THRESHOLD);
    //verify_size(base);
    return 0;
}