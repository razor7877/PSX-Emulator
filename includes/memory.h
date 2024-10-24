#pragma once

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
/// Reads a word at the address
/// </summary>
/// <param name="address">The address to be read</param>
/// <returns>The word at the address</returns>
uint32_t read_word(uint32_t address);

/// <summary>
/// Writes a word at the address with the value given in the KUSEG
/// </summary>
/// <param name="address">The address to write to</param>
/// <param name="value">The value to be written</param>
void write_word(uint32_t address, uint32_t value);
