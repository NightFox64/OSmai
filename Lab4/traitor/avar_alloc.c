#include "allocator.h"
#include <sys/mman.h>
#include <stdio.h>

struct Allocator {
    void *memory;
    size_t size;
};

Allocator* allocator_create(void *const memory, const size_t size) {
    return NULL;
}

void allocator_destroy(Allocator *const allocator) {}

void* allocator_alloc(Allocator *const allocator, const size_t size) {
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

void allocator_free(Allocator *const allocator, void *const memory) {
    munmap(memory, sizeof(memory));
}
