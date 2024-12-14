#pragma once
#include <stdint.h>
#include <limine.h>

typedef struct {
    char signature[8];
    uint8_t checksum;
    char OEMID[6];
    uint8_t revision;
    uint32_t rsdtAddr; // ignored
    uint32_t len;
    uint64_t xsdtAddr;
    uint8_t extchecksum;
    uint8_t reserved[8];
} __attribute__((packed)) xsdp_t;

/*typedef struct { // SDT header
    char signature[4];
    uint32_t len;
    uint8_t revision;
    uint8_t checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t creatorID;
    uint32_t creatorRevision;
} h_SDT;

typedef struct {
    h_SDT SDTHeader;
    uint64_t nextSDT[(SDTHeader.len - sizeof(SDTHeader)) / 8];
} xsdt_t;*/


void findXSDT(struct limine_rsdp_response *rsdp_response);