#ifndef OS_LAB_ALLOCATOR_H
#define OS_LAB_ALLOCATOR_H

#include <stddef.h>

typedef struct Allocator Allocator;

// Инициализация аллокатора
Allocator* allocator_create(void *const memory, const size_t size);

// Деинициализация аллокатора
void allocator_destroy(Allocator *const allocator);

// Выделение памяти
void* allocator_alloc(Allocator *const allocator, const size_t size);

// Освобождение памяти
void allocator_free(Allocator *const allocator, void *const memory);

#endif //OS_LAB_ALLOCATOR_H