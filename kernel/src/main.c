#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include <mem.h>
#include <sys/gdt.h>
#include <sys/idt.h>
#include <sys/port.h>

#include <kernel.h>

__attribute__((used, section(".requests")))
static volatile LIMINE_BASE_REVISION(2);

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".requests_start_marker")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
static volatile LIMINE_REQUESTS_END_MARKER;

// Halt and catch fire function.
static void hcf(void) {
    for (;;) {
        asm("hlt");
    }
}

extern uint8_t _binary_VGA_F16_start; // ttyfont
void kmain(void) {
    if (!LIMINE_BASE_REVISION_SUPPORTED 
    || !framebuffer_request.response 
    || !framebuffer_request.response->framebuffer_count) {
        hcf();
    } 
    fb = framebuffer_request.response->framebuffers[0];

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
        &_binary_VGA_F16_start, 
        8, 16, 
        0, 0, 
        0, 0
    );

    kprint("Welcome to \e[0;33mMangoOS\e[0m!\n");

    // Initialize the Global Descriptor Table
    initGDT();
    klog(LOG_OK, "Initialized GDT");

    // Initialize the Dynamic Memory (management)
    initDMM();
    klog(LOG_OK, "Initialized Dynamic Memory");

    // Initialize the Interrupt Descriptor Table
    initIDT();
    klog(LOG_OK, "Initialized IDT");

    // there is nothing left to do for now.
    kprint("Hello, world!\n");

    hcf();
}
