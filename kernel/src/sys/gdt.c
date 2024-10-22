#include <sys/gdt.h>

static GDT gdt = {
    // gdt.segments[0] = null
    // gdt.segments[1] = 16-bit code segment
    // gdt.segments[2] = 16-bit data segment
    // gdt.segments[3] = 32-bit code segment
    // gdt.segments[4] = 32-bit data segment
    // gdt.segments[5] = 64-bit code segment (CS = 0x28)
    // gdt.segments[6] = 64-bit data segment (DS = 0x30, ES = 0x30, FS = 0x30, GS = 0x30, SS = 0x30)
    // gdt.segments[7] = User Mode data segment
    // gdt.segments[8] = User Mode code segment 
    { 0x0000000000000000, 0x00009A000000FFFF, 0x000093000000FFFF, 
      0x00CF9A000000FFFF, 0x00CF93000000FFFF, 0x00AF9B000000FFFF, 
      0x00AF93000000FFFF, 0x00AFF3000000FFFF, 0x00AFFB000000FFFF },

    // gdt.tssSeg = GDT segment for the Task State Segment. Value is set in initGDT()
    {}
};

/// @brief Initialize the Global Descriptor Table.
void initGDT() {
    // create the TSS Segment in the GDT
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

    // make a GDT ptr
    gdtr_t gdtr;
    gdtr.base_16_79 = (uint64_t)&gdt;
    gdtr.limit_0_15 = sizeof(gdt) - 1;

    // load the GDT (and TSS)
    asm volatile("lgdt %0\n\t"::"m"(gdtr):"memory");
    asm volatile("ltr %0"::"r"((uint16_t)0x48):"memory");
}