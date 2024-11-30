![logo](https://raw.githubusercontent.com/cosmicdaman/mango/refs/heads/master/.art/logo.webp)
# mango
mango (or mangoOS) is a 64-bit kernel that uses Limine & the Limine Boot Protocol.
> [!WARNING]
> **Mango is still in development. Contents are subject to change in the near future.**
## Planned Features
- an EXT2 driver.
- ELF loading. (probably UNIX)
- PS/2 keyboard and mouse drivers.
- small GUI
## Roadmap

### Base System
- [X] Global Descriptor Table
- [X] Interrupts
- [ ] Dynamic Memory Management / Heap
- [ ] ACPI
- [ ] Multicore
- [ ] Switch to APIC
- [ ] Timer
- [ ] Thread Scheduler
- [ ] Virtual Filesystem 
- [ ] Ring 3, ELF loading and Syscalls
- [ ] PCI Driver
- [ ] NVMe Driver

### Userland
- [ ] PS/2 Driver
- [ ] msh (the mangoOS shell)
- [ ] small GUI
