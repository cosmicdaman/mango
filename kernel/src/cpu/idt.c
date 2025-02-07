#include <cpu/idt.h>
#include <cpu/port.h>

#include <lib/printf.h>

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
    "Reserved (int 0x0f)",
    "x87 FPU Floating-Point Error",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved (int 0x16)",
    "Reserved (int 0x17)",
    "Reserved (int 0x18)",
    "Reserved (int 0x19)",
    "Reserved (int 0x1a)",
    "Reserved (int 0x1b)",
    "Reserved (int 0x1c)",
    "Reserved (int 0x1d)",
    "Reserved (int 0x1e)",
};

__attribute__((aligned(0x10))) static idt_gate_t idt[MAX_IDT_GATES];
static void (*irq_handlers[MAX_IDT_GATES])(struct int_frame *);

static void pic_eoi(uint64_t vec) {
    if (vec <= 40) {
        outb(SLAVE_PIC, 0x20);
    }
    outb(MASTER_PIC, 0x20);
}

static void pic_init(uint8_t offs1, uint8_t offs2, uint8_t mask_mpic, uint8_t mask_spic) {
    outb(MASTER_PIC, 0x11); // ICW1_INIT | ICW1_ICW4
    outb(SLAVE_PIC, 0x11);
    outb(MASTER_PIC_DATA, offs1);
    outb(SLAVE_PIC_DATA, offs2);
    outb(MASTER_PIC_DATA, 0x04);
    outb(SLAVE_PIC_DATA, 0x02);
    outb(MASTER_PIC_DATA, 0x01); // ICW4_8086
    outb(SLAVE_PIC_DATA, 0x01); 
    outb(MASTER_PIC_DATA, mask_mpic); 
    outb(SLAVE_PIC_DATA, mask_spic);
}

void handle_int(struct int_frame *frame) {
    asm volatile("cli");
    if (frame->vec <= 31) {
        printf("\e[1;31mpanic\e[0m: %s (v=%x, e=%x)\n", exceptions[frame->vec], frame->vec, frame->errcode);
        printf("register dump:\n\trip=%x:%lx, rsp=%lx:%lx\n", frame->cs, frame->rip, frame->ss, frame->rsp);
        printf("\trax=%lx, rcx=%lx, rdx=%lx\n", frame->rax, frame->rcx, frame->rdx);
        printf("\trbx=%lx, rsi=%lx, rdi=%lx\n", frame->rbx, frame->rsi, frame->rdi);
        printf("\trbp=%lx, r8 =%lx, r9 =%lx\n", frame->rbp, frame->r8, frame->r9);
        printf("\tr10=%lx, r11=%lx, r12=%lx\n", frame->r10, frame->r11, frame->r12);
        printf("\tr13=%lx, r14=%lx, r15=%lx\n");
        printf("\tcr0=%lx, ", frame->cr0);
        
        if (frame->vec == 0x0e) {
            printf("\tpage fault address (cr2) = %lx, ", frame->cr2);
        }

        printf("cr3=%lx", frame->cr3);

        for (;;) {
            asm volatile("hlt");
        }
    } else {
        if (irq_handlers[frame->vec]) {
            irq_handlers[frame->vec](frame);
        } else {
            printf("ints: unhandled irq! (v=%x)", frame->vec);
        }
        pic_eoi(frame->vec);
    }
    asm volatile("sti");
}

/// @brief Set an IDT gate from the IDT.
/// @param handler A pointer to the interrupt service routine.
/// @param gate Which gate to set (index)
/// @param type Gate type attribute.
/// @param dpl Descriptor Privilege Level attribute.
void set_idt_gate(void (*handler)(), int gate, uint8_t type, uint8_t dpl) {
    if (gate > MAX_IDT_GATES) return;
    idt[gate].selector_16_31 = 0x28; // offset of the 64-bit code seg in the GDT.
    idt[gate].offset_0_15 = (uint64_t)handler & 0xffff;
    idt[gate].offset_48_63 = ((uint64_t)handler >> 16) & 0xffff;
    idt[gate].offset_64_95 = ((uint64_t)handler >> 32) & 0xffffffff;
    idt[gate].p_47 = 1;
    idt[gate].dpl_45_46 = dpl;
    idt[gate].type_40_43 = type;
}

/// @brief Initialize the Interrupt Descriptor Table (and remap the Programmable Interrupt Controller)
void init_idt() {
    for (size_t i = 0; i < MAX_IDT_GATES; i++) {
        set_idt_gate(int_handlers[i], i, 0xe, 0);
    }
    
    pic_init(0x20, 0x28, 0b00000011, 0x0);

    // create an IDT ptr
    idtr_t idtr = {
        .base_16_79 = (uint64_t)&idt,
        .limit_0_15 = sizeof(idt_gate_t) * MAX_IDT_GATES - 1
    };

    // load the IDT
    asm volatile("lidt %0"::"m"(idtr):"memory");
    asm volatile("sti");
}

void set_irq_handler(void (*handler)(struct int_frame *), int irq) {
    if (irq > MAX_IDT_GATES) return;
    irq_handlers[irq] = handler;
}

void set_pic_irq_mask(uint8_t mask, uint16_t pic) {
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