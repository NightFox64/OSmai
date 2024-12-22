#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "allocator.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path_to_allocator.so>\n", argv[0]);
        return 1;
    }

    void *lib = dlopen(argv[1], RTLD_LAZY);
    if (!lib) {
        fprintf(stderr, "Error loading library: %s\n", dlerror());
        return 1;
    }

    Allocator* (*allocator_create)(void *, size_t) = dlsym(lib, "allocator_create");
    void (*allocator_destroy)(Allocator *) = dlsym(lib, "allocator_destroy");
    void* (*allocator_alloc)(Allocator *, size_t) = dlsym(lib, "allocator_alloc");
    void (*allocator_free)(Allocator *, void *) = dlsym(lib, "allocator_free");

    if (!allocator_create || !allocator_destroy || !allocator_alloc || !allocator_free) {
        fprintf(stderr, "Invalid allocator library\n");
        dlclose(lib);
        return 1;
    }

    // Выделяем память для теста
    void *memory = mmap(NULL, 1024 * 1024, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (!memory) {
        perror("mmap");
        dlclose(lib);
        return 1;
    }

    Allocator *allocator = allocator_create(memory, 1024 * 1024);
    void *block = allocator_alloc(allocator, 256);
    allocator_free(allocator, block);
    allocator_destroy(allocator);

    munmap(memory, 1024 * 1024);
    dlclose(lib);
    return 0;
}
