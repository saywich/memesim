#include <string.h>

#include "include/allocator.h"

#define MAX(a, b) a < b ? b : a

static void alloc_free_list_remove(allocator* alloc, mem_block* block) {
    if (block->prev_free != NULL) {
        block->prev_free->next_free = block->next_free;
    } else {
        alloc->free_block_list = block->next_free;
    }

    if (block->next_free != NULL) {
        block->next_free->prev_free = block->prev_free;
    }

    block->prev_free = NULL;
    block->next_free = NULL;
}

static void alloc_free_list_insert(allocator* alloc, mem_block* block) {
    block->prev_free = NULL;
    block->next_free = alloc->free_block_list;

    if (alloc->free_block_list != NULL) {
        alloc->free_block_list->prev_free = block;
    }

    alloc->free_block_list = block;
}

static void alloc_free_list_merge_insert(allocator* alloc, mem_block* block) {
    block->allocated = false;
    block->prev_free = NULL;
    block->next_free = NULL;

    if (block->prev_phys != NULL && !block->prev_phys->allocated) {
        alloc_free_list_remove(alloc, block->prev_phys);
        block = merge_mem_block(block->prev_phys, block);
    }

    if (block->next_phys != NULL && !block->next_phys->allocated) {
        alloc_free_list_remove(alloc, block->next_phys);
        block = merge_mem_block(block, block->next_phys);
    }

    if (block->prev_phys == NULL && block->next_phys == NULL) {
        arena* a = (arena*)(((char*) block) - arena_header_size());
        arena_free(a);
    } else {
        alloc_free_list_insert(alloc, block);
    }
}

mem_block* allocator_alloc(allocator* alloc, size_t size) {
    mem_block* free_block = alloc->free_block_list;

    while (free_block != NULL && free_block->size < size) {
        free_block = free_block->next_free;
    }

    if (free_block == NULL) {
        arena* new_arena = make_arena(MAX(size, alloc->start_block_size));

        if (new_arena == NULL) {
            return NULL;
        }

        alloc_free_list_insert(alloc, new_arena->first_block);

        return allocator_alloc(alloc, size);
    }

    size_t aligned_size = align_up(size, _Alignof(MAX_ALIGN_TYPE));

    free_block->allocated = true;
    alloc_free_list_remove(alloc, free_block);

    if (free_block->size > aligned_size + block_header_size()) {
        mem_block* new_block = split_mem_block(free_block, size);
        alloc_free_list_insert(alloc, new_block);
    }

    return free_block;
}

mem_block* allocator_realloc(allocator* alloc, mem_block* block, size_t requested_size) {
    size_t required_size = align_up(requested_size, _Alignof(MAX_ALIGN_TYPE));

    if (required_size + block_header_size() < block->size) {
        mem_block* remainder = split_mem_block(block, requested_size);
        alloc_free_list_merge_insert(alloc, remainder);

        return block;
    }

    size_t available_merge_size = block->size;
    size_t header_size = block_header_size();

    mem_block* traverse_block = block->next_phys;
    while (traverse_block != NULL && !traverse_block->allocated && available_merge_size < required_size) {
        available_merge_size += traverse_block->size + header_size;
        traverse_block = traverse_block->next_phys;
    }

    if (available_merge_size >= required_size) {
        mem_block* merge_block = block->next_phys;

        while (merge_block != NULL && !merge_block->allocated && block->size < required_size) {
            alloc_free_list_remove(alloc, merge_block);
            block = merge_mem_block(block, merge_block);

            merge_block = merge_block->next_phys;
        }

        if (required_size + block_header_size() < block->size) {
            mem_block* remainder = split_mem_block(block, required_size);
            alloc_free_list_merge_insert(alloc, remainder);
        }

        return block;
    }

    mem_block* new_block = allocator_alloc(alloc, requested_size);
    if (new_block == NULL) {
        return NULL;
    }

    void* new_block_data = mem_block_alloc(new_block);

    memcpy(new_block_data, mem_block_alloc(block), block->size);

    allocator_free(alloc, block);

    return new_block;
}

void allocator_free(allocator* alloc, mem_block* block) {
    if (alloc == NULL || block == NULL) {
        return;
    }

    if (!block->allocated) {
        return;
    }

    if (block->next_phys == NULL && block->prev_phys == NULL) {
        arena* a = (arena*)(((char*) block) - arena_header_size());
        arena_free(a);
    } else {
        alloc_free_list_merge_insert(alloc, block);
    }
}

