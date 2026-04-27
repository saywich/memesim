#include <sys/mman.h>
#include <stdint.h>
#include <string.h>

#include "./include/meme.h"
#include "include/allocator.h"

static const size_t kMegabyte = 1024 * 1024;

static allocator alloc = {
    .free_block_list = NULL,
    .start_block_size = kMegabyte
};

void* ms_malloc_(size_t size) {
    mem_block* block = allocator_alloc(&alloc, size);

    if (block == NULL) return NULL;

    return mem_block_alloc(block);
}

void* ms_calloc_(size_t n, size_t size) {
    if (size != 0 && n > SIZE_MAX / size) {
        return NULL;
    }

    size_t requested_size = n * size;

    void* mapped = ms_malloc_(requested_size);
    if (mapped == NULL) return NULL;

    mapped = memset(mapped, 0, requested_size);

    return mapped;
}

void* ms_realloc_(void* ptr, size_t size) {
    if (ptr == NULL) {
        return ms_malloc_(size);
    } else if (size == 0) {
        ms_free_(ptr);

        return NULL;
    }

    mem_block* block = (mem_block*)(((char*) ptr) - block_header_size());
    mem_block* reallocated = allocator_realloc(&alloc, block, size);

    if (reallocated == NULL) return NULL;

    return mem_block_alloc(reallocated);
}

void ms_free_(void* ptr) {
    if (ptr == NULL) return;

    mem_block* block = (mem_block*)(((char*) ptr) - block_header_size());

    allocator_free(&alloc, block);
}

