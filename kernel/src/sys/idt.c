#include <sys/idt.h>
#include <sys/port.h>

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

static void EOI(uint64_t vec) {
    if (vec <= 40) {
        outb(0xa0, 0x20);
    }
    outb(0x20, 0x20);
}

void kprint(const char *);
void kprintHex(uint8_t);
void kprintHex16(uint16_t);
void kprintHex64(uint64_t);

void handleInterrupt(struct int_frame *frame) {
    asm("cli");
    if (frame->vec <= 31) {
        kprint("\e[1;31mpanic\e[0m: ");
        kprint(exceptions[frame->vec]);
        kprint(", v=");
        kprintHex(frame->vec);
        kprint(", e=");
        kprintHex(frame->errcode);

        kprint("\n\trsp=");
        kprintHex16(frame->ss);
        kprint(":");
        kprintHex64(frame->rsp);

        kprint(", rip=");
        kprintHex16(frame->cs);
        kprint(":");
        kprintHex64(frame->rip);

        kprint(", rflags=");
        kprintHex64(frame->rflags);

        kprint("\n\trax=");
        kprintHex64(frame->rax);
        
        kprint(", rcx=");
        kprintHex64(frame->rcx);

        kprint(", rdx=");
        kprintHex64(frame->rdx);

        kprint("\n\trbx=");
        kprintHex64(frame->rbx);

        kprint(", rsi=");
        kprintHex64(frame->rsi);

        kprint(", rdi=");
        kprintHex64(frame->rdi);

        kprint("\n\trbp=");
        kprintHex64(frame->rbp);

        kprint(", r8 =");
        kprintHex64(frame->r8);

        kprint(", r9 =");
        kprintHex64(frame->r9);

        kprint("\n\tr10=");
        kprintHex64(frame->r10);

        kprint(", r11=");
        kprintHex64(frame->r11);

        kprint(", r12=");
        kprintHex64(frame->r12);
        
        kprint("\n\tr13=");
        kprintHex64(frame->r13);

        kprint(", r14=");
        kprintHex64(frame->r14);

        kprint(", r15=");
        kprintHex64(frame->r15);

        kprint("\nSystem halted.");

        for (;;) {
            asm("hlt");
        }
    } else {
        if (irq_handlers[frame->vec]) {
            irq_handlers[frame->vec](frame);
        } else {
            kprint("[\e[1;33m WARN \e[0m] Unhandled Interrupt (vec=0x");
            kprintHex((uint8_t)frame->vec);
            kprint(")\n");
        }
        EOI(frame->vec);
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

    setIRQMask(0b00000011, MASTER_PIC); // mask IRQ0 and IRQ1 to prevent the timer or keyboard from sending logs

    // create an IDT ptr
    idtr_t idtr = {
        .base_16_79 = (uint64_t)&idt,
        .limit_0_15 = sizeof(idt_gate_t) * MAX_IDT_GATES - 1
    };

    // load the IDT
    asm volatile("lidt %0"::"m"(idtr):"memory");
    asm volatile("sti");
}

void setIRQHandler(void (*handler)(struct int_frame *), int irq) {
    if (irq > MAX_IDT_GATES) return;
    irq_handlers[irq] = handler;
}

void setIRQMask(uint8_t mask, uint16_t pic) {
    switch (pic) {
        case MASTER_PIC:
            outb(MASTER_PIC_DATA, mask);
            break;
        case SLAVE_PIC:
            outb(SLAVE_PIC_DATA, mask);
            break;
        default:
            break;
    }
}