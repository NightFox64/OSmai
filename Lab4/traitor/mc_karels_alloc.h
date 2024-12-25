#ifndef MC_KARELS_ALLOC_H
#define MC_KARELS_ALLOC_H

#include <stddef.h>

typedef struct Allocator Allocator;
Allocator* allocator_create(void *const memory, const size_t size);
void allocator_destroy(Allocator *const allocator);
void* allocator_alloc(Allocator *const allocator, const size_t size);
void allocator_free(Allocator *const allocator, void *const memory);

#endif