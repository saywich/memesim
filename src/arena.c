#include <sys/mman.h>

#include "include/arena.h"

size_t arena_header_size() {
    return align_up(sizeof(arena), _Alignof(MAX_ALIGN_TYPE));
}

arena* make_arena(size_t size) {
    size_t desc_size = arena_header_size();
    size_t block_desc_size = block_header_size();

    size_t total_size = desc_size + block_desc_size + size;

    void* mapped = mmap(
        NULL,
        total_size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
    );

    if (mapped == MAP_FAILED) {
        return NULL;
    }

    char* block_ptr = ((char*)mapped) + desc_size;

    mem_block* block = make_mem_block((mem_block*) block_ptr, size);
    block->prev_free = NULL;
    block->next_free = NULL;
    block->prev_phys = NULL;
    block->next_phys = NULL;

    arena a = {
        .mapped_size = total_size,
        .first_block = (mem_block*)(((char*)mapped) + desc_size),
    };

    arena* desc_p = (arena*) mapped;
    *desc_p = a;

    return desc_p;
}

void arena_free(arena* arena) {
    munmap((void*) arena, arena->mapped_size);
}