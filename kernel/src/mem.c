#include <mem.h>

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

static uint8_t dmem_area[10*1024*1024]; // 10 MiB
static mem_chunk_t *start;

/// @brief Initializes the Dynamic Memory Management
void initDMM() {
    start = (mem_chunk_t *)&dmem_area;
    start->alloc = false;
    start->last = NULL;
    start->next = NULL;
    start->size = ((10*1024*1024) - sizeof(mem_chunk_t));
}

/// @brief Allocate a specific number of bytes from dynamic memory.
/// @param size The number of bytes to allocate.
/// @return The pointer to the new array, otherwise NULL.
void *malloc(size_t size) {
    mem_chunk_t *ptr = NULL;
    for (mem_chunk_t *chnk = start; chnk && !ptr; chnk = chnk->next) {
        if (chnk->size > size && !chnk->alloc) {
            ptr = chnk;
        }
    }

    if (!ptr) {
        return NULL;
    }

    if (ptr->size >= size + sizeof(mem_chunk_t) + 1) {
        mem_chunk_t *tmp = (mem_chunk_t *)((size_t)ptr + sizeof(mem_chunk_t) + size);
        tmp->alloc = false;
        tmp->size = ptr->size - size - sizeof(mem_chunk_t);
        tmp->last = ptr;
        tmp->next = ptr->next;
        if (tmp->next) {
            tmp->next->last = tmp;
        }

        ptr->size = size;
        ptr->next = tmp;
    }

    ptr->alloc = true;
    return (void *)(((size_t)ptr) + sizeof(mem_chunk_t));
}

/// @brief Frees an allocated array of bytes from dynamic memory.
/// @param ptr The pointer to the allocated array.
void free(void *ptr) {
    mem_chunk_t *chnk = (mem_chunk_t *)((size_t)ptr - sizeof(mem_chunk_t));
    chnk->alloc = false;

    if (chnk->last && !chnk->last->alloc) {
        chnk->last->next = chnk->next;
        chnk->last->size += chnk->size + sizeof(mem_chunk_t);
        if (chnk->next) {
            chnk->next->last = chnk->last;
        }
        chnk = chnk->last;
    }

    if (chnk->next && chnk->next->alloc) {
        chnk->size += chnk->next->size + sizeof(mem_chunk_t);
        chnk->next = chnk->next->next;
        if (chnk->next) {
            chnk->next->last = chnk;
        }
    }
}

/// @brief Allocates an array with a total size of n*size, and initializes the elements to 0
/// @param n The number of elements
/// @param size The size of the elements
/// @return The pointer to the array. Otherwise, NULL
void *calloc(size_t n, size_t size) {
    size_t total = n * size;
    void *ptr = malloc(total);
    if (!ptr) {
        return NULL;
    }
    memset(ptr, 0, total);
    return ptr;
}
