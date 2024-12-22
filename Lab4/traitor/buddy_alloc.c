#include "allocator.h"
#include <string.h>
#include <math.h>

struct Allocator {
    void *memory;
    size_t size;
};

// Инициализация
Allocator* allocator_create(void *const memory, const size_t size) {
    Allocator *allocator = (Allocator *)memory;
    allocator->memory = (void *)((char *)memory + sizeof(Allocator));
    allocator->size = size - sizeof(Allocator);
    return allocator;
}

// Деинициализация
void allocator_destroy(Allocator *const allocator) {
    // Очистка данных
}

// Выделение памяти
void* allocator_alloc(Allocator *const allocator, const size_t size) {
    // Логика алгоритма двойников
    return NULL;
}

// Освобождение памяти
void allocator_free(Allocator *const allocator, void *const memory) {
    // Возвращение блока в пул
}
