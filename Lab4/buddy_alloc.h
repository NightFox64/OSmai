#ifndef __BUDDY_ALLOC_H
#define __BUDDY_ALLOC_H

#include <windows.h>
#include <stdint.h>
#include <math.h>
#include "demidovStdio.h"

typedef struct Allocator {
    void* memory;
    size_t size;
    size_t max_order;
    uint8_t* free_blocks; // Битовая карта свободных блоков
} Allocator;

// Экспортируемые функции
__declspec(dllexport) Allocator* allocator_create(void* memory, size_t size);
__declspec(dllexport) void allocator_destroy(Allocator* allocator);
__declspec(dllexport) void* allocator_alloc(Allocator* allocator, size_t size);
__declspec(dllexport) void allocator_free(Allocator* allocator, void* memory);

#endif // __BUDDY_ALLOC_H