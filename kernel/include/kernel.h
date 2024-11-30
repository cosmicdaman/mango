#pragma once
#include "ft/flanterm.h"
#include "ft/backends/fb.h"

struct limine_framebuffer *fb;

struct flanterm_context *ftctx;

void kprint(const char *str) {
    flanterm_write(ftctx, str);
}

#define LOG_OK 0
#define LOG_WARN 1
#define LOG_ERROR 2
#define LOG_INFO 3

void klog(int type, const char *msg) {
    switch (type) {
        case LOG_OK:
            kprint("[\e[1;32m OK \e[0m] ");
            kprint(msg);
            kprint("\n");
            break;
        case LOG_WARN:
            kprint("[\e[1;33m WARN \e[0m] ");
            kprint(msg);
            kprint("\n");
            break;
        case LOG_ERROR:
            kprint("[\e[1;31m ERROR \e[0m] ");
            kprint(msg);
            kprint("\n");
            break;
        case LOG_INFO:
            kprint("[\e[1;34m INFO \e[0m] ");
            kprint(msg);
            kprint("\n");
            break;
        default:
            break;
    }
}

void kprintHex(uint8_t i) {
    char buf[3];
    char *hex = "0123456789abcdef";
    buf[0] = hex[(i >> 4) & 0xf];
    buf[1] = hex[i & 0xf];
    buf[2] = '\0';
    kprint(&buf);
}

void kprintHex16(uint16_t i) {
    kprintHex((i >> 8) & 0xff);
    kprintHex(i & 0xff);
}

void kprintHex32(uint32_t i) {
    kprintHex16((i >> 16) & 0xffff);
    kprintHex16(i & 0xffff);
}

void kprintHex64(uint64_t i) {
    kprintHex32((i >> 32) & 0xffffffff);
    kprintHex32(i & 0xffffffff);
}