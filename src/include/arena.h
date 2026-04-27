#ifndef MEMESIM_ARENA_H_
#define MEMESIM_ARENA_H_

#include <stddef.h>

#include "mem_block.h"

typedef struct arena {
    size_t mapped_size;
    mem_block* first_block;
} arena;

size_t arena_header_size();

arena* make_arena(size_t size);

void arena_free(arena* arena);

#endif // MEMESIM_ARENA_H_