#include "allocator.h"
#include <math.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define MAX_LEVELS 20

typedef struct {
    uint8_t *bitmap;
    size_t total_size;
    size_t min_block;
    void *base;       
} BuddyAllocator;

struct Allocator {
    BuddyAllocator buddy;
};

static size_t next_power_of_two(size_t x) {
    return pow(2, ceil(log2(x)));
}

Allocator* allocator_create(void *const memory, const size_t size) {
    if (!memory || size < sizeof(Allocator)) return NULL;

    BuddyAllocator *buddy = (BuddyAllocator *)memory;
    size_t min_block = 16; // Минимальный размер блока (например, 16 байт)
    size_t levels = (size_t)(log2(size / min_block));

    buddy->total_size = size;
    buddy->min_block = min_block;
    buddy->base = (void *)((char *)memory + sizeof(BuddyAllocator));
    buddy->bitmap = (uint8_t *)((char *)buddy->base + buddy->total_size);

    // Инициализация битовой карты
    memset(buddy->bitmap, 0, 1 << levels);

    return (Allocator *)buddy;
}

void allocator_destroy(Allocator *const allocator) {
    if (!allocator) return;
    memset(allocator, 0, sizeof(Allocator));
}

void* allocator_alloc(Allocator *const allocator, const size_t size) {
    if (!allocator || size == 0) return NULL;

    BuddyAllocator *buddy = &allocator->buddy;
    size_t aligned_size = next_power_of_two(size);
    size_t block_size = buddy->min_block;
    size_t index = 0;

    // Найти подходящий свободный блок
    while (block_size < aligned_size && block_size <= buddy->total_size) {
        block_size *= 2;
        index++;
    }

    if (block_size > buddy->total_size) return NULL;

    // Устанавливаем блок как занятый
    buddy->bitmap[index] = 1;

    return (void *)((char *)buddy->base + index * block_size);
}

void allocator_free(Allocator *const allocator, void *const memory) {
    if (!allocator || !memory) return;

    BuddyAllocator *buddy = &allocator->buddy;
    size_t offset = (size_t)((char *)memory - (char *)buddy->base);

    if (offset >= buddy->total_size) return;

    size_t block_size = buddy->min_block;
    size_t index = offset / block_size;

    // Сбрасываем бит в битовой карте
    buddy->bitmap[index] = 0;
}
