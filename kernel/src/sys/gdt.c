#include <sys/gdt.h>

static GDT gdt;

static void setupSegments() {
    // offset = 0x00 : null segment
    // it is already nulled / zeroed out.

    // offset = 0x08 : kernel mode code segment
    gdt.kCode.limit_0_15 = 0xFFFF;
    gdt.kCode.limit_48_51 = 0xF;
    gdt.kCode.base_16_31 = 0;
    gdt.kCode.base_32_39 = 0;
    gdt.kCode.base_56_63 = 0;
    gdt.kCode.access_40_47 = 0x9A;
    gdt.kCode.flags_52_55 = 0xA;

    // offset = 0x10 : kernel mode data segment
    gdt.kData.limit_0_15 = 0xFFFF;
    gdt.kData.limit_48_51 = 0xF;
    gdt.kData.base_16_31 = 0;
    gdt.kData.base_32_39 = 0;
    gdt.kData.base_56_63 = 0;
    gdt.kData.access_40_47 = 0x92;
    gdt.kData.flags_52_55 = 0xC;

    // offset = 0x18 : user mode code segment
    gdt.uCode.limit_0_15 = 0xFFFF;
    gdt.uCode.limit_48_51 = 0xF;
    gdt.uCode.base_16_31 = 0;
    gdt.uCode.base_32_39 = 0;
    gdt.uCode.base_56_63 = 0;
    gdt.uCode.access_40_47 = 0xFA;
    gdt.uCode.flags_52_55 = 0xA;

    // offset = 0x20 : user mode data segment
    gdt.uData.limit_0_15 = 0xFFFF;
    gdt.uData.limit_48_51 = 0xF;
    gdt.uData.base_16_31 = 0;
    gdt.uData.base_32_39 = 0;
    gdt.uData.base_56_63 = 0;
    gdt.uData.access_40_47 = 0xF2;
    gdt.uData.flags_52_55 = 0xC;

    // offset = 0x28 : task state segment
    // this is a bit different than the other segments, because it uses a system segment descriptor
    // rather than a normal segment descriptor. we need to use this as the TSS has a 64-bit base.
    tss_t tss;
    uint64_t tssBase = (uint64_t)&tss;
    gdt.tssSeg.limit_0_15 = sizeof(tss);
    gdt.tssSeg.limit_48_51 = 0;
    gdt.tssSeg.base_16_31 = tssBase & 0xFFFF;
    gdt.tssSeg.base_32_39 = (tssBase >> 16) & 0xFF;
    gdt.tssSeg.base_56_63 = (tssBase >> 24) & 0xFF;
    gdt.tssSeg.base_64_95 = tssBase >> 32;
    gdt.tssSeg.access_40_47 = 0x89;
    gdt.tssSeg.flags_52_55 = 0x0;
}

/// @brief Initialize the Global Descriptor Table.
void initGDT() {
    // setup the segments.
    setupSegments();

    // make a GDT ptr
    gdtr_t gdtr;
    gdtr.base = (uint64_t)&gdt;
    gdtr.limit = sizeof(gdt) - 1;

    // load the GDT (and TSS)
    asm volatile("lgdt %0\n\t"::"m"(gdtr):"memory");
    asm volatile("ltr %0\n\t"::"r"((uint16_t)0x28):"memory");
}