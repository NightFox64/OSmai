// Макросы и зависимости
#include <windows.h>
#include <stdint.h>
#include "demidovStdio.h"

// Интерфейс аллокатора
typedef struct Allocator Allocator;

typedef Allocator* (*allocator_create_func)(void* memory, size_t size);
typedef void (*allocator_destroy_func)(Allocator* allocator);
typedef void* (*allocator_alloc_func)(Allocator* allocator, size_t size);
typedef void (*allocator_free_func)(Allocator* allocator, void* memory);

// Обертки для системного аллокатора
Allocator* system_allocator_create(void* memory, size_t size) {
    return (Allocator*)memory;
}

void system_allocator_destroy(Allocator* allocator) {
    // Ничего не делаем для системного аллокатора
}

void* system_allocator_alloc(Allocator* allocator, size_t size) {
    return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

void system_allocator_free(Allocator* allocator, void* memory) {
    VirtualFree(memory, 0, MEM_RELEASE);
}

// Тестовые параметры
#define TEST_MEMORY_SIZE (1024 * 1024) // 1 MB
#define TEST_BLOCK_SIZE 256
#define TEST_ALLOC_COUNT 1000

void run_tests(const char* library_path) {
    HMODULE library = LoadLibrary(library_path);
    if (!library) {
        demidov_printf("Ошибка загрузки библиотеки: %s\n", library_path);
        return;
    }

    allocator_create_func allocator_create = (allocator_create_func)GetProcAddress(library, "allocator_create");
    allocator_destroy_func allocator_destroy = (allocator_destroy_func)GetProcAddress(library, "allocator_destroy");
    allocator_alloc_func allocator_alloc = (allocator_alloc_func)GetProcAddress(library, "allocator_alloc");
    allocator_free_func allocator_free = (allocator_free_func)GetProcAddress(library, "allocator_free");

    if (!allocator_create || !allocator_destroy || !allocator_alloc || !allocator_free) {
        demidov_printf("Ошибка получения функций из библиотеки: %s\n", library_path);
        FreeLibrary(library);
        return;
    }

    // Выделение памяти для тестов
    void* test_memory = VirtualAlloc(NULL, TEST_MEMORY_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!test_memory) {
        demidov_printf("Ошибка выделения тестовой памяти.\n");
        FreeLibrary(library);
        return;
    }

    Allocator* allocator = allocator_create(test_memory, TEST_MEMORY_SIZE);
    if (!allocator) {
        demidov_printf("Ошибка инициализации аллокатора.\n");
        VirtualFree(test_memory, 0, MEM_RELEASE);
        FreeLibrary(library);
        return;
    }

    // Тест скорости выделения памяти
    clock_t start = clock();
    void* blocks[TEST_ALLOC_COUNT];
    for (size_t i = 0; i < TEST_ALLOC_COUNT; i++) {
        blocks[i] = allocator_alloc(allocator, TEST_BLOCK_SIZE);
        if (!blocks[i]) {
            demidov_printf("Ошибка выделения памяти на шаге %zu.\n", i);
        }
    }
    clock_t end = clock();
    demidov_printf("Скорость выделения: %lf секунд\n", (double)(end - start) / CLOCKS_PER_SEC);

    // Тест скорости освобождения памяти
    start = clock();
    for (size_t i = 0; i < TEST_ALLOC_COUNT; i++) {
        allocator_free(allocator, blocks[i]);
    }
    end = clock();
    demidov_printf("Скорость освобождения: %lf секунд\n", (double)(end - start) / CLOCKS_PER_SEC);

    // Освобождение ресурсов
    allocator_destroy(allocator);
    VirtualFree(test_memory, 0, MEM_RELEASE);
    FreeLibrary(library);
}

// Основная программа
int main(int argc, char* argv[]) {
    HMODULE library = NULL;
    allocator_create_func allocator_create = NULL;
    allocator_destroy_func allocator_destroy = NULL;
    allocator_alloc_func allocator_alloc = NULL;
    allocator_free_func allocator_free = NULL;

    if (argc > 1) {
        library = LoadLibrary(argv[1]);
        if (library) {
            allocator_create = (allocator_create_func)GetProcAddress(library, "allocator_create");
            allocator_destroy = (allocator_destroy_func)GetProcAddress(library, "allocator_destroy");
            allocator_alloc = (allocator_alloc_func)GetProcAddress(library, "allocator_alloc");
            allocator_free = (allocator_free_func)GetProcAddress(library, "allocator_free");
        }
    }

    if (!allocator_create || !allocator_destroy || !allocator_alloc || !allocator_free) {
        demidov_printf("Failed to load libeary.\n");
        allocator_create = system_allocator_create;
        allocator_destroy = system_allocator_destroy;
        allocator_alloc = system_allocator_alloc;
        allocator_free = system_allocator_free;
    }

    // Инициализация аллокатора
    void* memory = VirtualAlloc(NULL, 1024 * 1024, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    Allocator* allocator = allocator_create(memory, 1024 * 1024);

    // Тестовая работа с аллокатором
    demidov_printf("Testing library: %s\n", argv[1]);
    run_tests(argv[1]);

    return 0;
}
