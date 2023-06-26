#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

void* smalloc(size_t size){
    if (size == 0)
        return NULL;
    if (size > 100000000)
        return NULL;

    //todo what to do if size < 0 ??

    void *ret = sbrk(size);
    if ( ret == (void*)(-1))
        return NULL;
    return ret;
}



