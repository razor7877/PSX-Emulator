#include <stdint.h>

#include "memory.h"
#include "logging.h"

/// <summary>
/// 2048 KiB
/// </summary>
uint32_t ram[2048 * KIB_SIZE / WORD_SIZE] = { 0 };

/// <summary>
/// 8192 KiB
/// </summary>
uint32_t expansion_1[8192 * KIB_SIZE / WORD_SIZE] = { 0 };

/// <summary>
/// 1 KiB
/// </summary>
uint32_t scratchpad[1 * KIB_SIZE / WORD_SIZE] = { 0 };

/// <summary>
/// 4 KiB
/// </summary>
uint32_t io_ports[4 * KIB_SIZE / WORD_SIZE] = { 0 };

/// <summary>
/// 8 KiB
/// </summary>
uint32_t expansion_2[8 * KIB_SIZE / WORD_SIZE] = { 0 };

/// <summary>
/// 2048 KiB
/// </summary>
uint32_t expansion_3[2048 * KIB_SIZE / WORD_SIZE] = { 0 };

/// <summary>
/// Reads a word at the address in the KUSEG
/// </summary>
/// <param name="address">The address to be read</param>
/// <returns>The word at the address</returns>
static uint32_t read_word_kuseg(uint32_t address)
{
	return 0xFFFFFFFF;
}

/// <summary>
/// Reads a word at the address in the KSEG0
/// </summary>
/// <param name="address">The address to be read</param>
/// <returns>The word at the address</returns>
static uint32_t read_word_kseg0(uint32_t address)
{
	return 0xFFFFFFFF;
}

/// <summary>
/// Reads a word at the address in the KSEG1
/// </summary>
/// <param name="address">The address to be read</param>
/// <returns>The word at the address</returns>
static uint32_t read_word_kseg1(uint32_t address)
{
	return 0xFFFFFFFF;
}

/// <summary>
/// Reads a word at the address in the KSEG2
/// </summary>
/// <param name="address">The address to be read</param>
/// <returns>The word at the address</returns>
static uint32_t read_word_kseg2(uint32_t address)
{
	return 0xFFFFFFFF;
}

/// <summary>
/// Reads a word at the address
/// </summary>
/// <param name="address">The address to be read</param>
/// <returns>The word at the address</returns>
uint32_t read_word(uint32_t address)
{
	if (address <= 0x1FC00000) // KUSEG read
		return read_word_kuseg(address);
	else if (address >= 0x80000000 && address <= 0x9FC00000 + 0x80000) // KSEG 0 read
		return read_word_kseg0(address);
	else if (address >= 0xA0000000 && address <= 0xBFC00000 + 0x80000) // KSEG 1 read
		return read_word_kseg1(address);
	else if (address >= 0xFFFE0000 && address <= 0xFFFE0000 + 0x200) // KSEG 2 read
		return read_word_kseg2(address);

	log_error("Attempted reading word outside of usable address space! Address %x\n", address);

	return 0xFFFFFFFF;
}

/// <summary>
/// Writes a word at the address with the value given in the KUSEG
/// </summary>
/// <param name="address">The address to write to</param>
/// <param name="value">The value to be written</param>
static void write_word_kuseg(uint32_t address, uint32_t value)
{

}

/// <summary>
/// Writes a word at the address with the value given in the KUSEG
/// </summary>
/// <param name="address">The address to write to</param>
/// <param name="value">The value to be written</param>
static void write_word_kseg0(uint32_t address, uint32_t value)
{

}

/// <summary>
/// Writes a word at the address with the value given in the KUSEG
/// </summary>
/// <param name="address">The address to write to</param>
/// <param name="value">The value to be written</param>
static void write_word_kseg1(uint32_t address, uint32_t value)
{

}

/// <summary>
/// Writes a word at the address with the value given in the KUSEG
/// </summary>
/// <param name="address">The address to write to</param>
/// <param name="value">The value to be written</param>
static void write_word_kseg2(uint32_t address, uint32_t value)
{

}

/// <summary>
/// Writes a word at the address with the value given in the KUSEG
/// </summary>
/// <param name="address">The address to write to</param>
/// <param name="value">The value to be written</param>
void write_word(uint32_t address, uint32_t value)
{
	if (address <= 0x1FC00000) // KUSEG write
		return write_word_kuseg(address, value);
	else if (address >= 0x80000000 && address <= 0x9FC00000 + 0x80000) // KSEG 0 write
		return write_word_kseg0(address, value);
	else if (address >= 0xA0000000 && address <= 0xBFC00000 + 0x80000) // KSEG 1 write
		return write_word_kseg1(address, value);
	else if (address >= 0xFFFE0000 && address <= 0xFFFE0000 + 0x200) // KSEG 2 write
		return write_word_kseg2(address, value);

	log_error("Attempted writing word outside of usable address space! Address %x Value %x\n", address, value);
}