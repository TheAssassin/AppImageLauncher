#pragma once

/**
 * Calculate size of ELF binary. Useful e.g., to estimate the size of the runtime in an AppImage.
 * @param filename path to ELF file
 * @return size of ELF part in bytes
 */
ssize_t elf_binary_size(const char* filename);

/**
 * Check whether a given ELF file is a 32-bit binary.
 * @param filename path to ELF file
 * @return true if it's a 32-bit ELF, false otherwise
 */
bool is_32bit_elf(const char* filename);
