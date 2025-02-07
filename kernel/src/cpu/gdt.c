#include <cpu/gdt.h>

static GDT gdt = {
    { // null, 16-bit cs & ds, 32-bit cs & ds, 64-bit cs & ds, dpl 3 cs & ds
      0x0000000000000000, 0x00009a000000ffff, 0x000093000000ffff, 
      0x00cf9a000000ffff, 0x00cf93000000ffff, 0x00af9b000000ffff, 
      0x00af93000000ffff, 0x00aff3000000ffff, 0x00affb000000ffff 
    },

    // gdt.tssSeg = GDT segment for the Task State Segment. Value is set in initGDT()
    {
    }
};

/// @brief Initialize the Global Descriptor Table.
void init_gdt() {
    // create the TSS Segment in the GDT
    tss_t tss;
    uint64_t tssBase = (uint64_t)&tss;
    gdt.tssSeg.limit_0_15 = sizeof(tss);
    gdt.tssSeg.limit_48_51 = 0;
    gdt.tssSeg.base_16_31 = tssBase & 0xffff;
    gdt.tssSeg.base_32_39 = (tssBase >> 16) & 0xff;
    gdt.tssSeg.base_56_63 = (tssBase >> 24) & 0xff;
    gdt.tssSeg.base_64_95 = tssBase >> 32;
    gdt.tssSeg.access_40_47 = 0x89;
    gdt.tssSeg.flags_52_55 = 0x0;

    // make a GDT ptr
    gdtr_t gdtr;
    gdtr.base_16_79 = (uint64_t)&gdt;
    gdtr.limit_0_15 = sizeof(gdt) - 1;

    // load the GDT (and TSS)
    asm volatile("lgdt %0"::"m"(gdtr):"memory");
    asm volatile("ltr %0"::"r"((uint16_t)0x48):"memory");
}