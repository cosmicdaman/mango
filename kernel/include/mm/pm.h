#pragma once
#include <stddef.h>
#include <lib/global.h>

#define PAGE_SIZE 4096 // 4 KB
#define HIGHER_HALF(x) x + hhdm_offset

void init_pmm();
void *pmm_alloc(size_t s);
void pmm_free(void *p, size_t s);