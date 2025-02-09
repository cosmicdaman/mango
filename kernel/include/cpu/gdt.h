#pragma once
#include <stdint.h>

typedef struct {
    uint16_t limit_0_15;
    uint64_t base_16_79;
} __attribute__((packed)) gdtr_t;

typedef struct {
    uint16_t limit_0_15;
    uint16_t base_16_31;
    uint8_t base_32_39;
    uint8_t access_40_47;
    uint8_t limit_48_51 : 4;
    uint8_t flags_52_55 : 4;
    uint8_t base_56_63;
    uint32_t base_64_95;
    uint32_t reserved_96_127;
} __attribute__((packed)) system_segment_t; // 64-bit system segment descriptor

typedef struct {
    uint64_t segments[9];
    system_segment_t tssSeg;
} __attribute__((packed)) GDT;

typedef struct {
    uint32_t resv;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t resv1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t resv2;
    uint16_t resv3;
    uint16_t iopb;
} __attribute__((packed)) tss_t; // task state segment

void init_gdt();