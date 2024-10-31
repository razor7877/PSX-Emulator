#include <stdint.h>
#include <string.h>

#include "memory.h"
#include "logging.h"
#include "debug.h"
#include "coprocessor.h"
#include "cpu.h"

/// <summary>
/// 2048 KiB
/// </summary>
uint32_t ram[2048 * KIB_TO_WORD_SIZE] = { 0 };

/// <summary>
/// 8192 KiB
/// </summary>
uint32_t expansion_1[8192 * KIB_TO_WORD_SIZE] = { 0 };

/// <summary>
/// 1 KiB
/// </summary>
uint32_t scratchpad[1 * KIB_TO_WORD_SIZE] = { 0 };

/// <summary>
/// 4 KiB
/// </summary>
uint32_t io_ports[4 * KIB_TO_WORD_SIZE] = { 0 };

/// <summary>
/// 8 KiB
/// </summary>
uint32_t expansion_2[8 * KIB_TO_WORD_SIZE] = { 0 };

/// <summary>
/// 2048 KiB
/// </summary>
uint32_t expansion_3[2048 * KIB_TO_WORD_SIZE] = { 0 };

/// <summary>
/// 512 KiB - Doesn't correspond to actual BIOS ROM size
/// </summary>
uint32_t bios_rom[2048 * KIB_TO_WORD_SIZE] = { 0 };

/// <summary>
/// 0.5 KiB
/// </summary>
uint32_t cpu_cache_control[512 / WORD_SIZE] = { 0 };

void clear_memory()
{
	memset(ram, 0, sizeof(ram));
	memset(expansion_1, 0, sizeof(expansion_1));
	memset(scratchpad, 0, sizeof(scratchpad));
	memset(io_ports, 0, sizeof(io_ports));
	memset(expansion_2, 0, sizeof(expansion_2));
	memset(expansion_3, 0, sizeof(expansion_3));
	memset(bios_rom, 0, sizeof(bios_rom));
	memset(cpu_cache_control, 0, sizeof(cpu_cache_control));
}

/// <summary>
/// Reads a word at the address in the KUSEG
/// </summary>
/// <param name="address">The address to be read</param>
/// <returns>The word at the address</returns>
static uint32_t read_word_kuseg(uint32_t address)
{
	uint32_t word_index = address / WORD_SIZE;

	if (address < RAM_SIZE) // Main RAM
		return ram[word_index];
	else if (address >= 0x1F000000 && address < 0x1F000000 + EXPANSION_1_SIZE) // Expansion region 1
		return expansion_1[word_index - 0x1F000000 / WORD_SIZE];
	else if (address >= 0x1F800000 && address < 0x1F800000 + SCRATCHPAD_SIZE) // Scratchpad (D-cache)
		return scratchpad[word_index - 0x1F800000 / WORD_SIZE];
	else if (address >= 0x1F801000 && address < 0x1F801000 + IO_PORTS_SIZE) // IO ports
		return io_ports[word_index - 0x1F801000 / WORD_SIZE];
	else if (address >= 0x1F802000 && address < 0x1F802000 + EXPANSION_2_SIZE) // Expansion region 2
		return expansion_2[word_index - 0x1F802000 / WORD_SIZE];
	else if (address >= 0x1FA00000 && address < 0x1FA00000 + EXPANSION_3_SIZE) // Expansion region 3
		return expansion_3[word_index - 0x1FA00000 / WORD_SIZE];
	else if (address >= 0x1FC00000 && address < 0x1FC00000 + BIOS_ROM_SIZE) // BIOS ROM
		return bios_rom[word_index - 0x1FC00000 / WORD_SIZE];

	log_error("Attempted reading word outside of usable address space in KUSEG! Address %x\n", address);

	return 0xFFFFFFFF;
}

