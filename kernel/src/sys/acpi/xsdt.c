#include <sys/acpi/xsdt.h>

void findXSDT(struct limine_rsdp_response *rsdp_response) {
    xsdp_t *xsdp = (xsdp_t *)rsdp_response->address;
    
}