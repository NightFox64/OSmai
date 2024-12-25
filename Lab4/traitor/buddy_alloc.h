#ifndef OS_LAB_ALLOCATOR2_H
#define OS_LAB_ALLOCATOR2_H

#include <stddef.h>

typedef struct Allocator2 Allocator2;

Allocator2* allocator2_create(size_t size);
void allocator2_destroy(Allocator2* allocator);
void* allocator2_alloc(Allocator2* allocator, size_t size);
void allocator2_free(Allocator2* allocator, void* memory);

#endif //OS_LAB_ALLOCATOR2_H