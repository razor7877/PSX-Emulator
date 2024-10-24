#pragma once

#define WORD_SIZE 4
#define KIB_SIZE 1024

/// <summary>
/// 512 KiB - Doesn't correspond to actual BIOS ROM size
/// </summary>
uint32_t bios_rom[2048 * KIB_SIZE / WORD_SIZE];

/// <summary>
/// 0.5 KiB
/// </summary>
uint32_t cpu_cache_control[512 / WORD_SIZE];

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
