#ifndef MEMESIM_MEMBLOCK_H_
#define MEMESIM_MEMBLOCK_H_

#include <stddef.h>
#include <stdbool.h>

typedef max_align_t MAX_ALIGN_TYPE;

typedef struct mem_block {
    size_t size;
    bool allocated;

    struct mem_block* prev_free;
    struct mem_block* next_free;

    struct mem_block* prev_phys;
    struct mem_block* next_phys;
} mem_block;

size_t align_up(size_t n, size_t alignment);

size_t block_header_size(void);

mem_block* make_mem_block(mem_block* ptr, size_t size);

mem_block* merge_mem_block(mem_block* lhs, mem_block* rhs);

mem_block* split_mem_block(mem_block* ptr, size_t size);

void* mem_block_alloc(mem_block* ptr);

#endif // MEMESIM_MEMBLOCK_H_