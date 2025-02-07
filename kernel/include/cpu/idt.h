#pragma once
#include <stdint.h>
#include <stddef.h>

#define MAX_IDT_GATES 256

#define MASTER_PIC 0x20
#define MASTER_PIC_DATA MASTER_PIC + 1
#define SLAVE_PIC 0xa0
#define SLAVE_PIC_DATA SLAVE_PIC + 1

extern void *int_handlers[];

struct int_frame {
    uint64_t cr3;
    uint64_t cr2;
    uint64_t cr0;
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rbx;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rax;
    uint64_t vec;
    uint64_t errcode;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed));

typedef struct {
    uint16_t limit_0_15;
    uint64_t base_16_79;
} __attribute__((packed)) idtr_t;

typedef struct {
    uint16_t offset_0_15;
    uint16_t selector_16_31;
    uint8_t ist_32_34 : 3;
    uint8_t reserved_35_39 : 5;
    uint8_t type_40_43 : 4;
    uint8_t zero_44 : 1;
    uint8_t dpl_45_46 : 2;
    uint8_t p_47 : 1;
    uint16_t offset_48_63;
    uint32_t offset_64_95;
    uint32_t reserved_96_127;
} __attribute__((packed)) idt_gate_t;

void set_idt_gate(void (*handler)(), int gate, uint8_t type, uint8_t dpl);
void init_idt();

void set_irq_handler(void (*handler)(struct int_frame *), int irq);
void set_pic_irq_mask(uint8_t mask, uint16_t pic);
