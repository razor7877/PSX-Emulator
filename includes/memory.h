#pragma once

#include <stdint.h>
#include <stdio.h>

#include "main.h"

#define WORD_SIZE 4
#define KIB_SIZE 1024
#define KIB_TO_WORD_SIZE (KIB_SIZE / WORD_SIZE)

#define RAM_SIZE (2048 * KIB_SIZE)
#define EXPANSION_1_SIZE (8192 * KIB_SIZE)
#define SCRATCHPAD_SIZE (1 * KIB_SIZE)
#define IO_PORTS_SIZE (4 * KIB_SIZE)
#define EXPANSION_2_SIZE (8 * KIB_SIZE)
#define EXPANSION_3_SIZE (2048 * KIB_SIZE)
#define BIOS_ROM_SIZE (512 * KIB_SIZE)
#define CONTROL_REGISTERS_SIZE 512

/// <summary>
/// Clears all the system's memory
/// </summary>
void clear_memory();

/// <summary>
/// Reads a word at the address
/// </summary>
/// <param name="address">The address to be read</param>
/// <returns>The word at the address</returns>
uint32_t read_word(uint32_t address);

uint32_t read_word_internal(uint32_t address);

/// <summary>
/// Writes a word at the address with the value given in the KUSEG
/// </summary>
/// <param name="address">The address to write to</param>
/// <param name="value">The value to be written</param>
void write_word(uint32_t address, uint32_t value);

/// <summary>
/// Loads the content of a file stream into the memory of the BIOS ROM
/// </summary>
/// <param name="bios_file">The file stream to be loaded, it should contain 512 KiB of binary data</param>
void load_bios_into_mem(FILE* bios_file);

/// <summary>
/// Sideloads an EXE file into the RAM. May be called after BIOS finishes execution (when PC = 0x80030000)
/// to run a program without loading a full ISO disk image
///</summary>
/// <param name="exe_file">The file stream to be loaded</param>
/// <param name="exe_size">The size of the EXE to be loaded</param>
void sideload_exe_into_mem(exe_header file_header, FILE* exe_file);