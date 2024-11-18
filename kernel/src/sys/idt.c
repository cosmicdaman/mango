#include <sys/idt.h>
#include <sys/port.h>

void kprint(const char *);
void kprintHex(uint8_t);
void kprintHex64(uint64_t);
void kclear();

static void (*irq_handlers[MAX_IDT_GATES])(struct int_frame *);

static const char *exceptions[31] = {
    "Division Error",
    "Debug Exception",
    "Non-Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "BOUND Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid Task State Segment",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 FPU Floating-Point Error",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
};

__attribute__((aligned(0x10))) static idt_gate_t idt[MAX_IDT_GATES];

void handleInterrupt(struct int_frame *frame) {
    asm("cli");
    if (frame->vec <= 31) {
        kprint("\e[38;2;255;255;255m\e[48;2;255;0;0m");
        kclear();
        kprint("An exception has occured, and MangoOS cannot continue.\n\n");

        kprint(exceptions[frame->vec]);
        if (frame->errcode) {
            kprint("\nError Code: 0x");
            kprintHex64(frame->errcode);
        }

        kprint("\n\nRegisters:\n");

        kprint("RSP = 0x");
        kprintHex((uint8_t)frame->ss);
        kprint(":");
        kprint("0x");
        kprintHex64(frame->rsp);

        kprint(" RFLAGS = 0x");
        kprintHex64(frame->rflags);

        kprint(" RIP = 0x");
        kprintHex((uint8_t)frame->cs);
        kprint(":");
        kprint("0x");
        kprintHex64(frame->rip);

        kprint("\nRAX = 0x");
        kprintHex64(frame->rax);
        kprint(" RCX = 0x");
        kprintHex64(frame->rcx);
        kprint(" RDX = 0x");
        kprintHex64(frame->rdx);
        kprint(" RBX = 0x");
        kprintHex64(frame->rbx);
        kprint("\nRSI = 0x");
        kprintHex64(frame->rsi);
        kprint(" RDI = 0x");
        kprintHex64(frame->rdi);
        kprint(" RBP = 0x");
        kprintHex64(frame->rbp);
        kprint(" R8 = 0x");
        kprintHex64(frame->r8);
        kprint("\nR9 = 0x");
        kprintHex64(frame->r9);
        kprint(" R10 = 0x");
        kprintHex64(frame->r10);
        kprint(" R11 = 0x");
        kprintHex64(frame->r11);
        kprint(" R12 = 0x");
        kprintHex64(frame->r12);
        kprint("\nR13 = 0x");
        kprintHex64(frame->r13);
        kprint(" R14 = 0x");
        kprintHex64(frame->r14);
        kprint(" R15 = 0x");
        kprintHex64(frame->r15);
        kprint("\n\nSystem halted.");
        for (;;) {
            asm("hlt");
        }
    } else {
        if (irq_handlers[frame->vec] != NULL) {
            irq_handlers[frame->vec](frame);
        } else {
            kprint("[\e[1;33m WARN \e[0m] Unhandled Interrupt (vec=0x");
            kprintHex((uint8_t)frame->vec);
            kprint(")\n");
        }

        if (frame->vec <= 40) {
            outb(0xa0, 0x20);
        }
        outb(0x20, 0x20);
        outb(0x20, 0x20);
    }
    asm("sti");
}

void setIDTGate(void (*handler)(), int gate, uint8_t type, uint8_t dpl) {
    if (gate > MAX_IDT_GATES) return;
    idt[gate].selector_16_31 = 0x28; // offset of the 64-bit code seg in the GDT.
    idt[gate].offset_0_15 = (uint64_t)handler & 0xffff;
    idt[gate].offset_48_63 = ((uint64_t)handler >> 16) & 0xffff;
    idt[gate].offset_64_95 = ((uint64_t)handler >> 32) & 0xffffffff;
    idt[gate].p_47 = 1;
    idt[gate].dpl_45_46 = dpl;
    idt[gate].type_40_43 = type;
}

void initIDT() {
    for (int i = 0; i < MAX_IDT_GATES; i++) {
        setIDTGate(int_handlers[i], i, 0xe, 0);
    }
    
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    // remap offsets
    outb(0x21, 32); // 20h
    outb(0xA1, 40); // 28h
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x00); 
    outb(0xA1, 0x00);

    // create an IDT ptr
    idtr_t idtr = {
        .base_16_79 = (uint64_t)&idt,
        .limit_0_15 = sizeof(idt_gate_t) * MAX_IDT_GATES - 1
    };

    // load the IDT
    asm volatile("lidt %0"::"m"(idtr):"memory");
    asm volatile("sti");
}

void setIRQHandler(void (*handler)(struct int_frame *), int irq, int remove) {
    if (irq > MAX_IDT_GATES) return;
    if (!remove) irq_handlers[irq] = handler;
    else irq_handlers[irq] = 0;
}