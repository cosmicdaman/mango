#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct MemChnk {
    struct MemChnk *next;
    struct MemChnk *last;
    bool alloc;
    size_t size;
} __attribute__((packed)) mem_chunk_t;

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *malloc(size_t size);
void free(void *ptr);
int memcmp(const void *s1, const void *s2, size_t n);

void initDMM();