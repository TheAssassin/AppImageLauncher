#pragma once

/**
 * Calculate size of ELF binary. Useful e.g., to estimate the size of the runtime in an AppImage.
 * @param filename path to ELF file
 * @return size of ELF part in bytes
 */
ssize_t elf_binary_size(const char* filename);
