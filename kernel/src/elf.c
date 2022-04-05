#include "elf.h"

#include <string.h>

#include "kstdio.h"
#include "page.h"

/* Program header */
#define PT_NULL 0
#define PT_LOAD 1

/* These constants define the permissions on sections in the program
   header, p_flags. */
#define PF_R 0x4
#define PF_W 0x2
#define PF_X 0x1

// Source: linux/include/uapi/linux/elf.h
// 64-bit ELF base types
typedef uint64_t Elf64_Addr;
typedef uint16_t Elf64_Half;
typedef int16_t Elf64_SHalf;
typedef uint64_t Elf64_Off;
typedef int32_t Elf64_Sword;
typedef uint32_t Elf64_Word;
typedef uint64_t Elf64_Xword;
typedef int64_t Elf64_Sxword;

typedef struct elf_header {
    unsigned char e_ident[16]; /* ELF identification */
    Elf64_Half e_type;         /* Object file type */
    Elf64_Half e_machine;      /* Machine type */
    Elf64_Word e_version;      /* Object file version */
    Elf64_Addr e_entry;        /* Entry point address */
    Elf64_Off e_phoff;         /* Program header offset */
    Elf64_Off e_shoff;         /* Section header offset */
    Elf64_Word e_flags;        /* Processor-specific flags */
    Elf64_Half e_ehsize;       /* ELF header size */
    Elf64_Half e_phentsize;    /* Size of program header entry */
    Elf64_Half e_phnum;        /* Number of program header entries */
    Elf64_Half e_shentsize;    /* Size of section header entry */
    Elf64_Half e_shnum;        /* Number of section header entries */
    Elf64_Half e_shstrndx;     /* Section name string table index */
} elf_header_t;

typedef struct elf_program {
    Elf64_Word p_type;    /* Type of segment */
    Elf64_Word p_flags;   /* Segment attributes */
    Elf64_Off p_offset;   /* Offset in file */
    Elf64_Addr p_vaddr;   /* Virtual address in memory */
    Elf64_Addr p_paddr;   /* Reserved */
    Elf64_Xword p_filesz; /* Size of segment in file */
    Elf64_Xword p_memsz;  /* Size of segment in memory */
    Elf64_Xword p_align;  /* Alignment of segment */
} elf_program_t;

void_function_t load(uintptr_t p, size_t size) {
    elf_header_t *header = (elf_header_t *)(p);
    elf_program_t *program = p + header->e_phoff;

    // iterate and load program segment to memory
    for (size_t i = 0; i < header->e_phnum; i++) {
        if (program[i].p_type != PT_LOAD) {  // not loadable
            continue;
        }
        if (program[i].p_filesz == 0 || program[i].p_memsz == 0) {  // no size
            continue;
        }

        uintptr_t src = p + program[i].p_offset;  // source of the program segment
        uintptr_t dest = program[i].p_vaddr;      // virutal address destination
        bool executable = program[i].p_flags & PF_X;
        bool writable = program[i].p_flags & PF_W;
        bool readable = program[i].p_flags & PF_R;

        // prepare the page (by allocate enough space and page-aligned it)
        for (uintptr_t begin = (dest & 0xFFFFFFFFFFFFF000); begin < dest + program[i].p_memsz;
             begin += PAGE_SIZE) {
            if (!vm_map(read_cr3(), begin, true, true, true)) {
                kprintf("vm_map failed!\n");
            }
        }

        // copy program to position
        memcpy(dest, src, program[i].p_memsz);

        for (uintptr_t begin = (dest & 0xFFFFFFFFFFFFF000); begin < dest + program[i].p_memsz;
             begin += PAGE_SIZE) {
            if (!vm_protect(read_cr3(), begin, readable, writable, executable)) {
                kprintf("vm_protect failed!\n");
            }
        }

        kprintf("type: %d  vaddr: %p fsize: %d msize: %d offset: %d\n", program[i].p_type,
                program[i].p_vaddr, program[i].p_filesz, program[i].p_memsz, program[i].p_offset);
    }

    return header->e_entry;
}