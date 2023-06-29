//
// Created by aviaa on 29/06/2023.
//

#include "tests/my_stdlib.h"
#include <stdio.h>

int main(int argc, char *const argv[])
{
    printf("%zu\n",_size_meta_data());

    printf("%zu\n",_num_allocated_blocks());
    printf("%zu\n",_num_free_blocks());
    int* a = (int*) smalloc(10);
    printf("%zu\n",_num_allocated_blocks());
    printf("%zu\n",_num_free_blocks());

    printf("%lu\n",(unsigned long)a);
    sfree(a);
    printf("%zu\n",_num_allocated_blocks());

    int *b = (int*) smalloc(10);
    printf("%lu\n",(unsigned long)b);
    printf("%zu\n",_num_allocated_blocks());
    printf("%lu\n",(unsigned long)b - (unsigned long)a);

    return 0;
}