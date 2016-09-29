#ifndef __ELF_H__
#define __ELF_H__

#include <stddef.h>
#include <stdint.h>

#define PHDR_MAX 4

#define EI_NIDENT   16
#define EI_MAG0     0x7f
#define EI_MAG1     'E'
#define EI_MAG2     'L'
#define EI_MAG3     'F'
#define PT_LOAD     1
#define PF_R        4
#define PF_W        2
#define PF_X        1

/*
 *  ELF32
 */
typedef uint32_t  Elf32_Addr;
typedef uint32_t  Elf32_Off;

typedef struct{
    uint8_t     e_ident[EI_NIDENT];
    uint16_t    e_type;
    uint16_t    e_machine;
    uint32_t    e_version;
    Elf32_Addr  e_entry;
    Elf32_Off   e_phoff;
    Elf32_Off   e_shoff;
    uint32_t    e_flags;
    uint16_t    e_ehsize;
    uint16_t    e_phentsize;
    uint16_t    e_phnum;
    uint16_t    e_shentsize;
    uint16_t    e_shnum;
    uint16_t    e_shstrndx;
} Elf32_Ehdr;

typedef struct {
    uint32_t    p_type;
    Elf32_Off   p_offset;
    Elf32_Addr  p_vaddr;
    Elf32_Addr  p_paddr;
    uint32_t    p_filesz;
    uint32_t    p_memsz;
    uint32_t    p_flags;
    uint32_t    p_align;
} Elf32_Phdr;

typedef struct {
    uint32_t    sh_name;
    uint32_t    sh_type;
    uint32_t    sh_flags;
    Elf32_Addr  sh_addr;
    Elf32_Off   sh_offset;
    uint32_t    sh_size;
    uint32_t    sh_link;
    uint32_t    sh_info;
    uint32_t    sh_addralign;
    uint32_t    sh_entsize;
} Elf32_Shdr;

/*
 *  functions
 */
uintptr_t load_elf(const void *data, void* (*map)(void *dst, const void *src, size_t size));

#endif
