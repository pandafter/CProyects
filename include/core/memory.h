#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

// Memory management functions
void* safe_malloc(size_t size);
void* safe_calloc(size_t count, size_t size);
void safe_free(void* ptr);

// Memory pool for chunks
typedef struct {
    void* memory;
    size_t size;
    size_t used;
    size_t block_size;
} MemoryPool;

MemoryPool* create_memory_pool(size_t total_size, size_t block_size);
void destroy_memory_pool(MemoryPool* pool);
void* pool_alloc(MemoryPool* pool);
void pool_free(MemoryPool* pool, void* ptr);
void pool_reset(MemoryPool* pool);

// Memory statistics
typedef struct {
    size_t total_allocated;
    size_t total_freed;
    size_t peak_usage;
    size_t current_usage;
    int allocation_count;
} MemoryStats;

MemoryStats* get_memory_stats();
void print_memory_stats();

#endif // MEMORY_H
