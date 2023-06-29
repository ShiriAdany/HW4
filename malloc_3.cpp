#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

typedef struct MallocMetadata {
    size_t size;
    bool is_free;
    MallocMetadata* next;
    MallocMetadata* prev;
} MallocMetadata;

//typedef struct MMapList {
//    MallocMetadata* blockMeta;
//    MMapList* next;
//    MMapList* prev;
//} MMapList;

//void *head = nullptr;
size_t numOfFreeBlocks = 0 ;
size_t numOfFreeBytes = 0;
size_t totalBlocksCount = 0;
size_t totalBlocksBytes = 0;

const int MAX_ORDER = 10;
const size_t TOTAL_BLOCKS_SIZE = 32 * 128 * 1024;

MallocMetadata* arr[MAX_ORDER+1];
MallocMetadata* mmapList = nullptr;
unsigned int COOKIE;
//MMapList* mapList = nullptr;
//arr[10] = sbrk

void _initiateBlocks() {
    void* currentProgramBreak = sbrk(0);
    if (currentProgramBreak == (void*)(-1))
        return;

    COOKIE = rand();

    size_t offset = TOTAL_BLOCKS_SIZE - (unsigned long)currentProgramBreak % TOTAL_BLOCKS_SIZE;
    void* allocAddress = sbrk(offset + TOTAL_BLOCKS_SIZE);


    if (allocAddress == (void*)(-1))
        return;
    allocAddress = (void*)((unsigned long)allocAddress + offset); //now it's aligned - one block
    void* temp = allocAddress;
    void* prev;
    for(int i = 0; i < 32 ; i++)
    {
        MallocMetadata* metadata = (MallocMetadata*)temp;
        metadata->COOKIE = COOKIE;
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
        prev = (MallocMetadata*)temp;
        temp = (void*)((unsigned long)temp + metadata->size);
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
                        if (metadata->COOKIE != COOKIE)
                            exit(0xdeadbeef);
                        //remove from layer
                        arr[j] = arr[j]->next;
                        if (arr[j])
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

                        totalBlocksCount++;
                        numOfFreeBlocks++;
                        numOfFreeBytes -= sizeof(MallocMetadata);

                    }
                }
            }

            //bring the first block in the list, point to the next one in list
            if (arr[i]->COOKIE != COOKIE)
                exit(0xdeadbeef);
            void* address = arr[i];
            if(arr[i]->next) {
                if (arr[i]->next->COOKIE != COOKIE)
                    exit(0xdeadbeef);
                arr[i]->next->prev = nullptr;
            }
            arr[i] = arr[i]->next;
            ((MallocMetadata*)address)->is_free = false; //meta of the big

            numOfFreeBytes -= ((MallocMetadata*)address)->size;
            numOfFreeBlocks--;

            return (void *)((unsigned long)address + sizeof(MallocMetadata));
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

    void* metadataAddress = (void *)((unsigned long)p - sizeof(MallocMetadata));
    MallocMetadata* metadata = (MallocMetadata*) metadataAddress;
    if (metadata->COOKIE != COOKIE)
        exit(0xdeadbeef);

    if (metadata->size >= 128 * 1024)
    {
        munmap(metadata,metadata->size + sizeof(MallocMetadata));
        totalBlocksCount--;
        totalBytesMMap-= metadata->size;
        return;
    }

    void* buddyAddress = (void *)((unsigned long)metadataAddress ^ metadata->size); //should be XOR
    MallocMetadata* buddyBlock = (MallocMetadata*) buddyAddress;
    if (buddyBlock->COOKIE != COOKIE)
        exit(0xdeadbeef);

    numOfFreeBlocks++;
    numOfFreeBytes += metadata->size - sizeof(MallocMetadata);

    if (!buddyBlock->is_free)
    {
        size_t blockSize = buddyBlock->size;
        int order = blockSize/128 - 1;
        metadata->is_free = true;
        AddToArray(metadata,order);

        return;
    }
    while (buddyBlock->is_free)
    {
        size_t blockSize = buddyBlock->size;
//        printf("%zu is the block size\n",blockSize);
        int order = log2(blockSize/128);
        if(buddyBlock->prev)
            buddyBlock->prev->next = buddyBlock->next;
        else
            arr[order] = buddyBlock->next;

        if(buddyBlock->next)
            buddyBlock->next->prev = buddyBlock->prev;

        if(order < MAX_ORDER )
        {
            if (metadataAddress > buddyAddress) {
                //merge into buddy
                buddyBlock->size *= 2;
                buddyBlock->is_free = true;
                AddToArray(buddyBlock,order + 1);
                metadata = buddyBlock; //buddyBlock is the merged block
                buddyAddress = (void *)((unsigned long)metadataAddress ^ metadata->size);
                buddyBlock = (MallocMetadata*) buddyAddress;
                if (buddyBlock->COOKIE != COOKIE)
                    exit(0xdeadbeef);
            }
            else
            {
                //merge into meta
                metadata->size *= 2;
                metadata->is_free = true;
                AddToArray(metadata,order + 1);
                buddyAddress = (void *)((unsigned long)metadataAddress ^ metadata->size);
                buddyBlock = (MallocMetadata*) buddyAddress;
                if (buddyBlock->COOKIE != COOKIE)
                    exit(0xdeadbeef);
            }
//            printf("order is %d\n",order);
            numOfFreeBlocks--;
            numOfFreeBytes += sizeof(MallocMetadata);
            totalBlocksCount--;
        }
        else
        {
            //can't unite
            AddToArray(metadata,MAX_ORDER);
            return;
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

    int originalSize = oldMeta->size;
    void* buddyAddress = (void *)((unsigned long)oldMeta ^ oldMeta->size);
    MallocMetadata* buddyBlock = (MallocMetadata*)(buddyAddress);
    if (buddyBlock->COOKIE != COOKIE)
        exit(0xdeadbeef);
    size_t freeSpace = oldMeta->size;
    MallocMetadata* temp = oldMeta;
    size_t currentSize = oldMeta->size;
    while (buddyBlock->is_free)
    {
        if (temp->COOKIE != COOKIE)
            exit(0xdeadbeef);
        //count how much free space we might have by merging
        freeSpace += buddyBlock->size + sizeof(MallocMetadata);
        temp = buddyBlock < temp ? buddyBlock : temp;
        if (temp->COOKIE != COOKIE)
            exit(0xdeadbeef);
        currentSize *= 2;
        buddyBlock = (MallocMetadata *)((unsigned long)temp ^ temp->size);
        if (buddyBlock->COOKIE != COOKIE)
            exit(0xdeadbeef);
    }

    if (freeSpace >= size) {
        //merge the blocks
        freeSpace = oldMeta->size;
        buddyAddress = (void *)((unsigned long)oldMeta ^ oldMeta->size);
        buddyBlock = (MallocMetadata *) (buddyAddress);
        if (buddyBlock->COOKIE != COOKIE)
            exit(0xdeadbeef);

        while (freeSpace < size) {
            size_t blockSize = buddyBlock->size;
            int order = blockSize / 128 - 1;
            if (buddyBlock->prev)
                buddyBlock->prev->next = buddyBlock->next;
            else
                arr[order] = buddyBlock->next;

            if (buddyBlock->next)
                buddyBlock->next->prev = buddyBlock->prev;

            freeSpace += buddyBlock->size + sizeof(MallocMetadata);
            if (order < MAX_ORDER) {
                if (oldMeta > buddyAddress) {
                    //merge into buddy
                    buddyBlock->size *= 2;
//                    buddyBlock->is_free = false;
                    AddToArray(buddyBlock, order + 1);
                    oldMeta = buddyBlock; //buddyBlock is the merged block
                    buddyAddress = (void *)((unsigned long)oldMeta ^ oldMeta->size);
                    buddyBlock = (MallocMetadata *) buddyAddress;
                    if (buddyBlock->COOKIE != COOKIE)
                        exit(0xdeadbeef);
                } else {
                    //merge into oldMeta
                    oldMeta->size *= 2;
//                    oldMeta->is_free = false;
                    AddToArray(oldMeta, order + 1);
                    buddyAddress = (void*)((unsigned long)oldMeta ^ oldMeta->size); //TODO: NOTE maybe use 'xor'
                    buddyBlock = (MallocMetadata *) buddyAddress;
                    if (buddyBlock->COOKIE != COOKIE)
                        exit(0xdeadbeef);
                }
                numOfFreeBlocks--;
                numOfFreeBytes += sizeof(MallocMetadata); //to make up for later ???
                totalBlocksCount--;
            } else {
                //can't unite
                //here we reached max block size on oldMeta
//                memmove(oldMeta + sizeof(MallocMetadata), oldp, oldMeta->size);
//                return oldMeta + sizeof(MallocMetadata);
                break;
            }
        }
        numOfFreeBytes = numOfFreeBytes + (originalSize - sizeof(MallocMetadata)) - (oldMeta->size - sizeof(MallocMetadata));
        //original size is the size of the original block
        //we assume that originalSize would be 128 and not 96 (if metadata size is 32)
        oldMeta->is_free = false;
        memmove(oldMeta + sizeof(MallocMetadata), oldp, oldMeta->size);
        return (void *)((unsigned long)oldMeta + sizeof(MallocMetadata));
    }
    else
    {
        void* newAlloc = smalloc(size);
        if (newAlloc == NULL)
        {
            return NULL;
        }
        memmove(newAlloc,oldp,oldMeta->size);
        sfree(oldp);
        return newAlloc;
    }
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
    if (totalBlocksCount == 0)
        return 0;
    return TOTAL_BLOCKS_SIZE - totalBlocksCount * sizeof(MallocMetadata) + totalBytesMMap;
//    return totalBlocksBytes;
}

size_t _num_meta_data_bytes(){
    return totalBlocksCount*sizeof(MallocMetadata);
}

size_t _size_meta_data(){
    return sizeof (MallocMetadata);
}