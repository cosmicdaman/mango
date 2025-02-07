#pragma once
#include <stdint.h>

/// @brief Write an 8-bit value to an I/O port
static inline void outb(uint16_t p, uint8_t b) {
    asm volatile("outb %0, %1\n\t"::"a"(b),"Nd"(p):"memory");
}

/// @brief Read an 8-bit value from an I/O port
static inline uint8_t inb(uint16_t p) {
    uint8_t ret;
    asm volatile("inb %1, %0\n\t":"=a"(ret):"Nd"(p):"memory");
    return ret;
}

/// @brief Write a 16-bit value to an I/O port
static inline void outw(uint16_t p, uint16_t w) {
    asm volatile("outw %0, %1\n\t"::"a"(w),"Nd"(p):"memory");
}

/// @brief Read a 16-bit value from an I/O port
static inline uint8_t inw(uint16_t p) {
    uint16_t ret;
    asm volatile("inw %1, %0\n\t":"=a"(ret):"Nd"(p):"memory");
    return ret;
}

/// @brief Write a 32-bit value to an I/O port
static inline void outl(uint16_t p, uint32_t l) {
    asm volatile("outl %0, %1\n\t"::"a"(l),"Nd"(p):"memory");
}

/// @brief Read a 32-bit value from an I/O port
static inline uint32_t inl(uint16_t p) {
    uint32_t ret;
    asm volatile("inl %1, %0\n\t":"=a"(ret):"Nd"(p):"memory");
    return ret;
}