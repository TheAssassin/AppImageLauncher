// system headers
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <linux/elf.h>
#include <byteswap.h>
#include <fstream>
#include <iostream>
#include <stdexcept>

// own headers
#include "elf.h"
#include "logging.h"

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define NATIVE_BYTE_ORDER ELFDATA2LSB
#elif __BYTE_ORDER == __BIG_ENDIAN
#define NATIVE_BYTE_ORDER ELFDATA2MSB
#else
#error "Unknown machine endian"
#endif

template<typename T>
T bswap(T val) = delete;

template<>
uint16_t bswap(uint16_t val) {
    return bswap_16(val);
}

template<>
uint32_t bswap(uint32_t val) {
    return bswap_32(val);
}

template<>
unsigned long long bswap(unsigned long long val) {
    return bswap_64(val);
}

template<typename EhdrT, typename ValT>
void swap_data_if_necessary(const EhdrT& ehdr, ValT& val) {
    static_assert(std::is_same<Elf64_Ehdr, EhdrT>::value || std::is_same<Elf32_Ehdr, EhdrT>::value,
                  "must be Elf{32,64}_Ehdr");

    if (ehdr.e_ident[EI_DATA] != NATIVE_BYTE_ORDER) {
        val = bswap(val);
    }
}

template<typename EhdrT, typename ShdrT>
off_t get_elf_size(std::ifstream& ifs)
{
    static_assert(std::is_same<Elf64_Ehdr, EhdrT>::value || std::is_same<Elf32_Ehdr, EhdrT>::value,
                  "must be Elf{32,64}_Ehdr");
    static_assert(std::is_same<Elf64_Shdr, ShdrT>::value || std::is_same<Elf32_Shdr, ShdrT>::value,
                  "must be Elf{32,64}_Shdr");

    EhdrT elf_header{};

    ifs.seekg(0, std::ifstream::beg);
    ifs.read(reinterpret_cast<char*>(&elf_header), sizeof(elf_header));

    if (!ifs) {
        log_error("failed to read ELF header\n");
        return -1;
    }

    swap_data_if_necessary(elf_header, elf_header.e_shoff);
    swap_data_if_necessary(elf_header, elf_header.e_shentsize);
    swap_data_if_necessary(elf_header, elf_header.e_shnum);
    swap_data_if_necessary(elf_header, elf_header.e_shnum);

    off_t last_shdr_offset = elf_header.e_shoff + (elf_header.e_shentsize * (elf_header.e_shnum - 1));
    ShdrT section_header{};

    ifs.seekg(last_shdr_offset, std::ifstream::beg);
    ifs.read(reinterpret_cast<char*>(&section_header), sizeof(elf_header));

    if (!ifs) {
        log_error("failed to read ELF section header\n");
        return -1;
    }

    swap_data_if_necessary(elf_header, section_header.sh_offset);
    swap_data_if_necessary(elf_header, section_header.sh_size);

    /* ELF ends either with the table of section headers (SHT) or with a section. */
    off_t sht_end = elf_header.e_shoff + (elf_header.e_shentsize * elf_header.e_shnum);
    off_t last_section_end = section_header.sh_offset + section_header.sh_size;
    return sht_end > last_section_end ? sht_end : last_section_end;
}

bool is_32bit_elf(std::ifstream& ifs) {
    if (!ifs) {
        log_error("failed to read e_ident from ELF file\n");
        return -1;
    }

    // for the beginning, we just need to read e_ident to determine ELF class (i.e., either 32-bit or 64-bit)
    // that way, we can decide which way to go
    // the easiest way is to just use the ELF API
    Elf64_Ehdr ehdr;

    ifs.read(reinterpret_cast<char*>(&ehdr.e_ident), EI_NIDENT);

    switch (ehdr.e_ident[EI_CLASS]) {
        case ELFCLASS32: {
            return true;
        }
        case ELFCLASS64: {
            return false;
        }
    }

    throw std::logic_error{"ELF binary is neither 32-bit nor 64-bit"};
}

bool is_32bit_elf(const char* filename) {
    std::ifstream ifs(filename);

    if (!ifs) {
        log_error("could not open file\n");
        return -1;
    }

    return is_32bit_elf(ifs);
}

ssize_t elf_binary_size(const char* filename) {
    std::ifstream ifs(filename);

    if (!ifs) {
        log_error("could not open file\n");
        return -1;
    }

    if (is_32bit_elf(ifs)) {
        return get_elf_size<Elf32_Ehdr, Elf32_Shdr>(ifs);
    } else {
        return get_elf_size<Elf64_Ehdr, Elf64_Shdr>(ifs);
    }
}
