#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>

typedef struct Allocator Allocator;

// Создание аллокатора
Allocator* allocator_create(void *const memory, const size_t size);

// Уничтожение аллокатора
void allocator_destroy(Allocator *const allocator);

// Выделение памяти
void* allocator_alloc(Allocator *const allocator, const size_t size);

// Освобождение памяти
void allocator_free(Allocator *const allocator, void *const memory);

#endif
