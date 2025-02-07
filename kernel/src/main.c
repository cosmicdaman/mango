// essentials
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <mem.h>

// mm
#include <mm/pm.h>

// cpu
#include <cpu/gdt.h>
#include <cpu/idt.h>

// lib
#include <lib/printf.h>
#include <lib/global.h>

// flanterm
#include <ft/flanterm.h>
#include <ft/backends/fb.h>

__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

// stuff for global.h
struct limine_framebuffer *fb;
struct flanterm_context *ftctx;
uint64_t hhdm_offset;

// Halt and catch fire function.
void hcf(void) {
    for (;;) {
        asm("hlt");
    }
}

extern uint8_t _binary_kernel_res_ttyfont_start; // ttyfont
void kmain(void) {
    if (!LIMINE_BASE_REVISION_SUPPORTED 
    || !framebuffer_request.response 
    || !framebuffer_request.response->framebuffer_count) {
        hcf();
    } 

    fb = framebuffer_request.response->framebuffers[0];
    hhdm_offset = hhdm_request.response->offset;
    ftctx = flanterm_fb_init (
        NULL, NULL, 
        fb->address, fb->width, fb->height, fb->pitch, 
        fb->red_mask_size, fb->red_mask_shift, 
        fb->green_mask_size, fb->green_mask_shift, 
        fb->blue_mask_size, fb->blue_mask_shift, 
        NULL, 
        NULL, NULL, 
        NULL, NULL, 
        NULL, NULL, 
        &_binary_kernel_res_ttyfont_start, 
        8, 16, 
        0, 0,
        0, 0
    );

    init_gdt(); // Initialize the Global Descriptor Table
    init_idt(); // Initialize the Interrupt Descriptor Table
    init_pmm(); // Initialize the Physical Memory Manager

    printf("Initialization successful.\n");

    printf("Welcome to \e[0;33mMangoOS\e[0m!\n");

    // there is nothing left to do for now.
    printf("Hello, world!\n");

    hcf();
}
