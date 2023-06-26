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

//void *head = nullptr;
size_t numOfFreeBlocks = 0 ;
size_t numOfFreeBytes = 0;
size_t totalBlocksCount = 0;
size_t totalBlocksBytes = 0;

const int MAX_ORDER = 10;
const size_t TOTAL_BLOCKS_SIZE = 32 * 128 * 1024;

MallocMetadata* arr[MAX_ORDER+1];
//arr[10] = sbrk

void _initiateBlocks() {
    void* currentProgramBreak = sbrk(0);
    if (currentProgramBreak == (void*)(-1))
        return NULL;

    size_t offset = TOTAL_BLOCKS_SIZE - currentProgramBreak % TOTAL_BLOCKS_SIZE;
    void* allocAddress = sbrk(offset + TOTAL_BLOCKS_SIZE);

    if (allocAddress == (void*)(-1))
        return NULL;
    allocAddress = allocAddress + offset; //now it's aligned - one block
    void* temp = allocAddress;
    void* prev;
    for(int i = 0; i < 32 ; i++)
    {
        MallocMetadata* metadata = (MallocMetadata*)temp;
        metadata->size = 128 * 1024;
        metadata->is_free = true;
        if (i == 0)
        {
            arr[MAX_ORDER] = metadata;
            metadata->prev = nullptr;
        }
        else
        {
            metadata->prev = prev;
            metadata->prev->next = metadata;
        }
        prev = temp;
        temp = temp + metadata->size;
    }
}


void* smalloc(size_t size){
    //TODO check if need to first time allocate or return null for invalid sizes
    if (size == 0)
        return NULL;
    if (size > 100000000)
        return NULL;

    //todo what to do if size < 0 ??
    if (totalBlocksCount == 0) //initiate array
    {
        _initiateBlocks();
        totalBlocksCount = 32;
    }

    size_t sizeOfBlockInOrder = 1;
    for (int i = 0 ; i < MAX_ORDER + 1; i++ )
    {

        if (sizeOfBlockInOrder*128 >= size + sizeof(MallocMetadata))
        {
            //check if exist
            if (!arr[i])
            {
                int j = i;
                while (j < MAX_ORDER + 1 && !arr[j])
                    j++;
                if (j == MAX_ORDER + 1) {
                    //TODO size is valid but block big enough not found
                }

                else
                {
                    while (j > i)
                    {
                        void *addressOfBig = arr[j];
                        MallocMetadata *metadata = (MallocMetadata *) addressOfBig; //meta of the big
                        //remove from layer
                        arr[j] = arr[j]->next;
                        arr[j]->prev = nullptr;

                        //split to 2 blocks
                        void *addressOfBuddy = addressOfBig + metadata->size / 2;
                        MallocMetadata *newBlock = (MallocMetadata *) addressOfBuddy; //meta of the big
                        metadata->size = metadata->size / 2;
                        newBlock->size = metadata->size;
                        arr[j - 1] = metadata;
                        metadata->next = newBlock;
                        metadata->prev = nullptr;
                        metadata->is_free = true;
                        newBlock->prev = metadata;
                        newBlock->next = nullptr;
                        newBlock->is_free = true;
                        j--;
                    }
                }
            }
            void* address = arr[i]
            if(arr[i]->next)
                arr[i]->next->prev = nullptr;
            arr[i] = arr[i]->next;
            ((MallocMetadata*)address)->is_free = false; //meta of the big
            return address + sizeof(MallocMetadata);
        }
        sizeOfBlockInOrder *= 2;
    }
    //if reached here than size is bigger than any possible block
    return NULL;
}

void* scalloc(size_t num, size_t size){
    void * address = smalloc(num* size);
    if (address != NULL)
    {
        memset(address,0,size*num);
    }
    return address;
}


void sfree(void* p){
    //לבדוק אם הם חברים באמצעות xor ואז אם כן אז נעשה מיזוג, אחרת כלום ושום דבר
    // רק להכניס לתא המתאים במערך
    // לגשת ל address - sizeof(M גדול)
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
    return newAlloc;
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