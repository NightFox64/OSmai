#ifndef __MC_KARELS_ALLOC_H
#define __MC_KARELS_ALLOC_H
#include <windows.h>
#include <stdint.h>
#include <math.h>
#include "demidovStdio.h"


// Экспортируемые функции
typedef Allocator* allocator_create(void* memory, size_t size);
typedef void allocator_destroy(Allocator* allocator);
typedef void* allocator_alloc(Allocator* allocator, size_t size);
typedef void allocator_free(Allocator* allocator, void* memory);

#endif // __MC_KARELS_ALLOC_H