/// <summary>
/// Reads a word at the address in the KSEG0
/// </summary>
/// <param name="address">The address to be read</param>
/// <returns>The word at the address</returns>
static uint32_t read_word_kseg0(uint32_t address)
{
	uint32_t word_index = address / WORD_SIZE;

	if (address >= 0x80000000 && address < 0x80000000 + RAM_SIZE) // Main RAM
		return ram[word_index - 0x80000000 / WORD_SIZE];
	else if (address >= 0x9F000000 && address < 0x9F000000 + EXPANSION_1_SIZE) // Expansion region 1
		return expansion_1[word_index - 0x9F000000 / WORD_SIZE];
	else if (address >= 0x9F800000 && address < 0x9F800000 + SCRATCHPAD_SIZE) // Scratchpad (D-cache)
		return scratchpad[word_index - 0x9F800000 / WORD_SIZE];
	else if (address >= 0x9F801000 && address < 0x9F801000 + IO_PORTS_SIZE) // IO ports
		return io_ports[word_index - 0x9F801000 / WORD_SIZE];
	else if (address >= 0x9F802000 && address < 0x9F802000 + EXPANSION_2_SIZE) // Expansion region 2
		return expansion_2[word_index - 0x9F802000 / WORD_SIZE];
	else if (address >= 0x9FA00000 && address < 0x9FA00000 + EXPANSION_3_SIZE) // Expansion region 3
		return expansion_3[word_index - 0x9FA00000 / WORD_SIZE];
	else if (address >= 0x9FC00000 && address < 0x9FC00000 + BIOS_ROM_SIZE) // BIOS ROM
		return bios_rom[word_index - 0x9FC00000 / WORD_SIZE];

	log_error("Attempted reading word outside of usable address space in KSEG0! Address %x\n", address);

	return 0xFFFFFFFF;
}

/// <summary>
/// Reads a word at the address in the KSEG1
/// </summary>
/// <param name="address">The address to be read</param>
/// <returns>The word at the address</returns>
static uint32_t read_word_kseg1(uint32_t address)
{
	uint32_t word_index = address / WORD_SIZE;

	if (address >= 0xA0000000 && address < 0xA0000000 + RAM_SIZE) // Main RAM
		return ram[word_index - 0xA0000000 / WORD_SIZE];
	else if (address >= 0xBF000000 && address < 0xBF000000 + EXPANSION_1_SIZE) // Expansion region 1
		return expansion_1[word_index - 0xBF000000 / WORD_SIZE];
	else if (address >= 0xBF801000 && address < 0xBF801000 + IO_PORTS_SIZE) // IO ports
		return io_ports[word_index - 0xBF801000 / WORD_SIZE];
	else if (address >= 0xBF802000 && address < 0xBF802000 + EXPANSION_2_SIZE) // Expansion region 2
		return expansion_2[word_index - 0xBF802000 / WORD_SIZE];
	else if (address >= 0xBFA00000 && address < 0xBFA00000 + EXPANSION_3_SIZE) // Expansion region 3
		return expansion_3[word_index - 0xBFA00000 / WORD_SIZE];
	else if (address >= 0xBFC00000 && address < 0xBFC00000 + BIOS_ROM_SIZE) // BIOS ROM
		return bios_rom[word_index - 0xBFC00000 / WORD_SIZE];

	log_error("Attempted reading word outside of usable address space in KSEG1! Address %x\n", address);

	return 0xFFFFFFFF;
}

/// <summary>
/// Reads a word at the address in the KSEG2
/// </summary>
/// <param name="address">The address to be read</param>
/// <returns>The word at the address</returns>
static uint32_t read_word_kseg2(uint32_t address)
{
	uint32_t word_index = address / WORD_SIZE;

	if (address >= 0xFFFE0000 && address <= 0xFFFE0000 + CONTROL_REGISTERS_SIZE)
		return cpu_cache_control[word_index - 0xFFFE0000 / WORD_SIZE];

	log_error("Attempted reading word outside of usable address space in KSEG2! Address %x\n", address);

	return 0xFFFFFFFF;
}

/// <summary>
/// Reads a word at the address
/// </summary>
/// <param name="address">The address to be read</param>
/// <returns>The word at the address</returns>
uint32_t read_word(uint32_t address)
{
	if ((address & 0b11) != 0)
	{
		log_error("Unaligned memory read exception! PC is %x\n", cpu_state.pc);
		debug_state.in_debug = true;
	}

	// If bit 16 of reg 12 in CPR0 is set, writes are directed to the data cache
	if (CPR0(12) & 0x10000)
	{
		log_warning("Unhandled isolated cache mem read at %x\n", address);
		return 0xFFFFFFFF;
	}

	return read_word_internal(address);
}

