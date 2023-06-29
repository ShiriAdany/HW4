#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>


typedef struct MallocMetadata {
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
} MallocMetadata;



void *head = nullptr;
size_t numOfFreeBlocks = 0 ;
size_t numOfFreeBytes = 0;
size_t totalBlocksCount = 0;
size_t totalBlocksBytes = 0;



//void* smalloc(size_t size){
//    if (size == 0)
//        return NULL;
//    if (size > 100000000)
//        return NULL;
//
//    //todo what to do if size < 0 ??
//    if (head == nullptr)
//    {
//        head = sbrk(size + sizeof(MallocMetadata));
//        if ( head == (void*)(-1))
//            return NULL;
//        MallocMetadata* metadata = (MallocMetadata*)head;
//        metadata->size = size;
//        metadata->is_free = false;
//        metadata->next = nullptr;
//        metadata->prev = nullptr;
//        return head + sizeof(MallocMetadata);
//    }
//    else
//    {
//        void* temp = head;
//        void* prev = nullptr;
//        bool allocated = false;
//        while (temp)
//        {
//            MallocMetadata* metadata = (MallocMetadata*)temp;
//            if (metadata->size >= size + sizeof(MallocMetadata) && metadata->is_free)
//            {
//                size_t allBlockSize = metadata->size;
//                metadata->size = size;
//                metadata->is_free = false;
//                MallocMetadata* nextBlock = metadata->next;
//                metadata->next = metadata + sizeof(MallocMetadata) + size;
//
//                MallocMetadata* newMetadata = (MallocMetadata*)metadata->next;
//                newMetadata->is_free = true;
//                newMetadata->size = allBlockSize - size - sizeof(MallocMetadata); //100 - 50 - 10
//                newMetadata->next = nextBlock;
//                newMetadata->prev = metadata;
//
////                allocated = true;
//                return metadata + sizeof(MallocMetadata);
////                break;
//            }
//            else
//            {
//                prev = temp;
//                temp = metadata->next;
//            }
//        }
////        if(!allocated)
////        {
//            //sbrk
//            void* ret = sbrk(size + sizeof(MallocMetadata));
//            if ( ret == (void*)(-1))
//                return NULL;
//            MallocMetadata* metadata = (MallocMetadata*)ret;
//            metadata->size = size;
//            metadata->is_free = false;
//            metadata->next = nullptr;
//            metadata->prev = prev;
//            return ret + sizeof(MallocMetadata);
////        }
//
//    }
//}

void* smalloc(size_t size){
    if (size == 0)
        return NULL;
    if (size > 100000000)
        return NULL;

    //todo what to do if size < 0 ??
    if (head == nullptr)
    {
        head = sbrk(size + sizeof(MallocMetadata));
        if ( head == (void*)(-1))
            return NULL;
        totalBlocksCount++;
        totalBlocksBytes += size;
        MallocMetadata* metadata = (MallocMetadata*)head;
        metadata->size = size;
        metadata->is_free = false;
        metadata->next = nullptr;
        metadata->prev = nullptr;
        return (void*)((unsigned long)head + sizeof(MallocMetadata));
    }
    else
    {
        void* temp = head;
        void* prev = nullptr;
        bool allocated = false;
        while (temp)
        {
            MallocMetadata* metadata = (MallocMetadata*)temp;
            if (metadata->size >= size + sizeof(MallocMetadata) && metadata->is_free)
            {
                metadata->is_free = false;
                numOfFreeBlocks--;
                numOfFreeBytes-= metadata->size;
//                allocated = true;
                return metadata + sizeof(MallocMetadata);
            }
            else
            {
                prev = temp;
                temp = metadata->next;
            }
        }
//        if(!allocated)
//        {
        //sbrk
        void* ret = sbrk(size + sizeof(MallocMetadata));
        if ( ret == (void*)(-1))
            return NULL;
        totalBlocksCount++;
        totalBlocksBytes += size;
        MallocMetadata* metadata = (MallocMetadata*)ret;
        metadata->size = size;
        metadata->is_free = false;
        metadata->next = nullptr;
        metadata->prev = (MallocMetadata*)prev;
        ((MallocMetadata*)prev)->next = metadata;

        return (void*)((unsigned long)ret + sizeof(MallocMetadata));
//        }

    }
}

void* scalloc(size_t num, size_t size){
    void * address = smalloc(num* size);
    if (address != NULL)
    {
        memset (address,0,size*num);
    }
    return address;
}


void sfree(void* p){
    if(p == NULL)
        return;

    void* temp = head;
    while (temp)
    {
        MallocMetadata* metadata = (MallocMetadata*) temp;
        if (p == temp + sizeof(metadata))
        {
            if (metadata->is_free)
                return;

//            size_t sizeOfBlock = metadata->size;
            metadata->is_free = true; //if both previous and next block are allocated
            numOfFreeBlocks++;
            numOfFreeBytes+= metadata->size;
//
//            if (metadata->next && metadata->next->is_free)
//            {
//                metadata->size = metadata->size + metadata->next->size + sizeof(MallocMetadata);
//                metadata->next = metadata->next->next;
//            }
//            if (metadata->prev && metadata->prev->is_free)
//            {
//                metadata->prev->size = metadata->prev->size + metadata->size + sizeof(MallocMetadata);
//                metadata->prev->next = metadata->next;
//            }
            return;
        }
        else
        {
            temp = metadata->next;
        }
    }

}

void* srealloc(void* oldp, size_t size){
    if (oldp == NULL)
    {
        return smalloc(size);
    }
    MallocMetadata* oldMeta = (MallocMetadata*)(oldp - sizeof(MallocMetadata));
    if (oldMeta->size >= size)
        return oldp;

    void* newAlloc = smalloc(size);
    if (newAlloc == NULL)
    {
        return NULL;
    }
    memmove(newAlloc,oldp,oldMeta->size);
    sfree(oldp);
    return newAlloc; //we assume numOfFreeBlocks etc are updated because of internal implementation of smalloc and sfree
}

size_t _num_free_blocks(){
    return numOfFreeBlocks;
}

size_t _num_free_bytes(){
    return numOfFreeBytes;
}

size_t _num_allocated_blocks(){
    return totalBlocksCount;
}

size_t _num_allocated_bytes(){
    return totalBlocksBytes;
}

size_t _num_meta_data_bytes(){
    return totalBlocksCount*sizeof(MallocMetadata);
}

size_t _size_meta_data(){
    return sizeof (MallocMetadata);
}