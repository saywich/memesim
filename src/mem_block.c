#include "include/mem_block.h"

size_t align_up(size_t n, size_t alignment) {
    size_t remainder = n % alignment;

    if (remainder == 0) {
        return n;
    }

    return n + (alignment - remainder);
}

size_t block_header_size() {
    return align_up(sizeof(mem_block), _Alignof(MAX_ALIGN_TYPE));
}

mem_block* make_mem_block(mem_block* ptr, size_t size) {
    mem_block block = {
        .size = size,
        .allocated = false,
        .prev_free = NULL,
        .next_free = NULL,
        .prev_phys = NULL,
        .next_phys = NULL,
    };

    *ptr = block;
    
    return ptr;
}

mem_block* merge_mem_block(mem_block* lhs, mem_block* rhs) {
    lhs->size += block_header_size() + rhs->size;
    lhs->next_phys = rhs->next_phys;

    if (rhs->next_phys != NULL) {
        rhs->next_phys->prev_phys = lhs;
    }

    return lhs;
}

mem_block* split_mem_block(mem_block* ptr, size_t requested_size) {
    size_t header_size = block_header_size();
    size_t required_size = align_up(requested_size, _Alignof(MAX_ALIGN_TYPE));

    mem_block* new_block = (mem_block*)(((char*) ptr) + header_size + required_size);
    new_block->size = ptr->size - required_size - header_size;
    new_block->allocated = false;

    new_block->prev_phys = ptr;
    new_block->next_phys = ptr->next_phys;

    if (new_block->next_phys != NULL) {
        new_block->next_phys->prev_phys = new_block;
    }

    ptr->size = required_size;
    ptr->next_phys = new_block;
    
    return new_block;
}

void* mem_block_alloc(mem_block* ptr) {
    ptr->allocated = true;
    return (void*)(((char*)ptr) + align_up(sizeof(mem_block), _Alignof(MAX_ALIGN_TYPE)));
}
