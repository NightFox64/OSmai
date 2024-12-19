#include "buddy_alloc.h"

#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

EXPORT size_t get_order(size_t size) {
    return (size_t)ceil(log2(size));
}

EXPORT Allocator* allocator_create(void* memory, size_t size) {
    Allocator* allocator = (Allocator*)memory;
    allocator->memory = (uint8_t*)memory + sizeof(Allocator);
    allocator->size = size - sizeof(Allocator);
    allocator->max_order = (size_t)log2(allocator->size);
    size_t bitmap_size = (1 << allocator->max_order) / 8;

    allocator->free_blocks = (uint8_t*)allocator->memory;
    for (size_t i = 0; i < bitmap_size; i++) {
        allocator->free_blocks[i] = 0xFF;
    }

    return allocator;
}

EXPORT void allocator_destroy(Allocator* allocator) {
    // Освобождение не требуется
}

EXPORT void* allocator_alloc(Allocator* allocator, size_t size) {
    size_t order = get_order(size);
    if (order > allocator->max_order) {
        demidov_printf("Error: the size you want is too long.\n");
        return NULL;
    }

    size_t block_size = 1 << order;
    size_t total_blocks = allocator->size / block_size;

    for (size_t i = 0; i < total_blocks; i++) {
        size_t byte_index = i / 8;
        size_t bit_index = i % 8;

        if (allocator->free_blocks[byte_index] & (1 << bit_index)) {
            allocator->free_blocks[byte_index] &= ~(1 << bit_index);
            return (uint8_t*)allocator->memory + i * block_size;
        }
    }

    demidov_printf("Error: not enough memory.\n");
    return NULL;
}

EXPORT void allocator_free(Allocator* allocator, void* memory) {
    size_t offset = (uint8_t*)memory - (uint8_t*)allocator->memory;
    size_t index = offset / (1 << allocator->max_order);

    if (index < (allocator->size / (1 << allocator->max_order))) {
        size_t byte_index = index / 8;
        size_t bit_index = index % 8;

        allocator->free_blocks[byte_index] |= (1 << bit_index);
    } else {
        demidov_printf("Error: wrong memory adress.\n");
    }
}