uint32_t read_word_internal(uint32_t address)
{
	check_data_breakpoints(address);

	if (address <= 0x1FC00000) // KUSEG read
		return read_word_kuseg(address);
	else if (address >= 0x80000000 && address <= 0x9FC00000 + 0x80000) // KSEG 0 read
		return read_word_kseg0(address);
	else if (address >= 0xA0000000 && address <= 0xBFC00000 + 0x80000) // KSEG 1 read
		return read_word_kseg1(address);
	else if (address >= 0xFFFE0000 && address <= 0xFFFE0000 + 0x200) // KSEG 2 read
		return read_word_kseg2(address);

	log_error("Attempted reading word outside of any memory segment! Address %x --- pc is %x\n", address, cpu_state.pc);

	return 0xFFFFFFFF;
}

/// <summary>
/// Writes a word at the address with the value given in the KUSEG
/// </summary>
/// <param name="address">The address to write to</param>
/// <param name="value">The value to be written</param>
static void write_word_kuseg(uint32_t address, uint32_t value)
{
	uint32_t word_index = address / WORD_SIZE;

	if (address < RAM_SIZE) // Main RAM
		ram[word_index] = value;
	else if (address >= 0x1F000000 && address < 0x1F000000 + EXPANSION_1_SIZE) // Expansion region 1
		expansion_1[word_index - 0x1F000000 / WORD_SIZE] = value;
	else if (address >= 0x1F800000 && address < 0x1F800000 + SCRATCHPAD_SIZE) // Scratchpad (D-cache)
		scratchpad[word_index - 0x1F800000 / WORD_SIZE] = value;
	else if (address >= 0x1F801000 && address < 0x1F801000 + IO_PORTS_SIZE) // IO ports
		io_ports[word_index - 0x1F801000 / WORD_SIZE] = value;
	else if (address >= 0x1F802000 && address < 0x1F802000 + EXPANSION_2_SIZE) // Expansion region 2
		expansion_2[word_index - 0x1F802000 / WORD_SIZE] = value;
	else if (address >= 0x1FA00000 && address < 0x1FA00000 + RAM_SIZE) // Expansion region 3
		expansion_3[word_index - 0x1FA00000 / WORD_SIZE] = value;
	else if (address >= 0x1FC00000 && address < 0x1FC00000 + BIOS_ROM_SIZE) // BIOS ROM
		bios_rom[word_index - 0x1FC00000 / WORD_SIZE] = value;
	else
		log_error("Attempted to write outside of usable address space in KUSEG! ADDRESS %x VALUE %x\n", address, value);
}

/// <summary>
/// Writes a word at the address with the value given in the KSEG0
/// </summary>
/// <param name="address">The address to write to</param>
/// <param name="value">The value to be written</param>
static void write_word_kseg0(uint32_t address, uint32_t value)
{
	uint32_t word_index = address / WORD_SIZE;

	if (address >= 0x80000000 && address < 0x80000000 + RAM_SIZE) // Main RAM
		ram[word_index - 0x80000000 / WORD_SIZE] = value;
	else if (address >= 0x9F000000 && address < 0x9F000000 + EXPANSION_1_SIZE) // Expansion region 1
		expansion_1[word_index - 0x9F000000 / WORD_SIZE] = value;
	else if (address >= 0x9F800000 && address < 0x9F800000 + SCRATCHPAD_SIZE) // Scratchpad (D-cache)
		scratchpad[word_index - 0x9F800000 / WORD_SIZE] = value;
	else if (address >= 0x9F801000 && address < 0x9F801000 + IO_PORTS_SIZE) // IO ports
		io_ports[word_index - 0x9F801000 / WORD_SIZE] = value;
	else if (address >= 0x9F802000 && address < 0x9F802000 + EXPANSION_2_SIZE) // Expansion region 2
		expansion_2[word_index - 0x9F802000 / WORD_SIZE] = value;
	else if (address >= 0x9FA00000 && address < 0x9FA00000 + RAM_SIZE) // Expansion region 3
		expansion_3[word_index - 0x9FA00000 / WORD_SIZE] = value;
	else if (address >= 0x9FC00000 && address < 0x9FC00000 + BIOS_ROM_SIZE) // BIOS ROM
		bios_rom[word_index - 0x9FC00000 / WORD_SIZE] = value;
	else
		log_error("Attempted to write outside of usable address space in KSEG0! ADDRESS %x VALUE %x\n", address, value);
}

