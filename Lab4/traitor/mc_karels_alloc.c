#include "mc_karels_alloc.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Определения для классов размеров
#define MAX_CLASSES 10
#define PAGE_SIZE 4096

typedef struct Block {
    struct Block* next;
} Block;

typedef struct Allocator {
    Block* free_list[MAX_CLASSES];
    size_t class_sizes[MAX_CLASSES];
    void* memory_start;
    size_t memory_size;
} Allocator;

static size_t find_class(size_t size, size_t* class_sizes, size_t num_classes) {
    for (size_t i = 0; i < num_classes; i++) {
        if (size <= class_sizes[i]) {
            return i;
        }
    }
    return num_classes; // Error case
}

Allocator* allocator_create(void* const memory, const size_t size) {
    Allocator* allocator = (Allocator*)memory;
    if (size < sizeof(Allocator)) return NULL;

    allocator->memory_start = (char*)memory + sizeof(Allocator);
    allocator->memory_size = size - sizeof(Allocator);

    // Инициализация классов размеров
    size_t block_size = 16;
    for (size_t i = 0; i < MAX_CLASSES; i++) {
        allocator->class_sizes[i] = block_size;
        allocator->free_list[i] = NULL;
        block_size *= 2;
    }

    return allocator;
}

void allocator_destroy(Allocator* const allocator) {
    // Очистка списков свободных блоков
    for (size_t i = 0; i < MAX_CLASSES; i++) {
        allocator->free_list[i] = NULL;
    }
}

void* allocator_alloc(Allocator* const allocator, const size_t size) {
    size_t class_index = find_class(size, allocator->class_sizes, MAX_CLASSES);
    if (class_index >= MAX_CLASSES) return NULL;

    Block* block = allocator->free_list[class_index];
    if (block) {
        allocator->free_list[class_index] = block->next;
        return (void*)block;
    }

    // Если свободных блоков нет, выделяем из общей памяти
    size_t block_size = allocator->class_sizes[class_index];
    if (allocator->memory_size < block_size) return NULL;

    void* memory = allocator->memory_start;
    allocator->memory_start = (char*)allocator->memory_start + block_size;
    allocator->memory_size -= block_size;
    return memory;
}

void allocator_free(Allocator* const allocator, void* const memory) {
    if (!memory) return;

    uintptr_t addr = (uintptr_t)memory - (uintptr_t)allocator;
    if (addr >= allocator->memory_size) return;

    size_t class_index = 0;
    size_t block_size = 0;
    for (; class_index < MAX_CLASSES; class_index++) {
        block_size = allocator->class_sizes[class_index];
        if ((uintptr_t)memory % block_size == 0) {
            break;
        }
    }

    if (class_index >= MAX_CLASSES) return;

    Block* block = (Block*)memory;
    block->next = allocator->free_list[class_index];
    allocator->free_list[class_index] = block;
}