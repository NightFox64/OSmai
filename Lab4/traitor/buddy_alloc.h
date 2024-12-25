#ifndef BUDDY_ALLOC_H
#define BUDDY_ALLOC_H

#include <stddef.h>

typedef struct Allocator Allocator;

Allocator* allocator_create(void *const memory, const size_t size);
void allocator_destroy(Allocator* allocator);
void* allocator_alloc(Allocator* allocator, size_t size);
void allocator_free(Allocator* allocator, void* memory);

#endif