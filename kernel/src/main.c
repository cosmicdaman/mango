#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <mem.h>
#include <sys/gdt.h>
#include <ft/flanterm.h>
#include <ft/backends/fb.h>
#include <sys/gdt.h>
#include <sys/port.h>

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

void kmain(void) {
    if (!LIMINE_BASE_REVISION_SUPPORTED || !framebuffer_request.response || framebuffer_request.response->framebuffer_count < 1) hcf();
    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    struct flanterm_context *ftctx = flanterm_fb_init(
        NULL, NULL, 
        fb->address, fb->width, fb->height, fb->pitch, 
        fb->red_mask_size, fb->red_mask_shift, 
        fb->green_mask_size, fb->green_mask_shift, 
        fb->blue_mask_size, fb->blue_mask_shift, 
        NULL, 
        NULL, NULL, 
        NULL, NULL, 
        NULL, NULL, 
        NULL, 
        0, 0, 
        1, 0, 
        0, 0
    );

    flanterm_write(ftctx, "Welcome to \x1b[0;33mMangoOS\x1b[0m!\n");

    // Initialize the Global Descriptor Table
    initGDT();
    flanterm_write(ftctx, "[\x1b[1;32m OK \e[0m] Initialized GDT\n\n");

    // there is nothing left to do for now.
    flanterm_write(ftctx, "Hello, world!\n");
    hcf();
}