/// <summary>
/// Writes a word at the address with the value given in the KSEG1
/// </summary>
/// <param name="address">The address to write to</param>
/// <param name="value">The value to be written</param>
static void write_word_kseg1(uint32_t address, uint32_t value)
{
	uint32_t word_index = address / WORD_SIZE;

	if (address >= 0xA0000000 && address < 0xA0000000 + RAM_SIZE) // Main RAM
		ram[word_index - 0xA0000000 / WORD_SIZE] = value;
	else if (address >= 0xBF000000 && address < 0xBF000000 + EXPANSION_1_SIZE) // Expansion region 1
		expansion_1[word_index - 0xBF000000 / WORD_SIZE] = value;
	else if (address >= 0xBF801000 && address < 0xBF801000 + IO_PORTS_SIZE) // IO ports
		io_ports[word_index - 0xBF801000 / WORD_SIZE] = value;
	else if (address >= 0xBF802000 && address < 0xBF802000 + EXPANSION_2_SIZE) // Expansion region 2
		expansion_2[word_index - 0xBF802000 / WORD_SIZE] = value;
	else if (address >= 0xBFA00000 && address < 0xBFA00000 + RAM_SIZE) // Expansion region 3
		expansion_3[word_index - 0xBFA00000 / WORD_SIZE] = value;
	else if (address >= 0xBFC00000 && address < 0xBFC00000 + BIOS_ROM_SIZE) // BIOS ROM
		bios_rom[word_index - 0xBFC00000 / WORD_SIZE] = value;
	else
		log_error("Attempted to write outside of usable address space in KSEG1! ADDRESS %x VALUE %x\n", address, value);
}

/// <summary>
/// Writes a word at the address with the value given in the KSEG2
/// </summary>
/// <param name="address">The address to write to</param>
/// <param name="value">The value to be written</param>
static void write_word_kseg2(uint32_t address, uint32_t value)
{
	uint32_t word_index = address / WORD_SIZE;

	if (address >= 0xFFFE0000 && address <= 0xFFFE0000 + CONTROL_REGISTERS_SIZE)
		cpu_cache_control[word_index - 0xFFFE0000 / WORD_SIZE] = value;
	else
		log_error("Attempted to write outside of usable address space in KSEG2! ADDRESS %x VALUE %x\n", address, value);
}

/// <summary>
/// Writes a word at the address with the value given
/// </summary>
/// <param name="address">The address to write to</param>
/// <param name="value">The value to be written</param>
void write_word(uint32_t address, uint32_t value)
{
	if ((address & 0b11) != 0)
	{
		log_error("Unaligned memory write exception! PC is %x\n", cpu_state.pc);
		debug_state.in_debug = true;
		return;
	}

	check_data_breakpoints(address);

	// If bit 16 of reg 12 in CPR0 is set, writes are directed to the data cache
	if (CPR0(12) & 0x10000)
	{
		log_warning("Unhandled isolated cache mem write at %x value %x\n", address, value);
		return;
	}

	if (address <= 0x1FC00000) // KUSEG write
		return write_word_kuseg(address, value);
	else if (address >= 0x80000000 && address <= 0x9FC00000 + 0x80000) // KSEG 0 write
		return write_word_kseg0(address, value);
	else if (address >= 0xA0000000 && address <= 0xBFC00000 + 0x80000) // KSEG 1 write
		return write_word_kseg1(address, value);
	else if (address >= 0xFFFE0000 && address <= 0xFFFE0000 + 0x200) // KSEG 2 write
		return write_word_kseg2(address, value);

	log_error("Attempted writing word outside of any memory segment! Address %x Value %x --- pc is %x\n", address, value, cpu_state.pc);
}

void load_bios_into_mem(FILE* bios_file)
{
	fread(&bios_rom, sizeof(uint8_t), 512 * KIB_SIZE, bios_file);
}