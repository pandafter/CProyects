#include "core/memory.h"
#include <stdio.h>
#include <stdlib.h>

// Global memory statistics
static MemoryStats g_memory_stats = {0};

void* safe_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        printf("ERROR: Failed to allocate %zu bytes\n", size);
        return NULL;
    }
    
    g_memory_stats.total_allocated += size;
    g_memory_stats.current_usage += size;
    g_memory_stats.allocation_count++;
    
    if (g_memory_stats.current_usage > g_memory_stats.peak_usage) {
        g_memory_stats.peak_usage = g_memory_stats.current_usage;
    }
    
    return ptr;
}

void* safe_calloc(size_t count, size_t size) {
    void* ptr = calloc(count, size);
    if (!ptr) {
        printf("ERROR: Failed to allocate %zu bytes\n", count * size);
        return NULL;
    }
    
    size_t total_size = count * size;
    g_memory_stats.total_allocated += total_size;
    g_memory_stats.current_usage += total_size;
    g_memory_stats.allocation_count++;
    
    if (g_memory_stats.current_usage > g_memory_stats.peak_usage) {
        g_memory_stats.peak_usage = g_memory_stats.current_usage;
    }
    
    return ptr;
}

void safe_free(void* ptr) {
    if (ptr) {
        free(ptr);
        // Note: We can't track exact size freed without additional overhead
        // This is a simplified implementation
    }
}

MemoryPool* create_memory_pool(size_t total_size, size_t block_size) {
    MemoryPool* pool = (MemoryPool*)safe_malloc(sizeof(MemoryPool));
    if (!pool) return NULL;
    
    pool->memory = safe_malloc(total_size);
    if (!pool->memory) {
        safe_free(pool);
        return NULL;
    }
    
    pool->size = total_size;
    pool->used = 0;
    pool->block_size = block_size;
    
    printf("Memory pool creado: %zu bytes total, %zu bytes por bloque\n", total_size, block_size);
    
    return pool;
}

void destroy_memory_pool(MemoryPool* pool) {
    if (pool) {
        safe_free(pool->memory);
        safe_free(pool);
    }
}

void* pool_alloc(MemoryPool* pool) {
    if (!pool) return NULL;
    
    // Check if we have enough space
    if (pool->used + pool->block_size > pool->size) {
        printf("ERROR: Pool de memoria agotado. Usado: %zu/%zu bytes\n", pool->used, pool->size);
        return NULL;
    }
    
    void* ptr = (char*)pool->memory + pool->used;
    pool->used += pool->block_size;
    
    printf("Pool alloc: %zu bytes asignados, usado: %zu/%zu\n", pool->block_size, pool->used, pool->size);
    
    return ptr;
}

void pool_free(MemoryPool* pool, void* ptr) {
    // Simple implementation - just reset the pool
    // In a more sophisticated system, you'd track individual allocations
    (void)pool;
    (void)ptr;
}

void pool_reset(MemoryPool* pool) {
    if (pool) {
        printf("Pool de memoria reiniciado. Usado: %zu -> 0\n", pool->used);
        pool->used = 0;
    }
}

MemoryStats* get_memory_stats() {
    return &g_memory_stats;
}

void print_memory_stats() {
    printf("=== MEMORY STATISTICS ===\n");
    printf("Total Allocated: %zu bytes\n", g_memory_stats.total_allocated);
    printf("Total Freed: %zu bytes\n", g_memory_stats.total_freed);
    printf("Current Usage: %zu bytes\n", g_memory_stats.current_usage);
    printf("Peak Usage: %zu bytes\n", g_memory_stats.peak_usage);
    printf("Allocation Count: %d\n", g_memory_stats.allocation_count);
    printf("========================\n");
}
