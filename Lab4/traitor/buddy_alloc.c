#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "buddy_alloc.h"

#define MIN_BLOCK_SIZE 8
#define MAX_LEVELS 20

typedef struct Allocator {
    void *memory;
    size_t size;
    uint8_t *bitmap;
} Allocator;

size_t get_next_power_of_two(size_t size) {
    size_t power = 1;
    while (power < size) {
        power <<= 1;
    }
    return power;
}

Allocator* allocator_create(void *const memory, const size_t size) {
    Allocator *allocator = (Allocator*)mmap(NULL, sizeof(Allocator), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (allocator == MAP_FAILED) {
        return NULL;
    }

    size_t actual_size = get_next_power_of_two(size);
    allocator->memory = mmap(NULL, actual_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (allocator->memory == MAP_FAILED) {
        munmap(allocator, sizeof(Allocator));
        return NULL;
    }

    allocator->size = actual_size;
    allocator->bitmap = (uint8_t*)mmap(NULL, actual_size / MIN_BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (allocator->bitmap == MAP_FAILED) {
        munmap(allocator->memory, actual_size);
        munmap(allocator, sizeof(Allocator));
        return NULL;
    }

    memset(allocator->bitmap, 0, actual_size / MIN_BLOCK_SIZE);
    return allocator;
}

void allocator_destroy(Allocator *const allocator) {
    if (allocator) {
        munmap(allocator->memory, allocator->size);
        munmap(allocator->bitmap, allocator->size / MIN_BLOCK_SIZE);
        munmap(allocator, sizeof(Allocator));
    }
}

void* allocator_alloc(Allocator *const allocator, const size_t size) {
    if (!allocator || size == 0) {
        return NULL;
    }

    size_t block_size = get_next_power_of_two(size);
    if (block_size < MIN_BLOCK_SIZE) {
        block_size = MIN_BLOCK_SIZE;
    }

    size_t index = 0;
    size_t level = 0;
    while (block_size < allocator->size) {
        block_size <<= 1;
        level++;
    }

    if (block_size > allocator->size) {
        return NULL;
    }

    for (size_t i = 0; i < (allocator->size / block_size); i++) {
        if (!allocator->bitmap[index + i]) {
            allocator->bitmap[index + i] = 1;
            return (void*)((uint8_t*)allocator->memory + (i * block_size));
        }
    }

    return NULL;
}

void allocator_free(Allocator *const allocator, void *const memory) {
    if (!allocator || !memory) {
        return;
    }

    size_t offset = (uint8_t*)memory - (uint8_t*)allocator->memory;
    size_t block_size = MIN_BLOCK_SIZE;
    size_t index = offset / block_size;

    while (block_size < allocator->size) {
        if (allocator->bitmap[index]) {
            allocator->bitmap[index] = 0;
            return;
        }
        block_size *= 2;
        index /= 2;
    }
}