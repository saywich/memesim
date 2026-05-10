#ifndef MEMESIM_MEME_H_
#define MEMESIM_MEME_H_

#include <stddef.h>

typedef max_align_t MAX_ALIGN_TYPE;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void* ms_malloc_(size_t size);

void* ms_calloc_(size_t n, size_t size);

void* ms_realloc_(void* ptr, size_t size);

void ms_free_(void* ptr);

#ifdef __cplusplus
}
#endif // __cplusplus

#ifndef ms_malloc
#define ms_malloc(size) ms_malloc_(size)
#endif // ms_malloc

#ifndef ms_calloc
#define ms_calloc(n, size) ms_calloc_(n, size)
#endif // ms_calloc

#ifndef ms_realloc
#define ms_realloc(ptr, size) ms_realloc_(ptr, size)
#endif // ms_realloc

#ifndef ms_free
#define ms_free(ptr) ms_free_(ptr)
#endif // ms_free

#endif // MEMESIM_MEME_H_
