#include <mm/pm.h>
#include <mem.h>
#include <limine.h>
#include <lib/printf.h>

__attribute__((used, section(".requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

uint8_t *pm_page_bitmap;
uint64_t pm_total_pages;
size_t pm_page_bitmap_size;

static void pm_page_bitmap_set_bit(uint64_t bit) {
    pm_page_bitmap[bit / 8] |= 1 << (bit % 8);
}

static void pm_page_bitmap_clear_bit(uint64_t bit) {
    pm_page_bitmap[bit / 8] &= ~(1 << (bit % 8));
}

static bool pm_page_bitmap_get_bit(uint64_t bit) {
    return pm_page_bitmap[bit / 8] & 1 << (bit % 8);
}

/// @brief Initialize a page bitmap for Physical Memory Management
void init_pmm() {
    struct limine_memmap_response *memmap = memmap_request.response;
    if (!memmap) {
        printf("pmm: \e[1;31merror\e[0m: failed to fetch memory map.");
        for (;;) {
            asm("hlt");
        }
    }

    uint64_t highest = 0;
    for (size_t i = 0; i < memmap->entry_count; i++) {
        if (memmap->entries[i]->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }
        uint64_t entry_top = memmap->entries[i]->base + memmap->entries[i]->length;
        if (entry_top > highest) {
            highest = entry_top;
        }
    }

    pm_total_pages = ((highest + (PAGE_SIZE - 1)) / PAGE_SIZE) * PAGE_SIZE; // round up
    pm_page_bitmap_size = pm_total_pages / 8;

    for (size_t i = 0; i < memmap->entry_count; i++) {
        if (memmap->entries[i]->type != LIMINE_MEMMAP_USABLE || memmap->entries[i]->length < pm_page_bitmap_size) {
            continue;
        }
        pm_page_bitmap = (uint8_t *)HIGHER_HALF(memmap->entries[i]->base);
        memmap->entries[i]->base += pm_page_bitmap_size;
        memmap->entries[i]->length -= pm_page_bitmap_size;
        memset(pm_page_bitmap, 0xff, pm_page_bitmap_size);
    }

    for (size_t i = 0; i < memmap->entry_count; i++) {
        if (memmap->entries[i]->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }
        for (size_t j = 0; j < memmap->entries[i]->length; j++) {
            pm_page_bitmap_clear_bit((memmap->entries[i]->base + j) / PAGE_SIZE);
        }
    }

    printf("pmm: page bitmap initialized at %p with %i pages.\n", pm_page_bitmap, pm_page_bitmap_size);
}

static void *find_pages(size_t s) {
    if (s > pm_total_pages || !s) {
        return NULL;
    }
    int pages = 0;
    size_t start = 0;
    for (size_t i = 0; i < pm_total_pages; i++) {
        if (!pm_page_bitmap_get_bit(i)) {
            if (!pages) {
                start = i;
            }
            pages++;
            if (pages == s) {
                return (void *)(start * PAGE_SIZE);
            }
        } else {
            pages = 0;
        }
    }
    return NULL;
}

/// @brief Allocate an amount of pages.
/// @param s The amount of pages to allocate.
/// @return The physical starting address to the pages that were allocated. Otherwise, NULL
void *pmm_alloc(size_t s) {
    void *pages = find_pages(s);
    if (!pages) {
        // try again
        pages = find_pages(s);
        if (!pages) {
            printf("pmm: \e[1;31merror\e[0m: not enough memory to allocate %i pages.\n", s);
            return NULL;
        }
        return pages;
    }
    return pages;
}

/// @brief Free an amount of pages.
/// @param p The physical starting address to the pages that were allocated.
/// @param s The amount of pages to free.
void pmm_free(void *p, size_t s) {
    uint64_t page = (uint64_t)p / PAGE_SIZE; // convert from physical address to page number
    for (size_t i = page; i < page + s; i++) {
        pm_page_bitmap_clear_bit(i);
    }
}