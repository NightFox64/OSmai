#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h> // Для измерения времени

typedef struct Allocator {
    size_t size;   // Размер памяти, выделенной для аллокатора
    void *memory;  // Указатель на выделенную память
} Allocator;

// Определение типов для функций из динамических библиотек
typedef Allocator* (*allocator_create_f)(void *const memory, const size_t size);
typedef void (*allocator_destroy_f)(Allocator *const allocator);
typedef void* (*allocator_alloc_f)(Allocator *const allocator, const size_t size);
typedef void (*allocator_free_f)(Allocator *const allocator, void *const memory);

// Указатели на функции, которые будут загружаться динамически
static allocator_create_f allocator_create = NULL;
static allocator_destroy_f allocator_destroy = NULL;
static allocator_alloc_f allocator_alloc = NULL;
static allocator_free_f allocator_free = NULL;

// Функции для fallback-аллокатора
Allocator* fallback_allocator_create(void *const memory, const size_t size) {
    Allocator *allocator = (Allocator*) mmap(NULL, sizeof(Allocator), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (allocator != MAP_FAILED) {
        allocator->size = size;
        allocator->memory = memory;
    }
    return allocator;
}

void fallback_allocator_destroy(Allocator *const allocator) {
    munmap(allocator->memory, allocator->size);
    munmap(allocator, sizeof(Allocator));
}

void* fallback_allocator_alloc(Allocator *const allocator, const size_t size) {
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
}

void fallback_allocator_free(Allocator *const allocator, void *const memory) {
    munmap(memory, sizeof(memory));
}

// Функция для загрузки библиотеки
void load_allocator_library(const char *path) {
    void *handle = dlopen(path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Error loading library: %s\n", dlerror());
        return;
    }

    allocator_create = (allocator_create_f) dlsym(handle, "allocator_create");
    allocator_destroy = (allocator_destroy_f) dlsym(handle, "allocator_destroy");
    allocator_alloc = (allocator_alloc_f) dlsym(handle, "allocator_alloc");
    allocator_free = (allocator_free_f) dlsym(handle, "allocator_free");

    if (!allocator_create || !allocator_destroy || !allocator_alloc || !allocator_free) {
        fprintf(stderr, "Error loading functions from library\n");
        dlclose(handle);
    }
}

// Функция для измерения времени работы аллокатора
double measure_time_allocation(Allocator *allocator, size_t alloc_size, int num_allocs) {
    clock_t start = clock();
    for (int i = 0; i < num_allocs; ++i) {
        void *block = allocator_alloc(allocator, alloc_size);
        if (!block) {
            fprintf(stderr, "Allocation failed at iteration %d\n", i);
            break;
        }
        allocator_free(allocator, block);
    }
    clock_t end = clock();
    return (double)(end - start) / CLOCKS_PER_SEC;
}

// Функция для измерения времени освобождения памяти
double measure_time_free(Allocator *allocator, size_t alloc_size, int num_allocs) {
    void **blocks = malloc(num_allocs * sizeof(void*));
    for (int i = 0; i < num_allocs; ++i) {
        blocks[i] = allocator_alloc(allocator, alloc_size);
    }

    clock_t start = clock();
    for (int i = 0; i < num_allocs; ++i) {
        allocator_free(allocator, blocks[i]);
    }
    clock_t end = clock();
    free(blocks);
    return (double)(end - start) / CLOCKS_PER_SEC;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("No library path provided, using fallback allocator\n");
        allocator_create = (allocator_create_f) fallback_allocator_create;
        allocator_destroy = (allocator_destroy_f) fallback_allocator_destroy;
        allocator_alloc = (allocator_alloc_f) fallback_allocator_alloc;
        allocator_free = (allocator_free_f) fallback_allocator_free;
    } else {
        load_allocator_library(argv[1]);
        if (!allocator_create) {
            printf("Failed to load library, using fallback allocator\n");
            allocator_create = (allocator_create_f) fallback_allocator_create;
            allocator_destroy = (allocator_destroy_f) fallback_allocator_destroy;
            allocator_alloc = (allocator_alloc_f) fallback_allocator_alloc;
            allocator_free = (allocator_free_f) fallback_allocator_free;
        } else {
            printf("Library loaded successfully\n");
        }
    }

    size_t memory_size = 1024 * 1024 * 10; // 10MB
    void *memory = mmap(NULL, memory_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (memory == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    Allocator *allocator = allocator_create(memory, memory_size);
    if (!allocator) {
        fprintf(stderr, "Allocator creation failed\n");
        return 1;
    }

    // Сравнение времени работы аллокаторов
    printf("Measuring allocation time...\n");
    double alloc_time = measure_time_allocation(allocator, 128, 10000);
    printf("Allocation time for 10,000 allocations: %.6f seconds\n", alloc_time);

    printf("Measuring free time...\n");
    double free_time = measure_time_free(allocator, 128, 10000);
    printf("Free time for 10,000 deallocations: %.6f seconds\n", free_time);

    // Уничтожение аллокатора
    allocator_destroy(allocator);
    munmap(memory, memory_size);

    return 0;
}