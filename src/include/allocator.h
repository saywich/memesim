#ifndef MEMESIM_ALLOCATOR_H_
#define MEMESIM_ALLOCATOR_H_

#include "arena.h"

typedef struct {
    mem_block* free_block_list;
    size_t start_block_size;
} allocator;

mem_block* allocator_alloc(allocator* alloc, size_t size);

mem_block* allocator_realloc(allocator* alloc, mem_block* block, size_t requested_size);

void allocator_free(allocator* alloc, mem_block* block);

#endif // MEMESIM_ALLOCATOR_H_