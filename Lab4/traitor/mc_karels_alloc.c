#include "allocator.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct FreeBlock {
    struct FreeBlock *next;
} FreeBlock;

typedef struct {
    FreeBlock *free_list;
    size_t block_size;    
    size_t total_blocks; 
    void *base;         
} McKusickAllocator;

struct Allocator {
    McKusickAllocator mcc;
};

Allocator* allocator_create(void *const memory, const size_t size) {
    if (!memory || size < sizeof(Allocator)) return NULL;

    McKusickAllocator *mcc = (McKusickAllocator *)memory;
    size_t block_size = 32; // Размер блока по умолчанию
    mcc->block_size = block_size;
    mcc->total_blocks = size / block_size;
    mcc->base = (void *)((char *)memory + sizeof(McKusickAllocator));
    mcc->free_list = NULL;

    // Инициализация свободных блоков
    for (size_t i = 0; i < mcc->total_blocks; ++i) {
        void *block = (void *)((char *)mcc->base + i * block_size);
        ((FreeBlock *)block)->next = mcc->free_list;
        mcc->free_list = (FreeBlock *)block;
    }

    return (Allocator *)mcc;
}

void allocator_destroy(Allocator *const allocator) {
    if (!allocator) return;
    memset(allocator, 0, sizeof(Allocator));
}

void* allocator_alloc(Allocator *const allocator, const size_t size) {
    if (!allocator || size == 0) return NULL;

    McKusickAllocator *mcc = &allocator->mcc;
    if (size > mcc->block_size || !mcc->free_list) return NULL;

    // Получаем блок из списка свободных
    FreeBlock *block = mcc->free_list;
    mcc->free_list = block->next;
    return (void *)block;
}

void allocator_free(Allocator *const allocator, void *const memory) {
    if (!allocator || !memory) return;

    McKusickAllocator *mcc = &allocator->mcc;

    // Возвращаем блок в список свободных
    FreeBlock *block = (FreeBlock *)memory;
    block->next = mcc->free_list;
    mcc->free_list = block;
}
