#include <Arduino.h>
#include "Esp.h"
#include "consts.h"
#include "elf.h"
#include "http.h"
#include "aligned_memcpy.h"

// Returns 1 on success or 0 on failure.
uintptr_t load_elf(int deployment_id) {
    Elf32_Ehdr *ehdr;
    Elf32_Phdr *phdr;
    uint8_t *data = (uint8_t *) 0x3fff9000; // XXX
    size_t data_size = 1024; // XXX

    ehdr = (Elf32_Ehdr *) data;
    String path("/api/devices/");
    path.concat(DEVICE_SECRET);
    path.concat("/image?deployment_id=");
    path.concat(String(deployment_id));

    int offset = 0;
    while (offset < 1024) { // XXX
        do_http_request(SERVER_HOST, SERVER_PORT, "GET", path.c_str(),
                        "", "", 0, &data, &data_size, &offset, SERVER_TLS);
    }

    data = (uint8_t *) 0x3fff9000; // XXX

    Serial.println("elf: cheking elf magic");
    if (ehdr->e_ident[0] != EI_MAG0 || ehdr->e_ident[1] != EI_MAG1 ||
        ehdr->e_ident[2] != EI_MAG2 || ehdr->e_ident[3] != EI_MAG3) {

        Serial.println("elf: invalid magic");
        return 0;
    }

    if (ehdr->e_phoff == 0) {

        Serial.println("elf: e_phoff is null");
        return 0;
    }

    // flash where the init.elf exists allows only 32-bit accesses
    // TODO: clean up
    volatile uint32_t* p = (uint32_t *) &ehdr->e_phnum;
    int e_phnum = *p & 0xffff;

    p = (uint32_t *) ((uintptr_t) &ehdr->e_phentsize - sizeof(uint16_t));
    int e_phentsize = (*p >> 16) & 0xffff;

    uint32_t e_phoff = ehdr->e_phoff;

    Serial.println("elf: loading program headers");

    // load program headers
    for (int i=0; i < e_phnum && i < PHDR_MAX; i++) {
        data = (uint8_t *) 0x3fff9000; // XXX
        phdr = (Elf32_Phdr *) ((uintptr_t)  data + e_phoff + (e_phentsize * i));

        if (phdr->p_type == PT_LOAD) {
            uint32_t *dst = (uint32_t *) phdr->p_vaddr;
            uint32_t size = phdr->p_filesz;
            Serial.print("elf: loading to=0x");
            Serial.print((unsigned long) dst, HEX);
            Serial.print(", size=");
            Serial.println((unsigned long) size, DEC);

            offset = phdr->p_offset;
            data = (uint8_t *) 0x3fff9000 + 1024;
            data_size = 0x3000 - 1024;
            while (offset - phdr->p_offset < size) {
                do_http_request(SERVER_HOST, SERVER_PORT, "GET", path.c_str(),
                                "", "", 0, &data, &data_size, &offset,
                                SERVER_TLS);

                data = (uint8_t *) 0x3fff9000 + 1024;
                if ((uintptr_t) dst > 0x40200000)
                    ESP.flashWrite((uint32_t) dst - 0x40200000, (uint32_t *) data, size);
                else
                    aligned_memcpy(dst, data, size);
            }
        }
    }

    Serial.print("elf: entrypoint=0x");
    Serial.println(ehdr->e_entry, HEX);
    return ehdr->e_entry;
}
